#pragma once

enum { JMY_MAX_MSG_SIZE = 64*1024 };
const unsigned int DEFAULT_SEND_BUFFER_SIZE = 4096;
const unsigned int MAX_SEND_BUFFER_SIZE = 640*1024;
const unsigned int DEFAULT_RECV_BUFFER_SIZE = 4096;
const unsigned int MAX_RECV_BUFFER_SIZE = 640*1024;
const unsigned int DEFAULT_MAX_SEND_BUFFER_COUNT = 100;
const unsigned int DEFAULT_MAX_RECV_BUFFER_COUNT = 100;

enum { JMY_ACK_START_ID				= 1, };
enum { JMY_ACK_END_ID				= 60000, };

// connection type enum
enum JmyConnType {
	JMY_CONN_TYPE_NONE				= 0,
	JMY_CONN_TYPE_ACTIVE			= 1,
	JMY_CONN_TYPE_PASSIVE			= 2,
};

// connection state enum
enum JmyConnState {
	JMY_CONN_STATE_NOT_USE			= 0,
	JMY_CONN_STATE_NOT_CONNECT		= 1,
	JMY_CONN_STATE_CONNECTING		= 2,
	JMY_CONN_STATE_CONNECTED		= 3,
	JMY_CONN_STATE_DISCONNECTING	= 4,
	JMY_CONN_STATE_DISCONNECTED		= 5,
};

// buffer max count
enum { JMY_CONN_BUFFER_MAX_COUNT	= 500000, };

// event type enum
enum JmyEventType {
	JMY_EVENT_NONE					= 0,
	JMY_EVENT_CONNECT				= 1,
	JMY_EVENT_DISCONNECT			= 2,
	JMY_EVENT_TICK					= 3,
	JMY_EVENT_TIMER					= 4,
	JMY_EVENT_MAX_COUNT,
};

// message id
enum { JMY_MIN_MESSAGE_ID = 1 };
enum { JMY_MAX_MESSAGE_ID = 65535 };

// drop condition for buffer
enum JmyBufferDropCondition {
	JMY_DROP_CONDITION_GREAT_BUFFER_COUNT	= 0x0002,
	JMY_DROP_CONDITION_GREAT_USED_BYTES		= 0x0004,
	JMY_DROP_CONDITION_TIMEOUT				= 0x0008,
	JMY_DROP_CONDITION_MANUAL				= 0x0010,
	JMY_DROP_CONDITION_COUNT,
};

// active close default timeout
enum { JMY_ACTIVE_CLOSE_CONNECTION_TIMEOUT = 30, };
