#ifndef __NODE_DISPATCHER_H__   
#define __NODE_DISPATCHER_H__   

#include <map>

#include "node.h"

using namespace std;

class NodeDispatcher {
 public:
  static map<int, Node*>* sensors_map_ptr;
  static map<int, Node*>* storage_nodes_map_ptr;

  // static map<int, Node*>* sensors_map_ptr = new map<int, Node*>();
  // static map<int, Node*>* storage_nodes_map_ptr = new map<int, Nod
 };


#endif