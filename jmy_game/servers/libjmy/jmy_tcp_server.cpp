#include "jmy_tcp_server.h"
#include "jmy_tcp_session.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_log.h"

JmyTcpServer::JmyTcpServer(io_service& service) :
	service_(service),
	sock_(service_),
	conn_mgr_(service_),
	curr_conn_(service_, conn_mgr_, JMY_CONN_TYPE_PASSIVE),
	inited_(false)
#if USE_THREAD
	,
	free_sock_list_(0),
	accept_sock_list_(0)
#endif

{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_);
#if USE_NET_PROTO2
	handler_ = std::make_shared<JmyDataHandler2>();
#else
	handler_ = std::make_shared<JmyDataHandler>();
#endif
	event_handler_ = std::make_shared<JmyEventHandlerManager>();
	buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::JmyTcpServer(io_service& service, unsigned short port) :
	service_(service),
	sock_(service_),
	conn_mgr_(service_),
	curr_conn_(service_, conn_mgr_, JMY_CONN_TYPE_PASSIVE),
	inited_(false)
#if USE_THREAD
	, free_sock_list_(0),
	accept_sock_list_(0)
#endif
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, ip::tcp::endpoint(ip::tcp::v4(), port));
#if USE_NET_PROTO2
	handler_ = std::make_shared<JmyDataHandler2>();
#else
	handler_ = std::make_shared<JmyDataHandler>();
#endif
	event_handler_ = std::make_shared<JmyEventHandlerManager>();
	buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::~JmyTcpServer()
{
	close();
}

bool JmyTcpServer::loadConfig(const JmyServerConfig& conf)
{
	conf_ = conf;
	bool res = handler_->loadMsgHandle(conf.conn_conf.handlers, conf.conn_conf.nhandlers);
	if (!res) return false;
	handler_->setDefaultMsgHandler(conf.conn_conf.default_msg_handler);
	event_handler_->setBaseHandlers(conf.conn_conf.base_event_handlers);
	if (conf.conn_conf.other_event_handlers && conf.conn_conf.other_event_nhandlers) {
		event_handler_->init(conf.conn_conf.other_event_handlers, conf.conn_conf.other_event_nhandlers);
	}
	conn_mgr_.init(conf.max_conn, JMY_CONN_TYPE_PASSIVE, conf.conn_conf);
	// use send list
	res = buff_pool_->init(conf.max_conn,
							0, 0, conf.conn_conf.buff_conf.send_buff_size, conf.conn_conf.buff_conf.recv_buff_size);

	int init_buffer_size = 2 * conf.max_conn;
	if (!buffer_mgr_.init(init_buffer_size, buff_pool_)) {
		LibJmyLogError("buffer manager init size(%d) failed", init_buffer_size);
		return false;
	}

#if USE_THREAD
	free_sock_list_.reserve(conf.max_conn);
	int i = 0;
	for (; i<(int)conf.max_conn; ++i) {
		free_sock_list_.push(jmy_mem_malloc<ip::tcp::socket>(service_));
	}
	accept_sock_list_.reserve(conf.max_conn);
#endif
	
	if (res)
		inited_ = true;

	return res;
}

void JmyTcpServer::close()
{
	buff_pool_->clear();
	conns_.clear();
	conn_mgr_.clear();
	acceptor_->close();
#if USE_THREAD
	ip::tcp::socket* sock = nullptr;
	while (free_sock_list_.pop(sock)) {
		jmy_mem_free<ip::tcp::socket>(sock);
	}
	while (accept_sock_list_.pop(sock)) {
		jmy_mem_free<ip::tcp::socket>(sock);
	}
	thread_->join();
#endif
}

int JmyTcpServer::start()
{
	int res = 0;
	if (!inited_) return -1;
#if USE_THREAD
	thread_ = std::make_shared<std::thread>(std::thread(std::bind(&JmyTcpServer::do_loop, this)));
#else
	res = do_accept();
#endif
	return res;
}

int JmyTcpServer::listenStart(const std::string& ip, unsigned short port)
{
	if (!inited_) return -1;
	ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	acceptor_->open(ep.protocol());
	acceptor_->set_option(socket_base::reuse_address(true));
	acceptor_->bind(ep);
#if !USE_THREAD
	acceptor_->listen();
#endif
	return start();
}

int JmyTcpServer::listenStart(const char* ip, unsigned short port)
{
	return listenStart(std::string(ip), port);
}

