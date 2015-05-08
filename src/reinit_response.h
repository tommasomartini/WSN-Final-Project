#ifndef REINIT_RESPONSE_H_
#define REINIT_RESPONSE_H_

#include <vector>
#include <map>
#include "message.h"

class ReinitResponse : public Message {
 public:
  unsigned char xored_measure_;
  std::map<unsigned int, unsigned int> last_measures_;
  std::vector<unsigned int> blacklist_;
//  std::vector<unsigned int> updated_neighbour_list;

  ReinitResponse();

  unsigned int get_message_size();
};

#endif /* REINIT_RESPONSE_H_ */
