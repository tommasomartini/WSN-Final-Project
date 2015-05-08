#ifndef REINIT_QUERY_H_
#define REINIT_QUERY_H_

#include <vector>
#include "message.h"

class ReinitQuery : public Message {
public:
//  std::vector<unsigned int> neighbours_to_query_;

  ReinitQuery();
//  ReinitQuery(std::vector<unsigned int> /*neighbours_to_query*/);

  unsigned int get_message_size() {return 0;}	// it is just a "ping"
};

#endif /* REINIT_QUERY_H_ */