#if USE_THREAD
int JmyTcpServer::do_accept_loop()
{
	if (!inited_) return -1;
	acceptor_->listen();
	boost::system::error_code ec;
	ip::tcp::socket tmp_sock(service_);
	while (true) {
		acceptor_->accept(tmp_sock, ec);
		if (ec) {
			if (ec.value()==boost::system::errc::operation_canceled || ec.value()==boost::system::errc::operation_in_progress) {
				LibJmyLogError("error code(%d)", ec.value());
			} else {
				LibJmyLogError("async_accept error: %d", ec.value());
				return -1;	
			}
		} else {
			ip::tcp::socket* sock = nullptr;
			if (!free_sock_list_.pop(sock)) {
				LibJmyLogWarn("no free sock to accept new connection");
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			*sock = std::move(tmp_sock);
			accept_sock_list_.push(sock);
		}
	}
	return 1;
}
#else
int JmyTcpServer::do_accept()
{
	if (!inited_) return -1;
	acceptor_->async_accept(curr_conn_.getSock(),
		[this](boost::system::error_code ec) {
			if (ec) {
				if (ec.value()==boost::system::errc::operation_canceled || ec.value()==boost::system::errc::operation_in_progress) {
					LibJmyLogError("error code(%d)", ec.value());
				} else {
					LibJmyLogError("async_accept error: %d", ec.value());
					return;	
				}
			} else {
				size_t s = conns_.size();
				if (conf_.max_conn <= s) {
					curr_conn_.getSock().close();
					LibJmyLogWarn("already max connections: %d", s);
				} else {
					ip::tcp::endpoint ep = curr_conn_.getSock().remote_endpoint();
					const char* remote_ip = ep.address().to_string().c_str();
					unsigned short remote_port = ep.port();
					int id = accept_new(&curr_conn_.getSock());
					if (id < 0)
						return;
					LibJmyLogInfo("new connection(%d, %s:%d) start", id, remote_ip, remote_port);
				}
			}
			do_accept();
		}
	);
	return 1;
}
#endif

int JmyTcpServer::accept_new(ip::tcp::socket* sock)
{
	int id = id_gene_.get();
	if (id == 0) {
		LibJmyLogError("the id is used out");
		return -1;
	}
	JmyTcpConnection* conn = conn_mgr_.getFree(id);
	conn->getSock() = std::move(*sock); 
	//conn->getSock().set_option(ip::tcp::no_delay(true));
	boost::asio::socket_base::linger option(true, 0);
	conn->getSock().set_option(option);
	conn->setDataHandler(handler_);
	conn->setEventHandler(event_handler_);
	std::shared_ptr<JmyConnectionBuffer> buffer;
	if (!buffer_mgr_.getOneBuffer(buffer)) {
		LibJmyLogError("get free buffer failed");
		return -1;
	}
	buffer->init(conf_.conn_conf.buff_conf, buff_pool_);
	conn->setBuffer(buffer);
	conn->start();
	conns_.push_back(conn);
	// handle accept event
	JmyEventInfo evt;
	evt.event_id = JMY_EVENT_CONNECT;
	evt.conn_id = id;
	evt.param = (void*)&conn_mgr_;
	evt.param_l = 0;
	event_handler_->onConnect(&evt);
	return id;
}

#if USE_THREAD
int JmyTcpServer::do_loop()
{
	if (do_accept_loop() < 0) {
		return -1;
	}
	return 0;
}
#endif

int JmyTcpServer::run()
{
#if USE_THREAD
	ip::tcp::socket* sock = nullptr;
	if (accept_sock_list_.pop(sock)) {
		accept_new(sock);
		free_sock_list_.push(sock);
	}
#endif
	if (run_conns() < 0) {
		LibJmyLogError("run conns failed");
		return -1;
	}
	return 0;
}

int JmyTcpServer::run_conns()
{
	int n = 0;
	std::list<JmyTcpConnection*>::iterator tmp_it;
	JmyTcpConnection* conn = nullptr;
	std::list<JmyTcpConnection*>::iterator it = conns_.begin();
	for (; it!=conns_.end(); ) {
		tmp_it = it;
		tmp_it++;
		bool del = false;
		conn = *it;
		if (!conn || conn->isDisconnect()) {
			del = true;
		} else {
			if (conn->run() >= 0) {
				n += 1;
			}
		}
		if (del) {
			if (conn) {
				conn_mgr_.free(conn);
				conn->getBuffer()->clear();
				buffer_mgr_.freeBuffer(conn->getBuffer()->id);
			}
			conns_.erase(it);
		}
		it = tmp_it;
	}
	return n;
}
