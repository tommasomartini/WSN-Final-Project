#ifndef __MESSAGE_H_INCLUDED__   // if x.h hasn't been included yet...
#define __MESSAGE_H_INCLUDED__   //   #define this so the compiler knows it has been included

#include <vector>

using namespace std;

class Message {
  unsigned char message_;
  int source_sensor_id_;  // id of the sensor who generated this message
  int hop_counter_ = 0;
  
 public:
  enum MessageTypes {
    new_msg,
    update_msg
  };
  Message::MessageTypes message_type_ = new_msg;
  vector<int> header_;  // contains node_id_'s of the nodes whose measures have been xored inside msg_
  Message();  // to use for debug purposes only. Remove later on?
  Message(unsigned char /*message*/);
  void set_message(unsigned char /*message*/);
  void set_message_type(Message::MessageTypes);
  void set_source_sensor_id(int);
  unsigned char get_message() {return message_;}
  int get_source_sensor_id() {return source_sensor_id_;}
  Message::MessageTypes get_message_type() {return message_type_;}
  int get_hop_counter() {return hop_counter_;}
  int increase_hop_counter();
};

#endif
