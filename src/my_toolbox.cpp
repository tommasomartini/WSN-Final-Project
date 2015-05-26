#include <math.h>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <algorithm>
// #include <random>   // generation of random variables -> require -std=c++11

#include "my_toolbox.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"

using namespace std;

MyToolbox::MyTime MyToolbox::current_time_ = 0;
map<unsigned int, MyToolbox::MyTime> MyToolbox::timetable_;
unsigned int MyToolbox::node_id_ = 10;	// the first 10 ids are reserved

//  Global values
int MyToolbox::num_storage_nodes_ = 0;
int MyToolbox::num_sensors_ = 0;
int MyToolbox::num_users_ = 0;

int MyToolbox::num_bits_for_measure_ = 0;

double MyToolbox::bitrate_ = 0;
double MyToolbox::bit_error_prob_ = 0;
double MyToolbox::tx_range_ = 0;

MyToolbox::MyTime MyToolbox::ping_frequency_ = 0;
MyToolbox::MyTime MyToolbox::check_sensors_frequency_ = 0;

double MyToolbox::C1_ = 0;
int MyToolbox::max_num_hops_ = 0;

int MyToolbox::square_size_ = 0;

double MyToolbox::user_velocity_ = 0;
MyToolbox::MyTime MyToolbox::user_update_time_ = 0;

MyToolbox::MyTime MyToolbox::processing_time_ = 0;

MyToolbox::MyTime MyToolbox::max_tx_offset_ = 0;

MyToolbox::MyTime MyToolbox::user_observation_time_ = 0;

MyToolbox::MyTime MyToolbox::max_measure_generation_delay_ = 0;
double MyToolbox::sensor_failure_prob_ = 0;
int MyToolbox::num_measures_for_sensor_ = 0;

map<unsigned int, SensorNode> MyToolbox::sensors_map_;
map<unsigned int, StorageNode> MyToolbox::storage_nodes_map_;
map<unsigned int, User> MyToolbox::users_map_;

default_random_engine MyToolbox::generator_;

void MyToolbox::set_close_nodes(User* user) {
  user->near_storage_nodes_.clear();
  user->near_users_.clear();

  for (auto& st_node_elem : storage_nodes_map_) {
    StorageNode* st_node = &(st_node_elem.second);
    double his_x = st_node->get_x_coord();
    double his_y = st_node->get_y_coord();
    double my_x = user->get_x_coord();
    double my_y = user->get_y_coord();
    double dist = sqrt(pow(my_x - his_x, 2) + pow(my_y - his_y, 2));  // compute the distance between the two nodes
    if (dist <= MyToolbox::tx_range_) { // the users are able to communicate
      pair<unsigned int, StorageNode*> pp(st_node->get_node_id(), st_node);
      user->near_storage_nodes_.insert(pp);
    }
  }

  for (auto& us_node_elem : users_map_) {
	  User* us_node = &(us_node_elem.second);
	  if (us_node != user) {  // does not make sense to include myself among my neighbours
		  double his_x = us_node->get_x_coord();
		  double his_y = us_node->get_y_coord();
		  double my_x = user->get_x_coord();
		  double my_y = user->get_y_coord();
		  double dist = sqrt(pow(my_x - his_x, 2) + pow(my_y - his_y, 2));  // compute the distance between the two nodes
		  if (dist <= MyToolbox::tx_range_) { // the users are able to communicate
			  pair<unsigned int, User*> pp(us_node->get_node_id(), us_node);
			  user->near_users_.insert(pp);
		  }
	  }
  }
}

/**************************************
    Functions
**************************************/
void MyToolbox::initialize_toolbox() {
  cout << "Toolbox initialization..." << endl;
  cout << "Setting maximum number of hops: ";
  max_num_hops_ = ceil(C1_ * num_storage_nodes_ * log(num_storage_nodes_));
  cout << max_num_hops_ << endl;
  cout << "Setting default random engine...";
  generator_ = default_random_engine(time(NULL));
  cout << " done!" << endl;
}

