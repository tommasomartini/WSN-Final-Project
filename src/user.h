/*
    Model for a user
*/

#ifndef __USER_H__   // if x.h hasn't been included yet...
#define __USER_H__   //   #define this so the compiler knows it has been included

#include <vector>
#include <map>

#include "message.h"
#include "node.h"
#include "sensor_node.h"
#include "storage_node.h"
#include "storage_node_message.h"
#include "event.h"

using namespace std;

class User:  public Node{

  public:
  User() : Node () {}
  User(int node_id) : Node (node_id) {}
  User(int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}
  
  int user_id_;
  bool user_on_ = true;
 
  map <unsigned char, int> input_symbols_;  // list of the decoded measures and id of correspondent sensor
  // vector<Message> output_symbols_; // list of the xored messages retrieved from the storage nodes
  vector<StorageNodeMessage> output_symbols_; // list of the xored messages retrieved from the storage nodes

 // Debugging
  int do_action() {return 5;}
  
  void switch_on_user();  
  void switch_off_user();
  void collect_data(Message /*xored_message*/); // for debugging only
  bool message_passing(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
  vector<Event> move_user(int);  // the user "walks" randomly among the area
  vector<Event> user_send_to_user(User*, int );
  
   protected:
  typedef map<int, unsigned char>::iterator my_iterator;  // define my type of iterator
  
};

#endif