#include "intra_user_message.h"
#include "my_toolbox.h"

using namespace std;  

IntraUserMessage::IntraUserMessage() {
  message_type_ = message_type_intra_user;
}

unsigned int IntraUserMessage::get_message_size() {
  return 0;// messages_.size() * MyToolbox::num_bits_for_measure_;
}
