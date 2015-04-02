#include "blacklist_message.h"
#include "my_toolbox.h"

BlacklistMessage::BlacklistMessage() {
    sensor_id_=nullptr;
    hop_counter_=0;
    length_=0;
}

BlacklistMessage::BlacklistMessage(unsigned int* list) {
    sensor_id_=list;
    hop_counter_=0;
    length_=0;
}

BlacklistMessage::BlacklistMessage(unsigned int* list, int length) {
    sensor_id_=list;
    hop_counter_=0;
    length_=length;
}

BlacklistMessage::BlacklistMessage(vector<unsigned int>* list) {
    sensor_ids_ptr_ = list;
    hop_counter_=0;
}

void BlacklistMessage::set_id_list(unsigned int* list){
    sensor_id_=list;
}

void BlacklistMessage::set_length(int length){
    length_=length;
}

int BlacklistMessage::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int BlacklistMessage::get_message_size() {
  return 0;
}
