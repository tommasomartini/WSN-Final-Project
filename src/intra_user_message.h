#ifndef _INTRA_USER_MESSAGE_H_   
#define _INTRA_USER_MESSAGE_H_   

#include <vector>
#include "message.h"
#include "storage_node_message.h"

using namespace std;
  
class IntraUserMessage : public Message {

 public:
  vector<StorageNodeMessage> messages_;
  
  IntraUserMessage();

  unsigned int get_message_size();
};

#endif