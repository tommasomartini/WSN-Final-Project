#include "blacklist_message.h"
#include "my_toolbox.h"

BlacklistMessage::BlacklistMessage() {
    sensor_id_=nullptr;
    hop_counter_=0;
    length_=0;
}

BlacklistMessage::BlacklistMessage(int* list) {
    sensor_id_=list;
    hop_counter_=0;
    length_=0;

}

BlacklistMessage::BlacklistMessage(int* list, int length) {
    sensor_id_=list;
    hop_counter_=0;
    length_=length;

}

void BlacklistMessage::set_id_list(int* list){
    sensor_id_=list;
}

void BlacklistMessage::set_length(int length){
    length_=length;
}

int BlacklistMessage::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int BlacklistMessage::get_message_size() {
  unsigned int message_size = 0;
  // int num_measures = measure_type_ == measure_type_new ? 1 : 2; // new or update measure? It may contain 1 or 2 measures
  message_size += MyToolbox::get_bits_for_id();
  message_size += MyToolbox::get_bits_for_hop_counter();
  message_size += MyToolbox::get_bits_for_phy_mac_overhead();
  message_size = message_size*length_;
  return message_size;
}