#pragma once

enum JmySessionType {
	SESSION_TYPE_NONE = 0,
	SESSION_TYPE_AGENT = 1,
	SESSION_TYPE_SERVER = 2,
	SESSION_TYPE_CLIENT = 3,
};

enum { JMY_ACK_START_ID = 1, };
enum { JMY_ACK_END_ID = 60000, };
