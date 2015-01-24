
#ifndef USER_MESSAGE_H
#define	USER_MESSAGE_H

#include <vector>
#include "message.h"
#include "blacklist_message.h"

using namespace std;
  
class UserMessage : public Message {
  private:
    unsigned char xored_message_;
    int* sensor_ids_;
    BlacklistMessage blacklist_;

  public:
  // Constructrs
  UserMessage();
  UserMessage(unsigned char, int* ,BlacklistMessage);

  void set_xored_message(unsigned char);
  void set_sensor_id(int*);
  void set_blacklist(BlacklistMessage);
  
  unsigned char get_xored_message(){return xored_message_;}
  int* get_sensor_id(){return sensor_ids_;}
  BlacklistMessage get_blacklist(){return blacklist_;}

};

#endif	/* USER_MESSAGE_H */

