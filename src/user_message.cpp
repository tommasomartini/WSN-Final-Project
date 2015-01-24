#include "user_message.h"
#include "blacklist_message.h"

UserMessage::UserMessage() {
    xored_message_;
    sensor_ids_=nullptr;
    blacklist_=BlacklistMessage();
}

UserMessage::UserMessage(unsigned char xored_message, int* sensor_ids,BlacklistMessage blacklist) {
    xored_message_= xored_message;
    sensor_ids_=sensor_ids;
    blacklist_=blacklist;
}

void UserMessage:: set_xored_message(unsigned char xored_message){
    xored_message_=xored_message;
}

void UserMessage:: set_sensor_id(int* sensor_id){
    sensor_ids_=sensor_id;
}

void UserMessage::set_blacklist(BlacklistMessage blacklist){
    blacklist_=blacklist;
}
