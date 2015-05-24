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
  std::vector<MeasureKey> sources_;	// keys of the measures composing this xored measure
  std::vector<MeasureKey> outdated_measures_;	// keys of the (pure) measures I want to know
  std::vector<unsigned int> dead_sensors_;	// ids of the dead sensors

  NodeInfoMessage();
  NodeInfoMessage(unsigned int /*node id*/,
  		  unsigned char /*output message*/,
  		  std::vector<MeasureKey> /*outdated measures*/,
  		  std::vector<MeasureKey> /*sources*/);
  NodeInfoMessage(unsigned int /*node id*/,
    		  unsigned char /*output message*/,
			  std::vector<MeasureKey> /*sources*/,
    		  std::vector<MeasureKey> /*outdated measures*/,
    		  std::vector<unsigned int> /*dead sensors*/);

  unsigned int get_message_size();
};


#endif /* NODE_INFO_MESSAGE_H_ */
