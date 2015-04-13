#include <math.h>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <algorithm>
// #include <random>   // generation of random variables -> require -std=c++11

#include "my_toolbox.h"
#include "node.h"
#include "storage_node.h"
#include "user.h"

using namespace std;

MyToolbox::MyTime MyToolbox::current_time_ = 0;
map<unsigned int, MyToolbox::MyTime> MyToolbox::timetable_;
unsigned int MyToolbox::node_id_ = 0;
//DataCollector* MyToolbox::dc;

//  Global values
int MyToolbox::num_storage_nodes = 0;
int MyToolbox::num_sensors = 0;
int MyToolbox::max_num_users = 0;
int MyToolbox::num_users = 0;

int MyToolbox::num_bits_for_id = 0;
int MyToolbox::num_bits_for_measure = 0;
int MyToolbox::num_bits_phy_mac_overhead = 0;
int MyToolbox::num_bits_for_measure_id = 0;

double MyToolbox::bitrate = 0;
double MyToolbox::bit_error_prob = 0;
double MyToolbox::tx_range = 0;

MyToolbox::MyTime MyToolbox::ping_frequency = 0;
MyToolbox::MyTime MyToolbox::check_sensors_frequency = 0;

double MyToolbox::C1 = 0;
int MyToolbox::max_num_hops = 0;

int MyToolbox::square_size = 0;
int MyToolbox::space_precision = 0;

double MyToolbox::user_velocity = 0;
MyToolbox::MyTime MyToolbox::user_update_time = 0;

MyToolbox::MyTime MyToolbox::mean_processing_time = 0;
MyToolbox::MyTime MyToolbox::std_dev_processing_time = 0;

MyToolbox::MyTime MyToolbox::max_tx_offset = 0;
MyToolbox::MyTime MyToolbox::max_tx_offset_ping = 0;

MyToolbox::MyTime MyToolbox::user_observation_time = 0;

MyToolbox::MyTime MyToolbox::max_measure_generation_delay = 0;
double MyToolbox::sensor_failure_prob = 0;

map<unsigned int, Node*>* MyToolbox::sensors_map_ptr;
map<unsigned int, Node*>* MyToolbox::storage_nodes_map_ptr; 
map<unsigned int, Node*>* MyToolbox::users_map_ptr; 

default_random_engine MyToolbox::generator;


// TODO remove everything hereafter
vector<SensorNode*> MyToolbox::sensor_nodes_;
vector<StorageNode*> MyToolbox::storage_nodes_;
vector<User*> MyToolbox::users_;
  
/*
int MyToolbox::n_ = 0;
int MyToolbox::k_ = 0;
int MyToolbox::C1_ = 0;
int MyToolbox::max_msg_hops_ = 0;
int MyToolbox::bits_for_measure_ = 0;
int MyToolbox::bits_for_id_ = 0;
int MyToolbox::bits_for_phy_mac_overhead_ = 0;
int MyToolbox::bits_for_measure_id_ = 0;
int MyToolbox::bits_for_hop_counter_ = 0;
double MyToolbox::channel_bit_rate_ = 0;
int MyToolbox::ping_frequency_ = 0;
int MyToolbox::check_sensors_frequency_=0;
double MyToolbox::user_velocity_=0;
long MyToolbox::user_update_time_=0;
int MyToolbox::space_precision_=0;
int MyToolbox::square_size_=0;
/**/


/**************************************
    Setters
**************************************/
void MyToolbox::set_current_time(MyToolbox::MyTime current_time) {
  current_time_ = current_time;
}

void MyToolbox::set_timetable(map<unsigned int, MyTime> timetable) {
  timetable_ = timetable;
}

//void MyToolbox::set_user_update_time() {
//    user_update_time = (tx_range / user_velocity)*pow(10,9);
//}

