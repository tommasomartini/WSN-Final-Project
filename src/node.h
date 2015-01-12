#ifndef __NODE_H__   
#define __NODE_H__  

#include <vector>

#include "agent.h"

using namespace std;

class Node : public Agent {
  // vector<Node> near_storage_nodes;
  // vector<Node> near_sensor_nodes;
 protected:
  int node_id_;
  double y_coord_, x_coord_;
 public:
  vector<Node*> near_storage_nodes;
  vector<Node*> near_sensor_nodes;

  Node();
  Node(int /*node_id*/);
  Node(int /*node_id*/, double /*y_coord*/, double /*x_coord*/);
    
  void set_position(double /*y_coord*/, double /*x_coord*/);
  int get_node_id() {return node_id_;}
  double get_y_coord() {return y_coord_;}
  double get_x_coord() {return x_coord_;}
  void add_near_storage_node(Node*);
  void add_near_sensor_node(Node*);
};

#endif
