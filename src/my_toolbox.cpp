#include <math.h>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <random>   // genertion of random variables -> require -std=c++11
#include "my_toolbox.h"
#include "node.h"
#include "storage_node.h"
#include "user.h"
using namespace std;


MyToolbox::MyTime MyToolbox::current_time_ = 0;
int MyToolbox::n_ = 0;
int MyToolbox::k_ = 0;
int MyToolbox::C1_ = 0;
int MyToolbox::max_msg_hops_ = 0;
int MyToolbox::bits_for_measure_ = 0;
int MyToolbox::bits_for_id_ = 0;
int MyToolbox::bits_for_phy_mac_overhead_ = 0;
double MyToolbox::channel_bit_rate_ = 0;
map<int, MyToolbox::MyTime> MyToolbox::timetable_;
int MyToolbox::ping_frequency_ = 0;
int MyToolbox::check_sensors_frequency_=0;
double MyToolbox::user_velocity_=0;
long MyToolbox::user_update_time_=0;
int MyToolbox::tx_range_=0;
int MyToolbox::space_precision_=0;
int MyToolbox::square_size_=0;

vector<SensorNode*> MyToolbox::sensor_nodes_;
vector<StorageNode*> MyToolbox::storage_nodes_;
vector<User*> MyToolbox::users_;



void MyToolbox::set_current_time(MyToolbox::MyTime current_time) {
  current_time_ = current_time;
}

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

void MyToolbox::set_channel_bit_rate(double channel_bit_rate) {
  channel_bit_rate_ = channel_bit_rate;
}

void MyToolbox::set_timetable(map<int, MyTime> timetable) {
  timetable_ = timetable;
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

void MyToolbox::set_user_update_time() {
    user_update_time_ = (tx_range_/user_velocity_)*pow(10,9);    // time for do tx_range
}

void MyToolbox::set_tx_range(int range) {
    tx_range_=range;
}

void MyToolbox::set_space_precision(int space_precision){
    space_precision_=space_precision;
}

void MyToolbox::set_square_size(int square_size){
    square_size_=square_size;
}

void MyToolbox::set_sensor_nodes(vector<SensorNode*> sensor){
    sensor_nodes_=sensor;
}

void MyToolbox::set_storage_nodes(vector<StorageNode*> storage){
    storage_nodes_=storage;
}

void MyToolbox::set_users(vector<User*> user){
    users_=user;
}

void MyToolbox::set_near_storage_node(Node* node){
    for(int i = 0; i< node->near_storage_nodes.size(); i++)
          MyToolbox::remove_near_storage_node(node, (StorageNode*)node->near_storage_nodes.at(i));
      
    for(int i = 0; i< storage_nodes_.size(); i++){
          if (Node::are_nodes_near(storage_nodes_.at(i),node) == true)
              node->add_near_sensor_node(storage_nodes_.at(i));
      }
}

void MyToolbox::set_near_user(Node* node){
    for(int i = 0; i< node->near_users.size(); i++)
          MyToolbox::remove_near_user(node, (User*)node->near_users.at(i));
      
    for(int i = 0; i< users_.size(); i++){
          if (Node::are_nodes_near(users_.at(i),node) == true)
              node->add_near_user(users_.at(i));
      }  
}

void MyToolbox::remove_near_storage_node(Node* node, StorageNode *storage_node) {
  node->near_storage_nodes.erase(find(node->near_storage_nodes.begin(), node->near_storage_nodes.end(), storage_node));
}

void MyToolbox::remove_near_user(Node* node, User *user) {
  node->near_users.erase(find(node->near_users.begin(), node->near_users.end(), user));
}

void MyToolbox::new_user(User *user){
    users_.push_back (user);
}
int MyToolbox::get_ideal_soliton_distribution_degree() {

  /*
    What is the smallest interval of probability [1/(i^2 - i) - 1/(i^2 + i)] ?
    I must have a granularity smallest than this interval, therwise I would be
    sure I won't ever have some values!

    The smallest interval is the last one, always.

    Devo dividere l'intervallo in modo che l'unità (1/M) sia piu' corta dell'intervallo
    piu' corto.

    Divido l'intervallo [0, 1] in M intervalli.
    La lunghezza di un intervallo (1/M) deve essere piu' grande dell'intervallo di prob minore,
    che e' 1/(k^2 - k).

    1/M < int_min = 1 / (k^2 - k) => M > k(k - 1)

    Per andare sul sicuro: M = 10 * k(k - 1)

    Ho M intervalli. Genero un numero random tra 0 e M - 1 con "rand() % M" e 
    poi divido ancora per (M - 1) per normalizzare tra 0 e 1.

    The intervl bounds are so divided:
    [       ]      ]    ]  ] ]
  */

  int M = k_ * (k_ - 1);
  double prob = (rand() % M) / (double)(M - 1);

  // cout << "Prob: " << prob << endl;

  double up_bound = 1. / k_;
  double interval_length = up_bound;
  if (prob <= up_bound) { // between 0 and 1/k -> degree = 1;
    return 1;
  }
  for (int i = 2; i <= k_; ++i) {
    interval_length = 1. / (i * i - i);
    up_bound += interval_length;
    // cout << "Step: " << i << ". Interval length: " << interval_length << ", up_bound: " << up_bound << endl;
    if (prob <= up_bound) {
      return i;
    }
  }

  return k_;
}

int MyToolbox::get_robust_soliton_distribution_degree() {
  return -1;
}

MyToolbox::MyTime MyToolbox::get_random_processing_time() {
  // default_random_engine generator;
  // normal_distribution<double> distribution(MEAN_PROCESSING_TIME, STD_DEV_PROCESSING_TIME);
  // int rnd_proc_time = (int)(distribution(generator));
  // return rnd_proc_time;
  return MEAN_PROCESSING_TIME;
}

MyToolbox::MyTime MyToolbox::get_retransmission_offset() {
  int rand1 = rand();
  int rand2 = rand();
  unsigned long long_rand = rand1 * rand2;
  MyTime offset = long_rand % (MAX_OFFSET - 1) + 1;
  cout << "offset: " << offset << endl;
  return offset;
}
