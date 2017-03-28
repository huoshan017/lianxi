#pragma once

const unsigned int DEFAULT_SEND_BUFFER_SIZE = 4096;
const unsigned int MAX_SEND_BUFFER_SIZE = 64*1024;
const unsigned int DEFAULT_RECV_BUFFER_SIZE = 4096;
const unsigned int MAX_RECV_BUFFER_SIZE = 64*1024;
const unsigned int DEFAULT_MAX_SEND_BUFFER_COUNT = 100;
const unsigned int DEFAULT_MAX_RECV_BUFFER_COUNT = 100;

enum { JMY_ACK_START_ID = 1, };
enum { JMY_ACK_END_ID = 60000, };

enum JmyConnType {
	JMY_CONN_TYPE_NONE		= 0,
	JMY_CONN_TYPE_ACTIVE	= 1,
	JMY_CONN_TYPE_PASSIVE	= 2,
};

enum JmyConnState {
	JMY_CONN_STATE_NOT_USE			= 0,
	JMY_CONN_STATE_NOT_CONNECT		= 1,
	JMY_CONN_STATE_CONNECTING		= 2,
	JMY_CONN_STATE_CONNECTED		= 3,
	JMY_CONN_STATE_DISCONNECTING	= 4,
	JMY_CONN_STATE_DISCONNECTED		= 5,
};

enum { JMY_CONN_BUFFER_MAX_COUNT = 500000, };

enum JmyBufferDropCondition {
	DropConditionGreatBufferCount	= 0x0002,
	DropConditionGreatUsedBytes		= 0x0004,
	DropConditionTimeOut			= 0x0008,
	DropConditionManual				= 0x0010,
	DropConditionCount,
};
