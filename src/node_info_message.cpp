#include "node_info_message.h"
#include "my_toolbox.h"

using namespace std;

NodeInfoMessage::NodeInfoMessage() {
	message_type_ = MessageTypes::message_type_node_info_for_user;

	node_id_ = 0;
	output_message_ = 0;
	blacklist_ = vector<unsigned int>();
//	msrs_info_ = map<unsigned int, unsigned int>();
}

NodeInfoMessage::NodeInfoMessage(unsigned int node_id,
		  unsigned char output_message,
		  std::vector<unsigned int> blacklist,
		  std::map<unsigned int, unsigned int> measures_info) {
	message_type_ = MessageTypes::message_type_node_info_for_user;

	node_id_ = node_id;
	output_message_ = output_message;
	blacklist_ = blacklist;
	msrs_info_ = measures_info;
}

unsigned int NodeInfoMessage::get_message_size() {
	return MyToolbox::get_bits_for_measure();
}
