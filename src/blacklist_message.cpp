#include "blacklist_message.h"
#include "my_toolbox.h"

using namespace std;

BlacklistMessage::BlacklistMessage() {
  hop_counter_ = 0;
}


BlacklistMessage::BlacklistMessage(vector<unsigned int> blacklist) {
  hop_counter_ = 0;

  sensor_ids_ = blacklist;
}

int BlacklistMessage::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int BlacklistMessage::get_message_size() {
  return 0;
}
