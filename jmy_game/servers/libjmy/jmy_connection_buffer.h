#pragma once

#include "jmy_session_buffer.h"
#include <memory>
#include <list>
#include <vector>
#include <unordered_set>

enum JmyConnectionBufferState {
	JMY_CONN_BUFFER_STATE_IDLE = 0,
	JMY_CONN_BUFFER_STATE_USING = 1,
	JMY_CONN_BUFFER_STATE_SUSPEND = 2,
};

struct JmyConnectionBuffer {
	int id;
	JmyConnectionBufferState state;
	JmyDoubleSessionBuffer send_buff;					// send buffer
	JmySessionBufferList send_buff_list;				// send buffer list
#if !USE_NET_PROTO2
	JmyDoubleSessionBuffer recv_buff;					// recv buffer
#else
	JmySessionBufferList recv_buff_list;				// recv buffer list
#endif
	bool use_send_list;
	JmyTotalReconnInfo total_reconn_info;				// hold total reconn info

	JmyConnectionBuffer() : id(0), state(JMY_CONN_BUFFER_STATE_IDLE) {
	}

	void init(const JmyBufferConfig& config, std::shared_ptr<JmySessionBufferPool> pool) {
		use_send_list = config.use_send_buff_list;
#if !USE_NET_PROTO2
		use_recv_list = config.use_recv_buff_list;
#endif
		if (!use_send_list) {
			send_buff.init(pool, SESSION_BUFFER_TYPE_SEND);
		} else {
			send_buff_list.init(0, 0);
		}
#if !USE_NET_PROTO2
		if (!use_recv_list) {
			recv_buff.init(pool, SESSION_BUFFER_TYPE_RECV);
		} else {
			recv_buff_list.init(0, 0);
		}
#endif
		std::memset(&total_reconn_info, 0, sizeof(total_reconn_info));
	}

	void init(const JmyBufferConfig& config) {
		use_send_list = config.use_send_buff_list;
#if !USE_NET_PROTO2
		use_recv_list = config.use_recv_buff_list;
#endif
		if (!use_send_list) {
			send_buff.init(config.send_buff_size, SESSION_BUFFER_TYPE_SEND);
		} else {
			send_buff_list.init(0, 0);
		}
#if !USE_NET_PROTO2
		if (!use_recv_list) {
			recv_buff.init(config.recv_buff_size, SESSION_BUFFER_TYPE_SEND);
		} else {
			recv_buff_list.init(0, 0);
		}
#endif
	}

	void destroy() {
		send_buff_list.destroy();
		recv_buff_list.destroy();
		send_buff.destroy();
#if !USE_NET_PROTO2
		recv_buff.destroy();
#endif
		total_reconn_info.recv_count = 0;
		total_reconn_info.send_count = 0;	
	}

	void clear() {
		send_buff_list.clear();
		recv_buff_list.clear();
		send_buff.clear();
#if !USE_NET_PROTO2
		recv_buff.clear();
#endif
		total_reconn_info.recv_count = 0;
		total_reconn_info.send_count = 0;
		LibJmyLogDebug("clear connection buffer(%d)", id);
	}
};

class JmyConnectionBufferMgr {
public:
	JmyConnectionBufferMgr();
	~JmyConnectionBufferMgr();

	bool init(int max_size);
	bool init(int max_size, std::shared_ptr<JmySessionBufferPool> buff_pool);
	void clear();

	bool getOneBuffer(std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool getBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool freeBuffer(int id);

private:
	int max_size_;
	std::vector<std::shared_ptr<JmyConnectionBuffer> > conn_buff_vec_;
	std::list<std::shared_ptr<JmyConnectionBuffer> > free_queue_;
	std::unordered_set<std::shared_ptr<JmyConnectionBuffer> > used_set_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
};
