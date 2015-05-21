#ifndef NODE_INFO_MESSAGE_H_
#define NODE_INFO_MESSAGE_H_

#include "message.h"
#include "my_toolbox.h"
#include <vector>
#include <map>

class NodeInfoMessage : public Message {

 private:
  typedef MyToolbox::MeasureKey MeasureKey;
//  typedef MyToolbox::MeasureData MeasureData;

 public:
  unsigned int node_id_;
  unsigned char output_message_;
  std::vector<unsigned int> blacklist_;	// TODO to remove
  std::vector<MeasureKey> outdated_measures_;
  std::map<unsigned int, unsigned int> msrs_info_;	// TODO to remove
  std::vector<MeasureKey> sources_;	// keys of the measures composing this xored measure

  NodeInfoMessage();
  // TODO to remove
  NodeInfoMessage(unsigned int /*node id*/,
		  unsigned char /*output message*/,
		  std::vector<unsigned int> /*blacklist*/,
		  std::map<unsigned int, unsigned int> /*measures info*/);
  NodeInfoMessage(unsigned int /*node id*/,
  		  unsigned char /*output message*/,
  		  std::vector<MeasureKey> /*outdated measures*/,
  		  std::vector<MeasureKey> /*sources*/);

  unsigned int get_message_size();
};


#endif /* NODE_INFO_MESSAGE_H_ */
