#pragma once

enum ServerType {
	SERVER_TYPE_NONE = 0,
	SERVER_TYPE_LOGIN = 1,
	SERVER_TYPE_GATE = 2,
	SERVER_TYPE_CONFIG = 3,
	SERVER_TYPE_GAME = 4,
	SERVER_TYPE_DB = 5,
	SERVER_TYPE_COUNT,
};

// login server id range
enum { LOGIN_SERVER_MIN_ID = 10001 };
enum { LOGIN_SERVER_MAX_ID = 10100 };

// gate server id range
enum { GATE_SERVER_MIN_ID = 20001 };
enum { GATE_SERVER_MAX_ID = 20100 };

// config server id range
enum { CONFIG_SERVER_MIN_ID = 30001 };
enum { CONFIG_SERVER_MAX_ID = 30001 };

// game server id range
enum { GAME_SERVER_MIN_ID = 40001 };
enum { GAME_SERVER_MAX_ID = 40100 };

// db server id range
enum { DB_SERVER_MIN_ID = 50001 };
enum { DB_SERVER_MAX_ID = 50100 };

// server_id devide
enum { SERVER_ID_DEVIDE = 10000 };

// resend: max cached send buffer count
enum { RESEND_MAX_CACHED_SEND_BUFFER_COUNT = 100 };
// 
enum { RESEND_ACK_RECV_COUNT = 10 };

enum { CLIENT_RECONNECT_INTERVAL_SECS = 3 };
