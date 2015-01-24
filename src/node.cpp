#include "node.h"
#include "my_toolbox.h"
#include <math.h>

Node::Node() {
  node_id_ = -1;
}

Node::Node(int node_id) {
  node_id_ = node_id;
}

Node::Node(int node_id, double y_coord, double x_coord) {
  node_id_ = node_id;
  y_coord_ = y_coord;
  x_coord_ = x_coord;
}

void Node::set_position(double y_coord, double x_coord) {
  y_coord_ = y_coord;
  x_coord_ = x_coord;
}

void Node::add_near_storage_node(Node *node) {
  near_storage_nodes.push_back(node);
}

void Node::add_near_sensor_node(Node *node) {
  near_sensor_nodes.push_back(node);
}

void Node::add_near_user(Node *user) {
  near_users.push_back(user);
}

bool Node::are_nodes_near(Node* n1, Node* n2) {
    if (sqrt( pow((n1->x_coord_-n2->x_coord_),2)+ pow((n1->y_coord_-n2->y_coord_),2)) < MyToolbox::get_tx_range())
        return true;
    else
        return false;
}