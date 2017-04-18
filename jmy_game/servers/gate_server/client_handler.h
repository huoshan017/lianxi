#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/bi_map.h"
#include "../../proto/src/error.pb.h"
#include <unordered_map>
#include "client_array.h"


struct ClientData {
	std::string enter_session;
	std::string reconn_session;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

enum { RECONN_SESSION_CODE_BUF_LENGTH = 16 };

struct JmyMsgInfo;
struct JmyEventInfo;
class ClientHandler
{
public:
	static bool init();
	static void clear();

	static int processEnterGame(JmyMsgInfo*);
	static int processLeaveGame(JmyMsgInfo*);
	static int processReconnect(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

	static bool newClientSession(const std::string& account, const std::string& session_code);
	static int sendEnterGameResponse2Client(int id);
	static int getClientStartId();
	static ClientInfo* getClientInfo(int user_id);
	static ClientInfo* getClientInfoByAccount(const std::string& account);
	static ClientInfo* getClientInfoByConnId(int conn_id);
	static void send_error(JmyTcpConnection* conn, ProtoErrorType);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static char session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];
	static BiMap<std::string, int> account_id_bimap_;
	static BiMap<int, int> connid_id_bimap_;
	static ClientArray client_array_;
};

#define GET_CLIENT_INFO(account) (ClientHandler::getClientInfo(account))
#define SEND_CLIENT_ERROR(conn, error) (ClientHandler::send_error(conn, error))