///////////////////////////////////////////////////// TODO remove
/*
void MyToolbox::set_n(int n) {
  n_ = n;
}

void MyToolbox::set_k(int k) {
  k_ = k;
}

void MyToolbox::set_C1(int C1) {
  C1_ = C1;
  max_msg_hops_ = ceil(C1 * n_ * log(n_));
}

void MyToolbox::set_bits_for_measure(int bits_for_measure) {
  bits_for_measure_ = bits_for_measure;
}

void MyToolbox::set_bits_for_id(int bits_for_id) {
  bits_for_id_ = bits_for_id;
}

void MyToolbox::set_bits_for_phy_mac_overhead(int bits_for_phy_mac_overhead) {
  bits_for_phy_mac_overhead_ = bits_for_phy_mac_overhead;
}

void MyToolbox::set_bits_for_measure_id(int bits_for_measure_id) {
  bits_for_measure_id_ = bits_for_measure_id;
}

void MyToolbox::set_bits_for_hop_counter(int bits_for_hop_counter) {
  bits_for_hop_counter_ = bits_for_hop_counter;
}

void MyToolbox::set_channel_bit_rate(double channel_bit_rate) {
  channel_bit_rate_ = channel_bit_rate;
}

void MyToolbox::set_ping_frequency(int ping_frequency) {
  ping_frequency_ = ping_frequency;
}

void MyToolbox::set_check_sensors_frequency(int check_sensors_frequency) {
  check_sensors_frequency_ =check_sensors_frequency;
}
void MyToolbox::set_user_velocity(double velocity){
    user_velocity_ = velocity;
}

void MyToolbox::set_space_precision(int space_precision){
    space_precision_=space_precision;
}

void MyToolbox::set_square_size(int square_size){
    square_size_=square_size;
}
/**/

void MyToolbox::set_sensor_nodes(vector<SensorNode*> sensor) {
  sensor_nodes_ = sensor;
}

void MyToolbox::set_storage_nodes(vector<StorageNode*> storage) {
  storage_nodes_ = storage;
}

void MyToolbox::set_users(vector<User*> user) {
  users_ = user;
}

void MyToolbox::set_close_nodes(User* user) {
  user->near_storage_nodes_->clear();
  user->near_users_->clear();

  for (auto& st_node_elem : *storage_nodes_map_ptr) {
    StorageNode* st_node = (StorageNode*)st_node_elem.second;
    double his_x = st_node->get_x_coord();
    double his_y = st_node->get_y_coord();
    double my_x = user->get_x_coord();
    double my_y = user->get_y_coord();
    double dist = sqrt(pow(my_x - his_x, 2) + pow(my_y - his_y, 2));  // compute the distance between the two nodes
    if (dist < MyToolbox::tx_range) { // the users are able to communicate
      pair<unsigned int, Node*> pp(st_node->get_node_id(), st_node);
      user->near_storage_nodes_->insert(pp);
    } 
  }

  for (auto& us_node_elem : *users_map_ptr) {
  User* us_node = (User*)us_node_elem.second;
    if (us_node != user) {  // does not make sense to include myself among my neighbours
      double his_x = us_node->get_x_coord();
      double his_y = us_node->get_y_coord();
      double my_x = user->get_x_coord();
      double my_y = user->get_y_coord();
      double dist = sqrt(pow(my_x - his_x, 2) + pow(my_y - his_y, 2));  // compute the distance between the two nodes
      if (dist < MyToolbox::tx_range) { // the users are able to communicate
        pair<unsigned int, Node*> pp(us_node->get_node_id(), us_node);
        user->near_users_->insert(pp);
      } 
    }
  }
}

void MyToolbox::set_near_storage_node(Node* node) {
  // while(node->near_storage_nodes.size()>0)
  //   MyToolbox::remove_near_storage_node(node, (StorageNode*)node->near_storage_nodes.at(0));
  for(int i = 0; i< storage_nodes_.size(); i++){
    if (node!= storage_nodes_.at(i) && Node::are_nodes_near(storage_nodes_.at(i),node) == true)
      node->add_near_sensor_node(storage_nodes_.at(i));
  }
}

void MyToolbox::set_near_user(Node* node){
     while(node->near_users.size()>0)
          MyToolbox::remove_near_user(node, (User*)node->near_users.at(0));
     for(int i = 0; i< users_.size(); i++){
          if (node!=users_.at(i) && Node::are_nodes_near(users_.at(i),node) == true)
            node->add_near_user(users_.at(i));
      }   
}

void MyToolbox::remove_near_storage_node(Node* node, StorageNode* storage_node) {
   node->near_storage_nodes.erase(find(node->near_storage_nodes.begin(), node->near_storage_nodes.end(), storage_node));
}

