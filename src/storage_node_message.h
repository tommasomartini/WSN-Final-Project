#ifndef __STORAGE_NODE_MESSAGE_H__   
#define __STORAGE_NODE_MESSAGE_H__   

#include <vector>

using namespace std;
  
class StorageNodeMessage {
  // unsigned char xored_message_;
  // vector<int> sensor_ids_;
 public:
  unsigned char xored_message_;
  vector<int> sensor_ids_;
  StorageNodeMessage(unsigned char /*xored_message*/, vector<int> /*sensor_ids*/);

  // TODO remove?
  // unsigned char get_message() {return xored_message_;}
  // vector<int> get_sensor_ids() {return sensor_ids_;}
  // void set_message(unsigned char);
  // TODO remove? 
};

#endif