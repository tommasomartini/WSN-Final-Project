#include <iostream>
#include <map>
#include <algorithm>    // std::max
#include <iostream>

#include "storage_node.h"
#include "my_toolbox.h"

  
using namespace std;

// void StorageNode::activate_node() {
//   LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();
// }

vector<Event> manage_measure(Measure measure) {
  vector<Event> new_events;

  return new_events;
} 

void StorageNode::manage_message() {
/*
  if (running_) {
    cout << "Storage " << node_id_ << "-> ON, received " << static_cast<unsigned>(message.get_message()) << endl;
    // return;
  } else {
    cout << "Storage " << node_id_ << "-> OFF, received " << static_cast<unsigned>(message.get_message()) << endl;
    return;
  }

  cout << "Storage " << node_id_ << "-> processing " << static_cast<unsigned>(message.get_message()) << endl;
  
  if (message.get_message_type() == Message::MessageTypes::new_msg) { // new measure from a sensor
    int source_id = message.get_source_sensor_id();
    cout << "Storage " << node_id_ << "-> new measure from sensor " << source_id << endl;
    if (past_messages_.find(source_id) != past_messages_.end()) {  // already received a msg from this sensor
      unsigned char previous_msg = past_messages_.find(source_id)->second.get_message();
      cout << "Storage " << node_id_ << "-> last measure from sensor " << source_id << " was " << static_cast<unsigned>(previous_msg) << endl;
      if (previous_msg != message.get_message()) {  // new message from this sensor: accept wp degree / k
        cout << "Storage " << node_id_ << "-> last measure from sensor " << source_id << " was different" << endl;
        /*
          Sampling interval must be larger smaller than the smallest interval probability.
          I have only 2 intervals: [0, d/k] and [d/k, 1]. Call the smaller min_int = min(d/k, (1 - d/k))
          Divide the interval [0, 1] in M intervals with length 1/M.

          1/M < min_int => M > 1 / min_int = 1 / min(d/k, (1 - d/k)) = max(1 / (d/k), 1 / (1 - d/k)) = 
          = max(k/d, k/(k - d)).

          Then we can take M = 10 * max(k/d, k/(k - d))

          Choose randomly one element between 0 and M - 1 with "rand() % M".
          Divide it by (M - 1) to normaliz between 0 and 1.
        */
/*        int k = MyToolbox::get_k();
        int d = LT_degree_;
        double prob = 1;
        if (d != k) {
          int M = 10 * max(k/d, k/(k - d)); // if d == k this gives a zero denominator
          prob = (rand() % M) / (double)(M - 1);
        }
        printf("Storage %i-> k=%i, d=%i, prob=%3.2f\n", node_id_, k, d, prob);
        // accept the new msg with probability d/k
        if (prob <= LT_degree_ / k) { // accept it!
          xored_message_ = xored_message_ ^ message.get_message();  // save the new xored message
          cout << "Storage " << node_id_ << "-> accept new msg from sensor " << source_id << ". Now stores " << static_cast<unsigned>(xored_message_) << endl;
          past_messages_.find(source_id)->second = message; // save this message as the last received message from sensor source_id
        }
      }
    } else {  // first time I receive a message from this sensor
      cout << "Storage " << node_id_ << "-> never received a msg from sensor " << source_id << endl;

      int k = MyToolbox::get_k();
      int d = LT_degree_;
      double prob = 1;
      if (d != k) {
        int M = 10 * max(k/d, k/(k - d)); // if d == k this gives a zero denominator
        prob = (rand() % M) / (double)(M - 1);
      }
      printf("Storage %i-> k=%i, d=%i, prob=%3.2f\n", node_id_, k, d, prob);
      // accept the new msg with probability d/k
      if (prob <= LT_degree_ / k) { // accept it!
        xored_message_ = xored_message_ ^ message.get_message();  // save the new xored message
        cout << "Storage " << node_id_ << "-> accept new msg from sensor " << source_id << ". Now stores " << static_cast<unsigned>(xored_message_) << endl;
        header_.push_back(source_id);  // put in the header the id of this new message's source
        past_messages_.find(source_id)->second = message; // save this message as the last received message from sensor source_id
      }
    }
  } else if (message.get_message_type() == Message::MessageTypes::update_msg) { // update measure from a sensor
    int source_id = message.get_source_sensor_id();
    cout << "Storage " << node_id_ << "-> update from sensor " << source_id << endl;
    if (past_messages_.find(source_id) != past_messages_.end()) {  // already received a msg from this sensor
      xored_message_ = xored_message_ ^ message.get_message();  // save the new xored message
      cout << "Storage " << node_id_ << "-> accept update from sensor " << source_id << ". Now stores " << static_cast<unsigned>(xored_message_) << endl;
      past_messages_.find(source_id)->second.set_message(past_messages_.find(source_id)->second.get_message() ^ message.get_message()); // save this message as the last received message from sensor source_id
    }
  } else {
    cout << "Storage " << node_id_ << "-> Error! Neither new_msg nor update " << endl;
  }

  message.increase_hop_counter();
  int hop_limit = MyToolbox::get_max_msg_hops();
  if (message.get_hop_counter() < hop_limit) {  // the message has to be forwarded again
    int next_node_index = rand() % near_storage_nodes.size();
    StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
    cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(message.get_message()) << " to node " << next_node->get_node_id() << endl;
    next_node->manage_message(message);
  } else {
    cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(message.get_message()) << " is not to be forwarded" << endl;
  }
  */
}