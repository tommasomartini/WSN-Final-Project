#ifndef USER_INFO_MESSAGE_H_
#define USER_INFO_MESSAGE_H_

#include "message.h"
#include "my_toolbox.h"
#include <vector>
#include <map>

class UserInfoMessage : public Message {

 private:
  typedef MyToolbox::MeasureKey MeasureKey;
  typedef MyToolbox::OutputSymbol OutputSymbol;

 public:
  std::map<unsigned int, OutputSymbol> symbols_;
  std::vector<unsigned int> dead_sensors_;

  UserInfoMessage();
  UserInfoMessage(std::map<unsigned int, OutputSymbol>, std::vector<unsigned int>);

  unsigned int get_message_size();
};

#endif /* USER_INFO_MESSAGE_H_ */
