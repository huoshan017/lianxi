#pragma once

enum { JMY_ACK_START_ID = 1, };
enum { JMY_ACK_END_ID = 60000, };

enum JmyConnType {
	JMY_CONN_TYPE_ACTIVE	= 0,
	JMY_CONN_TYPE_PASSIVE	= 1,
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
