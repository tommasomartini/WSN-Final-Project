#include "storage_node_message.h"

using namespace std;

StorageNodeMessage::StorageNodeMessage() {
    message_type_= message_type_measures_for_user;
    xored_message_= 0;
    sensor_ids_ = vector<unsigned int>();
}

StorageNodeMessage::StorageNodeMessage(unsigned char xored_message, map<unsigned int, unsigned int> msrs_info) {
	message_type_ = message_type_measures_for_user;
	xored_message_ = xored_message;
	msrs_info_ = msrs_info;
//	sensor_ids_ = nullptr;	// FIXME to be deprecated
}

// FIXME to be deprecated
StorageNodeMessage::StorageNodeMessage(unsigned char xored_message, vector<unsigned int> sensor_ids) {
  message_type_ = message_type_measures_for_user;
  xored_message_ = xored_message;
  sensor_ids_ = sensor_ids;
//  msrs_info_ = nullptr;
}


// TODO to remove? 
// void StorageNodeMessage::set_message(unsigned char message) {
//   xored_message_ = message;
// }
