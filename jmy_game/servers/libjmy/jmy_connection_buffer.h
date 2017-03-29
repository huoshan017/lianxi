#pragma once

#include "jmy_session_buffer.h"
#include <memory>
#include <list>
#include <unordered_map>

enum JmyConnectionBufferState {
	JMY_CONN_BUFFER_STATE_IDLE = 0,
	JMY_CONN_BUFFER_STATE_USING = 1,
	JMY_CONN_BUFFER_STATE_SUSPEND = 2,
};

struct JmyConnectionBuffer {
	int id;
	JmyConnectionBufferState state;
	JmyDoubleSessionBuffer recv_buff, send_buff;		// data buffer
	JmySessionBufferList send_buff_list;				// send buffer list
	JmySessionBufferList recv_buff_list;				// recv buffer list
	bool use_send_list;
	bool use_recv_list;
	JmyTotalReconnInfo total_reconn_info;				// hold total reconn info

	void init(const JmyBufferConfig& config, std::shared_ptr<JmySessionBufferPool> pool) {
		use_send_list = config.use_send_buff_list;
		use_recv_list = config.use_recv_buff_list;
		if (!use_send_list) {
			send_buff.init(pool, SESSION_BUFFER_TYPE_SEND);
		} else {
			send_buff_list.init(0, 0);
		}
		if (!use_recv_list) {
			recv_buff.init(pool, SESSION_BUFFER_TYPE_RECV);
		} else {
			recv_buff_list.init(0, 0);
		}
		std::memset(&total_reconn_info, 0, sizeof(total_reconn_info));
	}

	void destroy() {
		send_buff_list.destroy();
		recv_buff_list.destroy();
		send_buff.destroy();
		recv_buff.destroy();
		total_reconn_info.recv_count = 0;
		total_reconn_info.send_count = 0;	
	}

	void reset() {
		send_buff_list.reset();
		recv_buff_list.reset();
		send_buff.reset();
		recv_buff.reset();
		total_reconn_info.recv_count = 0;
		total_reconn_info.send_count = 0;
	}
};

class JmyConnectionBufferMgr {
public:
	JmyConnectionBufferMgr();
	~JmyConnectionBufferMgr();

	bool init(int max_size, std::shared_ptr<JmySessionBufferPool> buff_pool);
	void clear();

	bool getOneBuffer(std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool getBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool freeBuffer(int id);
	void restoreBuffer(std::shared_ptr<JmyConnectionBuffer> buffer);
	bool suspendBuffer(std::shared_ptr<JmyConnectionBuffer> buffer);
	bool getSuspendBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool restoreSuspendBuffer(int id);

private:
	int max_size_;
	std::shared_ptr<JmyConnectionBuffer>* conn_buff_vec_;
	std::list<std::shared_ptr<JmyConnectionBuffer> > free_queue_;
	std::unordered_map<int, std::shared_ptr<JmyConnectionBuffer> > suspend_map_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
};