bool MyToolbox::is_node_active(unsigned int node_id) {

  if (sensors_map_.find(node_id) != sensors_map_.end()) {
    return true;
  }

  if (storage_nodes_map_.find(node_id) != storage_nodes_map_.end()) {
    return true;
  }

  if (users_map_.find(node_id) != users_map_.end()) {
    return true;
  }

  return false;
}

// Made by Tom
void MyToolbox::remove_sensor(unsigned int sensor_id) {
  timetable_.erase(sensor_id);
  sensors_map_.erase(sensor_id);
  cout << "TB: eliminato sensore " << sensor_id << endl;
}

unsigned int MyToolbox::get_node_id() {
  return node_id_++;
}

// TODO define this
//User* MyToolbox::new_user(){
//    double y_coord = rand() % (MyToolbox::get_space_precision()* MyToolbox::get_space_precision());
//    double x_coord = rand() % (MyToolbox::get_space_precision()* MyToolbox::get_space_precision());
////    User* new_user = new User(users_.size(),y_coord,x_coord);
////    users_.push_back (new_user);
//    return new User();
//}

int MyToolbox::get_ideal_soliton_distribution_degree() {
  uniform_real_distribution<double> distribution(0.0, 1.0);
  double rnd_point = distribution(generator_);

  double up_bound = 1. / num_sensors_;
  double interval_length = up_bound;
  if (rnd_point <= up_bound) { // between 0 and 1/k -> degree = 1;
	  return 1;
  }
  for (int i = 2; i <= num_sensors_; i++) {
	  interval_length = 1. / (i * (i - 1));
	  up_bound += interval_length;
	  if (rnd_point <= up_bound) {
		  return i;
	  }
  }
  return num_sensors_;
}

int MyToolbox::get_robust_soliton_distribution_degree() {
//
//    int K = num_storage_nodes_;
//    double c = 0.2;
//    double delta = 0.05;
//    double S = c * log(K / delta) * sqrt( K );
//    int tau_bound = ((int)((K / S) + .5)) - 1;
//    double den = 0;
//    for (int i = 1; i <= K; i++) {
//        if (i = 1) {
//            den += 1 / K + S / K;
//        }
//        if ((i >= 2) & (i <= tau_bound)) {
//            den += 1 / (i * (i - 1)) + S / (i * K);
//        }
//        if (i = tau_bound + 1) {
//            den += 1 / (i * (i - 1)) + S / K * log(S / delta);
//        }
//        if (i > tau_bound + 1) {
//            den += 1 / (i * (i - 1));
//        }
//    }
//
//    int M = 10 * num_storage_nodes_ * (num_storage_nodes_ - 1); // as in ideal soliton over, the smallest interval is the last
//    double prob = (rand() % M) / (double)(M - 1);
//
//    double up_bound = ((1. / K) + (S / K)) / den;
//    double interval_length = up_bound;
//    if (prob <= up_bound) { // between 0 and (1/k+s/k)/den -> degree = 1;
//        return 1;
//    }
//    int d = 2;
//    for (int i = d; i <= tau_bound; i++) { // case 2 <= d < K/S
//        interval_length = ((1. / (i * i - i)) + (S / (K * i))) / den;
//        up_bound += interval_length;
//        if (prob <= up_bound) {
//            return i;
//        }
//        d = i;
//    }
//    interval_length = ((1. / (d * d - d)) + (S / K * log(S / delta))) / den; // case d = K/S
//    up_bound += interval_length;
//    if (prob <= up_bound) {
//        return d;
//    } else {
//        d++;
//    }
//    for (int i = d; i <= K; i++) { // case K/S < d <= K
//       interval_length = (1. / (i * i - i)) / den;
//        up_bound += interval_length;
//        if (prob <= up_bound) {
//            return i;
//        }
//    }

    return -1;
}

MyToolbox::MyTime MyToolbox::get_random_processing_time() {
  return processing_time_;
}

MyToolbox::MyTime MyToolbox::get_tx_offset() {
  uniform_int_distribution<int> distribution(max_tx_offset_ / 2000, max_tx_offset_ / 1000);	// between 5ms and 10ms
  int offset = distribution(generator_);
  return (MyTime)(offset * 1000);
}

