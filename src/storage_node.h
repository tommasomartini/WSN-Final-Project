#ifndef __STORAGE_NODE_H_INCLUDED__   // if x.h hasn't been included yet...
#define __STORAGE_NODE_H_INCLUDED__   //   #define this so the compiler knows it has been included

#include <vector>
#include <map>
#include <thread>
#include <chrono>

#include "node.h"
#include "message.h"
#include "storage_node_message.h"

using namespace std;

class StorageNode: public Node {
  thread thread_;
  bool running_ = false;
  // bool ready_to_be_queried_ = false;  // states whether a node has already collected d = degree source packets
  int LT_degree_; // number of xored message
  unsigned char xored_message_ = 0;
  vector<unsigned char> xored_msgs_;
  vector<int> header_;  // contains node_id_'s of the nodes whose measure has been xored inside msg_
  map<int, Message> past_messages_; // this map contains the last msg received from each sensor (no matter whther I accepted it or not!)
 public:
  typedef std::chrono::milliseconds Interval;

  StorageNode (int node_id) : Node (node_id) {}
  StorageNode (int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}

  // void set_xored_message(Message);
  unsigned char get_xored_message() {return xored_message_;}
  void activate_node();
  void stop_node();
  void manage_message(Message); // called by another node when it wants to transfer a msg to this node
  StorageNodeMessage retrieve_data();
};

#endif