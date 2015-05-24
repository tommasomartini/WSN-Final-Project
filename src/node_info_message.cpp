#include "node_info_message.h"
#include "my_toolbox.h"

using namespace std;

NodeInfoMessage::NodeInfoMessage() {
	message_type_ = MessageTypes::message_type_node_info_for_user;

	node_id_ = 0;
	output_message_ = 0;
}

NodeInfoMessage::NodeInfoMessage(unsigned int node_id,
		  unsigned char output_message,
		  std::vector<MyToolbox::MeasureKey> sources,
		  std::vector<MeasureKey> outdated_measures,
		  std::vector<unsigned int> dead_sensors) {
	message_type_ = MessageTypes::message_type_node_info_for_user;

	node_id_ = node_id;
	output_message_ = output_message;
	sources_ = sources;
	outdated_measures_ = outdated_measures;
	dead_sensors_ = dead_sensors;
}

unsigned int NodeInfoMessage::get_message_size() {
	return MyToolbox::num_bits_for_measure_;
}
