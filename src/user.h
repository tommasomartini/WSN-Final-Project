/*
    Model for a user
*/

#ifndef __USER_H__   // if x.h hasn't been included yet...
#define __USER_H__   //   #define this so the compiler knows it has been included

#include <thread>
#include <vector>
#include <map>

#include "message.h"
#include "node.h"
#include "sensor_node.h"
#include "storage_node.h"
#include "storage_node_message.h"

using namespace std;

class User {

  int user_id_;
  bool user_on_ = false;
  thread thread_;
  double y_coord_;
  double x_coord_;
  vector<unsigned char> input_symbols_;  // list of the decoded measures
  // vector<Message> output_symbols_; // list of the xored messages retrieved from the storage nodes
  vector<StorageNodeMessage> output_symbols_; // list of the xored messages retrieved from the storage nodes
  vector<Node*> near_storage_nodes;
  vector<Node*> near_sensor_nodes;

 protected:
  typedef map<int, unsigned char>::iterator my_iterator;  // define my type of iterator

 public: 
  User(int /*user_id*/);
  User(int /*user_id*/, double /*initial_y_coord*/, double /*initial_x_coord*/);

  double get_y_coord() {return y_coord_;}
  double get_x_coord() {return x_coord_;}
  void add_near_storage_node(StorageNode*);
  void add_near_sensor_node(SensorNode*);
  void remove_near_storage_node(StorageNode*);
  void remove_near_sensor_node(SensorNode*);
  void switch_on_user();  
  void switch_off_user();
  void collect_data(Message /*xored_message*/); // for debugging only
  bool message_passing(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
  void move();  // the user "walks" randomly among the area
};

#endif