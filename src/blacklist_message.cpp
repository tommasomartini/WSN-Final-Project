#include "blacklist_message.h"

BlacklistMessage::BlacklistMessage() {
    sensor_id_=NULL;
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
