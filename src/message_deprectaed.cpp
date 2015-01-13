#include "message.h"

Message::Message() {
	message_ = 0;
}

Message::Message(unsigned char message) {
	message_ = message;
}

void Message::set_message(unsigned char message) {
  message_ = message;
}

void Message::set_message_type(Message::MessageTypes message_type) {
  message_type_ = message_type;
}

void Message::set_source_sensor_id(int source_sensor_id) {
  source_sensor_id_ = source_sensor_id;
}

int Message::increase_hop_counter() {
  return ++hop_counter_;
}