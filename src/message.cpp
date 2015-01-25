#include "message.h"

/**************************************
    Setters
**************************************/
void Message::set_receiver_node_id(unsigned int receiver_node_id) {
  receiver_node_id_ = receiver_node_id;
}

void Message::set_sender_node_id(unsigned int sender_node_id) {
  sender_node_id_ = sender_node_id;
}
