
#ifndef USER_MESSAGE_H
#define	USER_MESSAGE_H

#include <vector>
#include "message.h"
#include "blacklist_message.h"

class User;
using namespace std;
  
class UserMessage : public Message {
  private:
    unsigned char xored_message_;
    vector <int> sensor_ids_;
    BlacklistMessage blacklist_;
    User* user_to_reply_;

  public:
  // Constructrs
  UserMessage();
  UserMessage(unsigned char, vector<int> ,BlacklistMessage);

  void set_xored_message(unsigned char);
  void set_sensor_id(vector<int>);
  void set_blacklist(BlacklistMessage);
  void set_user_to_reply(User*);
  
  unsigned char get_xored_message(){return xored_message_;}
  vector<int> get_sensor_id(){return sensor_ids_;}
  BlacklistMessage get_blacklist(){return blacklist_;}
  User* get_user_to_reply(){return user_to_reply_;}

};

#endif	/* USER_MESSAGE_H */

