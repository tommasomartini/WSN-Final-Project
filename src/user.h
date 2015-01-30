/*
    Model for a user
*/

#ifndef __USER_H__   
#define __USER_H__   

#include <vector>
#include <map>

// #include "message.h"
#include "node.h"
// #include "sensor_node.h"
// #include "storage_node.h"
#include "storage_node_message.h"
#include "event.h"
// #include "user_message.h"

class Message;
class UserMessage;

using namespace std;

class User: public Node {

 public:
  User() : Node () {}
  User(unsigned int node_id) : Node (node_id) {}
  User(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}
  
  unsigned int user_id_;
 
  map<unsigned int, unsigned char> input_symbols_;  // list of the decoded measures and id of correspondent sensor
  // vector<Message> output_symbols_; // list of the xored messages retrieved from the storage nodes
  vector<StorageNodeMessage> output_symbols_; // list of the xored messages retrieved from the storage nodes
  
  void collect_data(Message /*xored_message*/); // for debugging only
  bool message_passing(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
  vector<Event> move_user(int);  // the user "walks" randomly among the area
  vector<Event> user_send_to_user(UserMessage*, int);
  vector<Event> user_receive_data(int, UserMessage*);
  void add_symbols(vector<StorageNodeMessage>, User*);
  
 protected:
  typedef map<unsigned int, unsigned char>::iterator my_iterator;  // define my type of iterator
  
 private:
  typedef MyToolbox::MyTime MyTime;

  vector<Event> send(Node*, Message*);
};

#endif