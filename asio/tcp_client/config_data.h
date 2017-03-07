#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

#define s_log_cate_name "client_log"

static JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

const JmyConnectorConfig test_connector_config = {
	{ 1024, 1024}, s_test_handlers, sizeof(s_test_handlers)/sizeof(s_test_handlers), true, true,
};
