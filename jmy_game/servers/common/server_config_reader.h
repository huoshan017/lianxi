#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"

class ServerConfigReader
{
public:
	ServerConfigReader();
	~ServerConfigReader();

	bool loadJson(const char* jsonpath);
	void close();

	struct ServerNode {
		std::string name;
		int id;
		std::string ip;
		short port;
		int max_conn;
		bool support_reconnect;
	};
	struct ClientNode {
		std::string name;
		int id;
		std::string ip;
		short port;
	};
	const ServerNode* getServerNode() const { return &server_node_; }
	const ClientNode* getClientNode(int index) const { if (index>=MaxClientNodeNum-1)return nullptr; return &client_nodes_[index]; }

private:
	bool parseServerNode();
	bool parseClientNodes();

private:
	rapidjson::Document doc_;
	ServerNode server_node_;
	enum { MaxClientNodeNum=32, };
	ClientNode client_nodes_[MaxClientNodeNum];
};