string MyToolbox::int2nodetype(unsigned int num) {
	string res = "unknown";
	if (sensors_map_.find(num) != sensors_map_.end()) {
		res = "sensor";
	} else if (storage_nodes_map_.find(num) != storage_nodes_map_.end()) {
		res = "cache";
	} else if (users_map_.find(num) != users_map_.end()) {
		res = "user";
	} else {
		res = "unknown";
	}
	return res;
}

void show_c(vector<pair<int, Node*>>* clouds_, int num_clouds) {
  for (vector<pair<int, Node*>>::iterator it1 = clouds_->begin(); it1 != clouds_->end(); it1++) {
	cout << "|" << it1->first << ", " << it1->second->get_node_id();
  }
  cout << "|" << endl;
  cout << num_clouds << " clouds:";
  int curr_color = -1;
  for (vector<pair<int, Node*>>::iterator iterator = clouds_->begin(); iterator != clouds_->end(); iterator++) {
	if (curr_color != iterator->first) {
      cout << "\n" << iterator->first << " | ";
	}
	curr_color = iterator->first;
	cout << iterator->second->get_node_id() << " ";
  }
  cout << endl;
}

int MyToolbox::check_clouds2() {
  int color = 0;
  vector<pair<int, StorageNode*>> clouds;	// cloud_id - node
  map<unsigned int, StorageNode*> allnodes;
  // copy all the nodes into the allnodes map
  for (map<unsigned int, StorageNode>::iterator it_cache = storage_nodes_map_.begin(); it_cache != storage_nodes_map_.end(); it_cache++) {
    allnodes.insert(pair<unsigned int, StorageNode*>(it_cache->first, &(it_cache->second)));
  }

//  cout << "Allnodes contiene " << allnodes.size() << " elementi" << endl;
//  cout << "#ALLNODES" << endl;
//  for (map<unsigned int, StorageNode*>::iterator it = allnodes.begin(); it != allnodes.end(); it++) {
//	  cout << "Node " << it->second->get_node_id() << endl;
//  }

  clouds.push_back(pair<int, StorageNode*>(color, allnodes.begin()->second));	// insert in the cloud the seed, the first node
  allnodes.erase(allnodes.begin());	// erase the element
//  cout << "Allnodes contiene " << allnodes.size() << " elementi" << endl;
//  cout << "#ALLNODES" << endl;
//    for (map<unsigned int, StorageNode*>::iterator it = allnodes.begin(); it != allnodes.end(); it++) {
//  	  cout << "-Node " << it->second->get_node_id() << endl;
//    }
  int cloud_index = 0;	// pointing to the first (and only!) element in the cloud vector
//  cout << "Primo nodo di cloud " << clouds.begin()->second->get_node_id() << ". Ora allnodes ha " << allnodes.size() << " elementi" << endl;
  while (!allnodes.empty()) {	// in allnodes there are the not-yet-classified nodes
//	  cout << "Cloud index: " << cloud_index << endl;
	  StorageNode* curr_node = clouds.at(cloud_index).second;	// current node, at position pointed by cloud_index
//	  cout << " curr node " << curr_node->get_node_id() << endl;
//	  bool neighbours_in_allnodes = false;	// are there any of my neighbours in allnodes?
	  for (map<unsigned int, StorageNode*>::iterator neigh_it = curr_node->near_storage_nodes_.begin(); neigh_it != curr_node->near_storage_nodes_.end(); neigh_it++) {	// for each neighbour of the current node
		  unsigned int curr_neigh_id = neigh_it->first;	// id of this neighbour
//		  cout << " Vicino " << curr_neigh_id;
		  if (allnodes.find(curr_neigh_id) != allnodes.end()) {	// if in allnodes there is this neighbour
			  clouds.push_back(pair<int, StorageNode*>(color, neigh_it->second));	// put this neighbour into the cloud
			  allnodes.erase(neigh_it->first);	// erase the neighbour from the map of non-classified neighbours
//			  cout << " presente! Messo in clouds (" << clouds.size() << ") con colore " << color << ". Allnodes ha " << allnodes.size() << " elems" << endl;
//			  cout << " Allnodes contiene " << allnodes.size() << " elementi" << endl;
//			  cout << " #ALLNODES" << endl;
//			  for (map<unsigned int, StorageNode*>::iterator it = allnodes.begin(); it != allnodes.end(); it++) {
//			  	  cout << " -Node " << it->second->get_node_id() << endl;
//			    }
			  //			  neighbours_in_allnodes = true;
		  } else {
//			  cout << " non presente" << endl;
		  }
	  }
	  cloud_index++;
	  if (cloud_index == int(clouds.size())) {	// new cloud
		  color++;
		  clouds.push_back(pair<int, StorageNode*>(color, allnodes.begin()->second));
		  allnodes.erase(allnodes.begin());

//		  			  cout << " Allnodes contiene " << allnodes.size() << " elementi" << endl;
//		  			cout << " #ALLNODES" << endl;
//		  			  for (map<unsigned int, StorageNode*>::iterator it = allnodes.begin(); it != allnodes.end(); it++) {
//		  			  	  cout << " -Node " << it->second->get_node_id() << endl;
//		  			    }
		  cloud_index = clouds.size() - 1;
//		  cout << " No piu' nodi in cloud. Nuovo colore " << color << endl;
	  }
  }

  //  show_c(&clouds, color + 1);
  return color + 1;
}

