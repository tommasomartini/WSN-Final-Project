#include <vector>

#include "storage_node_message.h"

StorageNodeMessage::StorageNodeMessage(unsigned char xored_message, vector<int> sensor_ids) {
  xored_message_ = xored_message;
  sensor_ids_ = sensor_ids;
}

// TODO to remove? 
// void StorageNodeMessage::set_message(unsigned char message) {
//   xored_message_ = message;
// }