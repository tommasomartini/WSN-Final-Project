#ifndef __STORAGE_NODE_MESSAGE_H__   
#define __STORAGE_NODE_MESSAGE_H__   

#include <vector>
#include "message.h"

using namespace std;
  
class StorageNodeMessage : public Message {
  // unsigned char xored_message_;
  // vector<int> sensor_ids_;
 public:
  unsigned char xored_message_;
  vector<unsigned int> sensor_ids_;
  StorageNodeMessage();
  StorageNodeMessage(unsigned char /*xored_message*/, vector<unsigned int> /*sensor_ids*/);

  unsigned char get_xored_message() {return xored_message_;}
  vector<unsigned int> get_sensor_ids() {return sensor_ids_;}
  // TODO remove?
  // unsigned char get_message() {return xored_message_;}
  // vector<int> get_sensor_ids() {return sensor_ids_;}
  // void set_message(unsigned char);
  // TODO remove?
};

#endif