void MyToolbox::remove_near_user(Node* node, User* user) {
   // node->near_users.erase(find(node->near_users.begin(), node->near_users.end(), user));
}

////////////////////////////////////////////////////////////////////////////////////////////

/**************************************
    Functions
**************************************/
void MyToolbox::initialize_toolbox() {
  cout << "Devo ancora fare l'inizializzazione!!" << endl;
  max_num_hops = ceil(C1 * num_storage_nodes * log(num_storage_nodes));
  generator = default_random_engine(time(NULL));
}

bool MyToolbox::is_node_active(unsigned int node_id) {

  if (sensors_map_ptr->find(node_id) != sensors_map_ptr->end()) {
    return true;
  }

  if (storage_nodes_map_ptr->find(node_id) != storage_nodes_map_ptr->end()) {
    return true;
  }

  if (users_map_ptr->find(node_id) != users_map_ptr->end()) {
    return true;
  }

  return false;
}

void MyToolbox::remove_sensor(unsigned int sensor_id) {
  timetable_.erase(sensor_id);
  sensors_map_ptr->erase(sensor_id);
  cout << "TB: eliminato sensore " << sensor_id << endl;
}

unsigned int MyToolbox::get_node_id() {
  return node_id_++;
}

User* MyToolbox::new_user(){
    double y_coord = rand() % (MyToolbox::get_space_precision()* MyToolbox::get_space_precision());
    double x_coord = rand() % (MyToolbox::get_space_precision()* MyToolbox::get_space_precision());
    User* new_user = new User(users_.size(),y_coord,x_coord);
    users_.push_back (new_user);
    return new_user;
}

int MyToolbox::get_ideal_soliton_distribution_degree() {
  uniform_real_distribution<double> distribution(0.0, 1.0);
  double rnd_point = distribution(generator);

  double up_bound = 1. / num_storage_nodes;
  double interval_length = up_bound;
  if (rnd_point <= up_bound) { // between 0 and 1/k -> degree = 1;
	  return 1;
  }
  for (int i = 2; i <= num_storage_nodes; i++) {
	  interval_length = 1. / (i * (i - 1));
	  up_bound += interval_length;
	  if (rnd_point <= up_bound) {
		  return i;
	  }
  }
  return num_storage_nodes;
}

int MyToolbox::get_robust_soliton_distribution_degree() {
    
    int K = num_storage_nodes;
    double c = 0.2;
    double delta = 0.05;
    double S = c * log(K / delta) * sqrt( K );
    int tau_bound = ((int)((K / S) + .5)) - 1;
    double den = 0;
    for (int i = 1; i <= K; i++) {
        if (i = 1) {
            den += 1 / K + S / K;
        }
        if ((i >= 2) & (i <= tau_bound)) {
            den += 1 / (i * (i - 1)) + S / (i * K);
        }
        if (i = tau_bound + 1) {
            den += 1 / (i * (i - 1)) + S / K * log(S / delta);
        }
        if (i > tau_bound + 1) {
            den += 1 / (i * (i - 1));
        }
    }
    
    int M = 10 * num_storage_nodes * (num_storage_nodes - 1); // as in ideal soliton over, the smallest interval is the last
    double prob = (rand() % M) / (double)(M - 1);

    double up_bound = ((1. / K) + (S / K)) / den;
    double interval_length = up_bound;
    if (prob <= up_bound) { // between 0 and (1/k+s/k)/den -> degree = 1;
        return 1;
    }
    int d = 2;
    for (int i = d; i <= tau_bound; i++) { // case 2 <= d < K/S
        interval_length = ((1. / (i * i - i)) + (S / (K * i))) / den;
        up_bound += interval_length;
        if (prob <= up_bound) {
            return i;
        }
        d = i;
    }
    interval_length = ((1. / (d * d - d)) + (S / K * log(S / delta))) / den; // case d = K/S
    up_bound += interval_length;
    if (prob <= up_bound) {
        return d;    
    } else {
        d++;
    }
    for (int i = d; i <= K; i++) { // case K/S < d <= K
       interval_length = (1. / (i * i - i)) / den;
        up_bound += interval_length;
        if (prob <= up_bound) {
            return i;
        } 
    }

    return -1;
}

