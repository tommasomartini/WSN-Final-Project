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
#include "node_info_message.h"
// #include "user_message.h"

class Message;
class UserMessage;

using namespace std;

class User: public Node {

 public:
  User() : Node () {}
  User(unsigned int node_id) : Node (node_id) {}
  User(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}

  map<unsigned int, unsigned char> input_symbols_;  // list of the decoded measures and id of correspondent sensor
  vector<StorageNodeMessage> output_symbols_; // list of the xored messages retrieved from the storage nodes
  
  void collect_data(Message /*xored_message*/); // for debugging only

  vector<Event> move_user(int);  // the user "walks" randomly among the area
  vector<Event> move(); // different implementation of the random walk
  vector<Event> user_send_to_user(unsigned int /*sender user*/);  // Tom
  vector<Event> receive_data(NodeInfoMessage);	// Tom
  vector<Event> user_receive_data(UserMessage*);
  vector<Event> user_receive_data_from_user(int, Message*);
  vector<Event> try_retx(Message*, int /*next_node_id*/);
  vector<Event> try_retx_to_user(Message*, int /*next_node_id*/);
  
 protected:
  typedef map<unsigned int, unsigned char>::iterator my_iterator;  // define my type of iterator
  
 private:
  typedef MyToolbox::MyTime MyTime;

  double speed_;  // user's speed in meters / seconds
  int direction_; // number from 0 to 359, represents a degree
  map<unsigned int, NodeInfoMessage> nodes_info;
  map<unsigned int, unsigned int> updated_sensors_measures_;
  vector<unsigned int> pending_dispatches;  // another user asked me for my data, I didn't manage to send him all my data, so I moved and the transmission the that user is still pending

  vector<Event> send(Node*, Message*);
  bool message_passing(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
  void add_symbols(vector<StorageNodeMessage>, User*);
};

#endif
