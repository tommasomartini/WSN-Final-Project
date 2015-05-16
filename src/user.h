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

class User: public Node {

 private:
  struct OutputSymbol {
    unsigned char xored_msg_;
    std::vector<MyToolbox::MeasureKey> sources_;
    OutputSymbol(unsigned char xored_msg, std::vector<MyToolbox::MeasureKey> sources) {
      xored_msg_ = xored_msg;
      sources_ = sources;
    }
  };

 public:
  User() : Node () {}
  User(unsigned int node_id) : Node (node_id) {}
  User(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}

  std::map<unsigned int, unsigned char> input_symbols_;  // list of the decoded measures and id of corresponding sensor	// TODO to remove
  std::vector<StorageNodeMessage> output_symbols2_; // list of the xored messages retrieved from the storage nodes	// TODO to remove
  
  void collect_data(Message /*xored_message*/); // for debugging only
  std::vector<Event> move_user(int);  // the user "walks" randomly among the area
  std::vector<Event> move(); // different implementation of the random walk
  std::vector<Event> user_send_to_user(unsigned int /*sender user*/);  // Tom
  std::vector<Event> receive_data(NodeInfoMessage);	// Tom
  std::vector<Event> user_receive_data(UserMessage*);
  std::vector<Event> user_receive_data_from_user(int, Message*);
  std::vector<Event> try_retx(Message*, int /*next_node_id*/);
  std::vector<Event> try_retx_to_user(Message*, int /*next_node_id*/);

 protected:
  typedef std::map<unsigned int, unsigned char>::iterator my_iterator;  // define my type of iterator
  
 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  double speed_;  // user's speed in meters / seconds
  int direction_; // number from 0 to 359, represents a degree
  std::map<unsigned int, NodeInfoMessage> nodes_info_;	// output symbols
  std::map<unsigned int, OutputSymbol> nodes_info2_;	// output symbols
  std::map<unsigned int, unsigned char> decoded_symbols_;	// input symbols
  std::map<MeasureKey, unsigned char> decoded_symbols2_;	// input symbols
  std::map<unsigned int, unsigned int> updated_sensors_measures_;	// for each sensor, the measure of its I consider the most recent
  std::map<unsigned int, unsigned char> blacklist_;		// dead sensors and relative measures
  std::map<MeasureKey, unsigned char> outdated_measures_;		// old measures
  std::vector<unsigned int> pending_dispatches;  // another user asked me for my data, I didn't manage to send him all my data, so I moved and the transmission the that user is still pending

  std::vector<Event> send(Node*, Message*);
  bool message_passing(); // implements the message passing procedure
  bool message_passing2(); // implements the message passing procedure
  bool message_passing3(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
  void add_symbols(std::vector<StorageNodeMessage>, User*);	// TODO what for??
};

#endif
