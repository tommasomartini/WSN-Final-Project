#include <vector>

#include "storage_node_message.h"

StorageNodeMessage::StorageNodeMessage(){
    message_type_=message_type_measures_for_user;
    xored_message_=0;
    sensor_ids_;
}
StorageNodeMessage::StorageNodeMessage(unsigned char xored_message, vector<unsigned int> sensor_ids) {
  message_type_ = message_type_measures_for_user;
  xored_message_ = xored_message;
  sensor_ids_ = sensor_ids;
}

// TODO to remove? 
// void StorageNodeMessage::set_message(unsigned char message) {
//   xored_message_ = message;
// }