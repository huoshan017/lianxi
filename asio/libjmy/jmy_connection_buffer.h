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
	void init(const JmyBufferConfig& config) {
		use_send_list = config.use_send_buff_list;
		use_recv_list = config.use_recv_buff_list;
		if (!use_send_list) {
			send_buff.init(config.send_buff_size, SESSION_BUFFER_TYPE_SEND);
		}
		if (!use_recv_list) {
			recv_buff.init(config.recv_buff_size, SESSION_BUFFER_TYPE_RECV);
		}
	}
	void destroy() {
		send_buff_list.destroy();
		recv_buff_list.destroy();
		send_buff.destroy();
		recv_buff.destroy();
	}
	void reset() {
		send_buff_list.reset();
		recv_buff_list.reset();
		send_buff.reset();
		recv_buff.reset();
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
