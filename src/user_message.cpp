#include <vector>
#include "user_message.h"
#include "blacklist_message.h"
#include "storage_node_message.h"
#include "my_toolbox.h"

UserMessage::UserMessage(){
    symbols_;
    blacklist_ ;
} 

UserMessage::UserMessage(vector<StorageNodeMessage> symbol, BlacklistMessage blacklist) {
    symbols_=symbol;
    blacklist_=blacklist;
}


void UserMessage::set_symbols(vector<StorageNodeMessage> symbol){
    symbols_ = symbol;
}

void UserMessage::set_blacklist(BlacklistMessage blacklist){
    blacklist_=blacklist;
}

void UserMessage::set_user_to_reply(User* user){
    user_to_reply_=user;
}

unsigned int UserMessage::get_message_size() {
  unsigned int message_size = 0;
  // int num_measures = measure_type_ == measure_type_new ? 1 : 2; // new or update measure? It may contain 1 or 2 measures
  message_size += MyToolbox::get_bits_for_measure(); 
  message_size += MyToolbox::get_bits_for_id(); 
  message_size += MyToolbox::get_bits_for_phy_mac_overhead();
  message_size = message_size * symbols_.size();
  message_size += blacklist_.get_message_size(); 
  return message_size;
}