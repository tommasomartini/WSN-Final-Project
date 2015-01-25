#ifndef __MESSAGE_H__   
#define __MESSAGE_H__   

using namespace std;

class Message {
 public:
  enum MessageTypes {
    message_type_measure,
    message_type_blacklist,
    message_type_remove_measure,
    message_type_ping,
    message_type_measures_for_user,
    message_type_new_node
  } message_type_;

  // Message() {message_type_ = message_type_ping;};

  // getters
  unsigned int get_receiver_node_id() {return receiver_node_id_;}
  unsigned int get_sender_node_id() {return sender_node_id_;}

  // setters
  void set_receiver_node_id(unsigned int);
  void set_sender_node_id(unsigned int);

  // functions
  virtual unsigned int get_message_size() {return 0;} // number of bits this message is made of

 private:
  unsigned int receiver_node_id_;
  unsigned int sender_node_id_;
};

#endif
