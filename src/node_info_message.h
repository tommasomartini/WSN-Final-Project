/*
 * node_info_message.h
 *
 *  Created on: 08/mag/2015
 *      Author: tommaso
 */

#ifndef NODE_INFO_MESSAGE_H_
#define NODE_INFO_MESSAGE_H_

#include "message.h"
#include <vector>
#include <map>

class NodeInfoMessage : public Message {
 public:
  unsigned int node_id_;
  unsigned char output_message_;
  std::vector<unsigned int> blacklist_;
  std::map<unsigned int, unsigned int> msrs_info_;

  NodeInfoMessage();
  NodeInfoMessage(unsigned int /*node id*/,
		  unsigned char /*output message*/,
		  std::vector<unsigned int> /*blacklist*/,
		  std::map<unsigned int, unsigned int> /*measures info*/);

  unsigned int get_message_size();
};


#endif /* NODE_INFO_MESSAGE_H_ */
