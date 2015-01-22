#ifndef __MESSAGE_H__   
#define __MESSAGE_H__   

// using namespace std;

class Message {
 public:
  enum MessageTypes {
    message_type_measure,
    message_type_blacklist,
    message_type_remove_measure,
    message_type_ping,
    message_type_new_node
  } message_type_;

  int get_receiver_node_id() {return receiver_node_id_;}
  void set_receiver_node_id(int);

 private:
  int receiver_node_id_;
};

#endif