MyToolbox::MyTime MyToolbox::get_random_processing_time() {
  // normal_distribution<double> distribution(mean_processing_time, std_dev_processing_time);
  normal_distribution<long double> distribution(mean_processing_time, std_dev_processing_time);
  MyTime rnd_proc_time = (MyTime)(distribution(generator));
  //return rnd_proc_time;

  return mean_processing_time;
}

// MyToolbox::MyTime MyToolbox::get_retransmission_offset() {
//   int rand1 = rand();
//   int rand2 = rand();
//   unsigned long long_rand = rand1 * rand2;
//   MyTime offset = long_rand % (max_tx_offset - 1) + 1;
//   // cout << "offset: " << offset << endl;
//   return offset;
// }

MyToolbox::MyTime MyToolbox::get_tx_offset() {
  uniform_int_distribution<int> distribution(max_tx_offset_ping / 1000, max_tx_offset / 1000);	// between 5ms and 10ms
  int offset = distribution(generator);
  return (MyTime)(offset * 1000);
}

MyToolbox::MyTime MyToolbox::get_tx_offset_ping() {
  uniform_int_distribution<int> distribution(1, (max_tx_offset_ping / 1000) - 1);	// between 1 and 4999us
  int offset = distribution(generator);
  return (MyTime)(offset * 1000);
}

string MyToolbox::int2nodetype(unsigned int num) {
	string res = "unknown";
	if (sensors_map_ptr->find(num) != sensors_map_ptr->end()) {
		res = "sensor";
	} else if (storage_nodes_map_ptr->find(num) != storage_nodes_map_ptr->end()) {
		res = "cache";
	} else if (users_map_ptr->find(num) != users_map_ptr->end()) {
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

void MyToolbox::show_clouds() {
  int color = 0;
  vector<pair<int, Node*>> clouds;	// cloud_id - node
  map<unsigned int, Node*> allnodes;
  // copy all the nodes into the allnodes map
  for (map<unsigned int, Node*>::iterator it_cache = storage_nodes_map_ptr->begin(); it_cache != storage_nodes_map_ptr->end(); it_cache++) {
    allnodes.insert(pair<unsigned int, Node*>(it_cache->first, it_cache->second));
  }
  for (map<unsigned int, Node*>::iterator it_sns = sensors_map_ptr->begin(); it_sns != sensors_map_ptr->end(); it_sns++) {
	allnodes.insert(pair<unsigned int, Node*>(it_sns->first, it_sns->second));
  }

//  cout << "allnodes: ";
//  for (map<unsigned int, Node*>::iterator it = allnodes.begin(); it != allnodes.end(); it++) {
//	cout << "|" << it->first << " (" << it->second->get_x_coord() << ", " << it->second->get_y_coord() << ")";
//  }
//  cout << "|" << endl;

  clouds.push_back(pair<int, Node*>(color, allnodes.begin()->second));	// insert in the cloud the seed, the first node
  allnodes.erase(allnodes.begin());	// erase the element
  int cloud_index = 0;	// pointing to the first (and only!) element in the cloud vector
  while (!allnodes.empty()) {	// in allnodes there are the not-yet-classified nodes
	map<unsigned int, Node*>::iterator allnodes_iter = allnodes.begin();
	while (allnodes_iter != allnodes.end()) {
	  double x1 = clouds.at(cloud_index).second->get_x_coord();
	  double y1 = clouds.at(cloud_index).second->get_y_coord();
	  double x2 = allnodes_iter->second->get_x_coord();
	  double y2 = allnodes_iter->second->get_y_coord();
	  bool near = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2)) <= tx_range;
	  if (near) {
		clouds.push_back(pair<int, Node*>(color, allnodes_iter->second));
		allnodes_iter = allnodes.erase(allnodes_iter);
	  } else {
		allnodes_iter++;
	  }
	}
	cloud_index++;
	if (cloud_index == clouds.size()) {	// new cloud
	  color++;
	  clouds.push_back(pair<int, Node*>(color, allnodes.begin()->second));
	  allnodes.erase(allnodes.begin());
	  cloud_index = clouds.size() - 1;
	}
  }

  show_c(&clouds, color + 1);
}

