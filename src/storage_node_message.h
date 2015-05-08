#ifndef __STORAGE_NODE_MESSAGE_H__   
#define __STORAGE_NODE_MESSAGE_H__   

#include <vector>
#include <map>
#include "message.h"

using namespace std;
  
class StorageNodeMessage : public Message {
  // unsigned char xored_message_;
  // vector<int> sensor_ids_;
 public:
  unsigned char xored_message_;
  vector<unsigned int> sensor_ids_;	// FIXME this can be removed!
  map<unsigned int, unsigned int> msrs_info_;

  StorageNodeMessage();
  StorageNodeMessage(unsigned char /*xored_message*/, vector<unsigned int> /*sensor_ids*/);	// FIXME to be deprecated
  StorageNodeMessage(unsigned char /*xored_message*/, map<unsigned int, unsigned int> /*msr_info*/);

  unsigned char get_xored_message() {return xored_message_;}	// FIXME remove this!
  vector<unsigned int> get_sensor_ids() {return sensor_ids_;}	// FIXME remove this!
  // TODO remove?
  // unsigned char get_message() {return xored_message_;}
  // vector<int> get_sensor_ids() {return sensor_ids_;}
  // void set_message(unsigned char);
  // TODO remove?
};

#endif
