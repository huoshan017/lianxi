#include "jmy_tcp_server.h"
#include "jmy_tcp_session.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_log.h"

JmyTcpServer::JmyTcpServer() :
	sock_(service_),
#if USE_CONNECTOR_AND_SESSION
	curr_session_(service_),
#else
	curr_conn_(service_, JMY_CONN_TYPE_PASSIVE),
#endif
	inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_);
	handler_ = std::make_shared<JmyDataHandler>();
	conn_mgr_ = std::make_shared<JmyTcpConnectionMgr>();
#if USE_CONNECTOR_AND_SESSION
	session_mgr_ = std::make_shared<JmyTcpSessionMgr>();
#endif
	session_buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::JmyTcpServer(short port) :
	sock_(service_),
#if USE_CONNECTOR_AND_SESSION
	curr_session_(service_),
#else
	curr_conn_(service_, JMY_CONN_TYPE_PASSIVE),
#endif
	inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, ip::tcp::endpoint(ip::tcp::v4(), port));
	handler_ = std::make_shared<JmyDataHandler>();
	conn_mgr_ = std::make_shared<JmyTcpConnectionMgr>();
#if USE_CONNECTOR_AND_SESSION
	session_mgr_ = std::make_shared<JmyTcpSessionMgr>();
#endif
	session_buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::~JmyTcpServer()
{
	close();
}

bool JmyTcpServer::loadConfig(const JmyServerConfig& conf)
{
	conf_ = conf;
	bool res = handler_->loadMsgHandle(conf.handlers, conf.nhandlers);
	if (!res) return false;
#if USE_CONNECTOR_AND_SESSION
	res = session_mgr_->init(conf.max_conn, service_);
	if (res) inited_ = true;
#endif
	// use send list
#if USE_CONNECTOR_AND_SESSION
	if (conf.session_conf.use_send_list) {
		
	} else {
		res = session_buff_pool_->init(conf.max_conn,
							conf.session_conf.send_buff_min,
							conf.session_conf.recv_buff_min,
							conf.session_conf.send_buff_max,
							conf.session_conf.recv_buff_max);
	}
#else
	res = session_buff_pool_->init(conf.max_conn,
							0, 0, conf.conn_conf.buff_conf.send_buff_size, conf.conn_conf.buff_conf.recv_buff_size);
#endif
	return res;
}

void JmyTcpServer::close()
{
	session_buff_pool_->clear();
#if USE_CONNECTOR_AND_SESSION
	session_mgr_->clear();
#endif
	acceptor_->close();
#if USE_THREAD
	thread_->join();
#endif
}

int JmyTcpServer::start()
{
	if (!inited_) return -1;
	int res = do_accept();
#if USE_THREAD
	thread_ = std::make_shared<std::thread>(std::thread(std::bind(&JmyTcpServer::do_loop, this)));
#endif
	return res;
}

int JmyTcpServer::listenStart(short port)
{
	if (!inited_) return -1;
	ip::tcp::endpoint ep(ip::tcp::v4(), port);
	acceptor_->open(ep.protocol());
	acceptor_->set_option(socket_base::reuse_address(true));
	acceptor_->bind(ep);
	acceptor_->listen();
	return start();
}

int JmyTcpServer::do_accept()
{
	if (!inited_) return -1;
#if USE_CONNECTOR_AND_SESSION
	acceptor_->async_accept(curr_session_.getSock(),
#else
	acceptor_->async_accept(curr_conn_.getSock(),
#endif
		[this](boost::system::error_code ec){
			if (ec) {
				if (ec.value()==boost::system::errc::operation_canceled || ec.value()==boost::system::errc::operation_in_progress) {
					LibJmyLogError("error code(%d)", ec.value());
				} else {
					LibJmyLogError("async_accept error: %d", ec.value());
					return;	
				}
			} else {
#if USE_CONNECTOR_AND_SESSION
				JmyTcpSession* session = session_mgr_->getOneSession(session_buff_pool_, handler_, conf_.session_conf.use_send_list);
				if (!session) {
					LibJmyLogError("get free MyTcpSession failed");
					return;
				}

				session->getSock() = std::move(curr_session_.getSock());
				session->getSock().set_option(ip::tcp::no_delay(true));
				session->start();
				ip::tcp::endpoint ep = session->getSock().remote_endpoint();
				LibJmyLogInfo("new session %d(%s:%d) start", session->getId(), ep.address().to_string().c_str(), ep.port());
#else
				int id = id_gene_.get();
				if (id == 0) {
					LibJmyLogError("the id is used out");
					return;
				}
				JmyTcpConnection* conn = conn_mgr_->getFree(id);
				conn->getSock() = std::move(curr_conn_.getSock());
				conn->getSock().set_option(ip::tcp::no_delay(true));
				conn->start();
				ip::tcp::endpoint ep = conn->getSock().remote_endpoint();
				LibJmyLogInfo("new connection %(%s:%d) start", conn->getId(), ep.address().to_string().c_str(), ep.port());
#endif
			}
			do_accept();
		});
	return 1;
}

#if USE_THREAD
int JmyTcpServer::do_loop()
{
	size_t s = service_.run();
	return s;
}
#endif

int JmyTcpServer::run()
{
#if USE_CONNECTOR_AND_SESSION
	if (session_mgr_->run() < 0)
		return -1;
#else
	if (conn_mgr_->usedRun() < 0)
		return -1;
#endif

#if !USE_THREAD
	service_.poll();
#endif
	return 0;
}
