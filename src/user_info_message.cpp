#include "user_info_message.h"

using namespace std;

UserInfoMessage::UserInfoMessage() {
	message_type_ = MessageTypes::message_type_user_info_for_user;
}

UserInfoMessage::UserInfoMessage(map<unsigned int, OutputSymbol> symbols, std::vector<unsigned int> dead_sensors) {
	message_type_ = MessageTypes::message_type_user_info_for_user;

	symbols_ = symbols;
	dead_sensors_ = dead_sensors;
}

unsigned int UserInfoMessage::get_message_size() {
	return MyToolbox::num_bits_for_measure_ * symbols_.size();
}
