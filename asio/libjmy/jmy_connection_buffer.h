#pragma once

#include "jmy_session_buffer.h"
#include "jmy_singleton.hpp"
#include <memory>
#include <vector>
#include <atomic>
#include <boost/lockfree/queue.hpp>

struct JmyConnectionBuffer {
	int id;
	std::atomic<bool> is_used;
	JmyDoubleSessionBuffer recv_buff, send_buff;		// data buffer
	JmySessionBufferList send_buff_list;				// send buffer list
	JmySessionBufferList recv_buff_list;				// recv buffer list
	bool use_send_list;
	bool use_recv_list;
};

class JmyConnectionBufferMgr : public JmySingleton<JmyConnectionBufferMgr> {
public:
	JmyConnectionBufferMgr();
	~JmyConnectionBufferMgr();

	bool init(int max_size);
	void clear();

	bool getOneBuffer(std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool getBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer);
	bool freeBuffer(int id);
	bool restoreBuffer(std::shared_ptr<JmyConnectionBuffer> buffer);

private:
	int max_size_;
	std::shared_ptr<JmyConnectionBuffer>* conn_buff_vec_;
	boost::lockfree::queue<std::shared_ptr<JmyConnectionBuffer> > free_queue_;
	boost::lockfree::queue<std::shared_ptr<JmyConnectionBuffer> > suspend_queue_;
};
