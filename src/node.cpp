#include <math.h>

#include "node.h"
#include "my_toolbox.h"
#include "user.h"
#include "sensor_node.h"
#include "storage_node.h"

using namespace std;

/**************************************
    Constructors
**************************************/
Node::Node() {
  node_id_ = 0;
  data_collector = nullptr;
  y_coord_ = 0;
  x_coord_ = 0;
}

Node::Node(unsigned int node_id) {
  node_id_ = node_id;
  data_collector = nullptr;
  y_coord_ = 0;
  x_coord_ = 0;
}

Node::Node(unsigned int node_id, double y_coord, double x_coord) {
  node_id_ = node_id;
  y_coord_ = y_coord;
  x_coord_ = x_coord;
  data_collector = nullptr;
  near_sensors_ = map<unsigned int, SensorNode*>();
  near_storage_nodes_ = map<unsigned int, StorageNode*>();
  near_users_ = map<unsigned int, User*>();
}

/**************************************
    Setters
**************************************/
void Node::set_position(double y_coord, double x_coord) {
  y_coord_ = y_coord;
  x_coord_ = x_coord;
}

/**************************************
    Functions
**************************************/
//void Node::add_near_storage_node(Node *node) {
//  near_storage_nodes.push_back(node);
//}
//
//void Node::add_near_sensor_node(Node *node) {
//  near_sensor_nodes.push_back(node);
//}
//
//void Node::add_near_user(Node *user) {
//  near_users.push_back(user);
//}

//bool Node::are_nodes_near(Node* n1, Node* n2) {
//    if (sqrt( pow((n1->x_coord_-n2->x_coord_),2)+ pow((n1->y_coord_-n2->y_coord_),2)) < MyToolbox::tx_range_ * MyToolbox::space_precision_){
//        return true;
//    }
//    else
//        return false;
//}

unsigned int Node::get_random_neighbor() {
	if (near_storage_nodes_.size() == 0) {	// I don't have neighbors
		return 0;
	} else {
		int next_node_index = rand() % near_storage_nodes_.size();
		map<unsigned int, StorageNode*>::iterator node_iter = near_storage_nodes_.begin();
		for (int i = 0; i < next_node_index; i++) {
			node_iter++;
		}
		return node_iter->first;
	}
}
