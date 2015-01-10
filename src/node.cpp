#include "node.h"

Node::Node (int node_id) {
  node_id_ = node_id;
}

Node::Node (int node_id, double y_coord, double x_coord) {
  node_id_ = node_id;
  y_coord_ = y_coord;
  x_coord_ = x_coord;
}

void Node::set_position (double y_coord, double x_coord) {
  y_coord_ = y_coord;
  x_coord_ = x_coord;
}

void Node::add_near_storage_node(Node *node) {
  near_storage_nodes.push_back(node);
}

void Node::add_near_sensor_node(Node *node) {
  near_sensor_nodes.push_back(node);
}