int MyToolbox::check_clouds() {
  int color = 0;
  vector<pair<int, Node*>> clouds;	// cloud_id - node
  map<unsigned int, Node*> allnodes;
  // copy all the nodes into the allnodes map
  for (map<unsigned int, StorageNode>::iterator it_cache = storage_nodes_map_.begin(); it_cache != storage_nodes_map_.end(); it_cache++) {
    allnodes.insert(pair<unsigned int, Node*>(it_cache->first, &(it_cache->second)));
  }

  clouds.push_back(pair<int, Node*>(color, allnodes.begin()->second));	// insert in the cloud the seed, the first node
  allnodes.erase(allnodes.begin());	// erase the element
  int cloud_index = 0;	// pointing to the first (and only!) element in the cloud vector
  while (!allnodes.empty()) {	// in allnodes there are the not-yet-classified nodes
	map<unsigned int, Node*>::iterator allnodes_iter = allnodes.begin();	// first element of allnodes
	while (allnodes_iter != allnodes.end()) {	// while I don't finish the nodes in the list...
	  double x1 = clouds.at(cloud_index).second->get_x_coord();	// get the coordinates of the node in the cloud
	  double y1 = clouds.at(cloud_index).second->get_y_coord();
	  double x2 = allnodes_iter->second->get_x_coord();	// get the coordinates of the node in allnode
	  double y2 = allnodes_iter->second->get_y_coord();
	  bool near = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)) <= tx_range_;	// compute the distance
	  if (near) {	// if they are near
		clouds.push_back(pair<int, Node*>(color, allnodes_iter->second));	// put this node into the cloud
		allnodes_iter = allnodes.erase(allnodes_iter);	// erase it and return a pointer to the following element (C++11)
	  } else {
		allnodes_iter++;
	  }
	}
	cloud_index++;
	if (cloud_index == int(clouds.size())) {	// new cloud
	  color++;
	  clouds.push_back(pair<int, Node*>(color, allnodes.begin()->second));
	  allnodes.erase(allnodes.begin());
	  cloud_index = clouds.size() - 1;
	}
  }

//  show_c(&clouds, color + 1);
  return color + 1;
}

bool MyToolbox::sensor_connected() {
	for (map<unsigned int, SensorNode>::iterator sns_it = sensors_map_.begin(); sns_it != sensors_map_.end(); sns_it++) {	// for each sensor...
		if (sns_it->second.near_storage_nodes_.empty()) {	// ...check if it has at least one neighbour. If it has not...
			return false;	// ...break and return false.
		}
	}
	return true;
}

