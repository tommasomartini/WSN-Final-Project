
#ifndef USER_MESSAGE_H
#define	USER_MESSAGE_H

#include <vector>
#include "message.h"
#include "blacklist_message.h"
#include "storage_node_message.h"

class User;
using namespace std;
  
class UserMessage : public Message {
  private:
    vector<StorageNodeMessage> symbols_;
    //vector<unsigned char> xored_message_;
    //vector <unsigned int> sensor_ids_;
    BlacklistMessage blacklist_;
    User* user_to_reply_;

  public:
  // Constructrs
  UserMessage();
  //UserMessage(unsigned char, vector<unsigned int> ,BlacklistMessage);
  UserMessage(vector<StorageNodeMessage>, BlacklistMessage);

  void set_symbols(vector<StorageNodeMessage>);
  //void set_sensor_id(vector<unsigned int>);
  void set_blacklist(BlacklistMessage);
  void set_user_to_reply(User*);
  
  vector<StorageNodeMessage> get_symbols(){return symbols_;}
  BlacklistMessage get_blacklist(){return blacklist_;}
  User* get_user_to_reply(){return user_to_reply_;}

  unsigned int get_message_size();
};

#endif	/* USER_MESSAGE_H */

