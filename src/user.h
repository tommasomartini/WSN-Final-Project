#ifndef __USER_H__   
#define __USER_H__   

#include <vector>
#include <map>

#include "node.h"

class Message;
class NodeInfoMessage;

class User: public Node {

 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  struct OutputSymbol {
    unsigned char xored_msg_;
    std::vector<MeasureKey> sources_;	// keys of the measures which compose the xor
    std::vector<MeasureKey> outdated_;	// keys of the outdated measures (subset of sources)
    OutputSymbol(unsigned char xored_msg, std::vector<MeasureKey> sources, std::vector<MeasureKey> outdated) {
      xored_msg_ = xored_msg;
      sources_ = sources;
      outdated_ = outdated;
    }
  };

 public:
  User() : Node () {}
  User(unsigned int node_id) : Node (node_id) {}
  User(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {}

  std::map<unsigned int, unsigned char> input_symbols_;  // list of the decoded measures and id of corresponding sensor	// TODO to remove
  
  std::vector<Event> move(); // different implementation of the random walk
  std::vector<Event> user_send_to_user(unsigned int /*sender user*/);  // Tom
  std::vector<Event> receive_data(NodeInfoMessage*);	// Tom
  std::vector<Event> user_receive_data_from_user(int, Message*);
  std::vector<Event> try_retx(Message*, int /*next_node_id*/);
  std::vector<Event> try_retx_to_user(Message*, int /*next_node_id*/);

 protected:
  typedef std::map<unsigned int, unsigned char>::iterator my_iterator;  // define my type of iterator
  
 private:
  bool decoding_succeeded = false;	// when message passing succeeds I ignore other messages sent by the cache (I could interrogate 100 caches but decode the measures after 10 answers!)
  double speed_;  // user's speed in meters / seconds
  int direction_; // number from 0 to 359, represents a degree
  std::map<unsigned int, OutputSymbol> nodes_info_;	// output symbols <cache id, info about the cache and the output symbol>
  std::map<MeasureKey, unsigned char> decoded_symbols_;	// input symbols eventually decoded (contains ALL the measures I have received, also the older ones)
  std::map<unsigned int, unsigned int> updated_sensors_measures_;	// for each sensor, the measure of its I consider the most recent
  std::vector<unsigned int> dead_sensors_;		// dead sensors ids
//  std::map<MeasureKey, unsigned char> outdated_measures_;		// old measures
  std::vector<unsigned int> pending_dispatches;  // another user asked me for my data, I didn't manage to send him all my data, so I moved and the transmission the that user is still pending

  std::vector<Event> send(unsigned int, Message*);
  std::vector<Event> re_send(Message*);
  bool message_passing(); // implements the message passing procedure
  bool CRC_check(Message /*message*/);  // check with the CRC field whether the message is valid
};

#endif
