#ifndef __MESSAGE_H__   
#define __MESSAGE_H__   

#include <string>

class Message {
 public:
  enum MessageTypes {
    message_type_measure,
    message_type_blacklist,
    message_type_remove_measure,
    message_type_ping,
    message_type_measures_for_user,	// FIXME maybe we can remove this... better to check
	message_type_node_info_for_user,
    message_type_intra_user,
	message_type_reinit_query,
	message_type_reinit_response,
    message_type_new_node,	// FIXME ancora necessario?
    message_type_user_to_user,  // TODO: per cosa lo uso?
  } message_type_;

  virtual ~ Message() {}
  // Message() {message_type_ = message_type_ping;};

  static std::string int2msgtype(int);	// TODO just for debug

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
