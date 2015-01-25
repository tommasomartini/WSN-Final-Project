/*  
  This class provides some tools statically accessible from every class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__  

#include <vector> 
#include <map> 

#include "agent.h"

class SensorNode;
class StorageNode;
class User;
class Node;

using namespace std;

class MyToolbox {
 public:
  typedef unsigned long MyTime;

  static constexpr double kLightSpeed = 299792458; // meter / seconds

  //  Global values
  static int num_storage_nodes;
  static int num_sensors;
  static int max_num_users;
  static int num_users;

  static int num_bits_for_id;
  static int num_bits_for_measure;
  static int num_bits_phy_mac_overhead;
  static int num_bits_for_measure_id;

  static double bitrate;
  static double bit_error_prob;
  static double ray_length;
  static double tx_range;

  static int ping_frequency;
  static int check_sensors_frequency; 

  static double C1;
  static int max_num_hops;

  static int square_size;
  static int space_precision;

  static double user_velocity;
  static MyTime user_update_time;

  static MyTime mean_processing_time;
  static MyTime std_dev_processing_time;

  static MyTime max_tx_offset;
  static MyTime max_tx_offset_ping;

  static MyTime max_measure_generation_delay;
  static double sensor_failure_prob;

  // Maps <node_id, node_reference>
  static map<unsigned int, Node*>* sensors_map_ptr;
  static map<unsigned int, Node*>* storage_nodes_map_ptr; 
  static map<unsigned int, Node*>* users_map_ptr; 
  
  // Vectors of the nodes  TODO remove!!
  static vector<SensorNode*> sensor_nodes_;
  static vector<StorageNode*> storage_nodes_;
  static vector<User*> users_;

  // TODO all of these variables are public
  // getters
  static MyTime get_current_time() {return current_time_;}
  static map<unsigned int, MyTime> get_timetable() {return timetable_;}

  static int get_n() {return n_;}
  static int get_k() {return k_;}
  static int get_bits_for_measure() {return bits_for_measure_;}
  static int get_bits_for_id() {return bits_for_id_;}
  static int get_bits_for_phy_mac_overhead() {return bits_for_phy_mac_overhead_;}
  static int get_bits_for_measure_id() {return bits_for_measure_id_;}
  static int get_bits_for_hop_counter() {return bits_for_hop_counter_;}
  static int get_max_msg_hops() {return max_msg_hops_;}
  static double get_channel_bit_rate_() {return channel_bit_rate_;}
  static int get_ping_frequency() {return ping_frequency_;}
  static int get_check_sensors_frequency_() {return check_sensors_frequency_;}
  static int get_tx_range() {return tx_range_;}
  static long get_user_update_time(){return user_update_time_;} 
  static int get_user_size() {return users_.size();}
  static int get_space_precision(){return space_precision_;}
  static int get_square_size(){return square_size_;} 

  // setters
  static void set_current_time(MyTime);
  static void set_timetable(map<unsigned int, MyTime>);

  static void set_n(int);
  static void set_k(int);
  static void set_C1(int);
  static void set_bits_for_measure(int);
  static void set_bits_for_id(int);
  static void set_bits_for_phy_mac_overhead(int);
  static void set_bits_for_measure_id(int);
  static void set_bits_for_hop_counter(int);
  static void set_channel_bit_rate(double);
  static void set_ping_frequency(int);
  static void set_check_sensors_frequency(int);
  static void set_user_velocity(double);
  static void set_user_update_time();
  static void set_tx_range(int);
  static void set_space_precision(int);
  static void set_square_size(int);
  
  static void set_sensor_nodes(vector<SensorNode*>);
  static void set_storage_nodes(vector<StorageNode*>);
  static void set_users(vector<User*>);
  static void set_near_storage_node(Node*);
  static void set_near_user(Node*);
  
  static void remove_near_storage_node(Node*, StorageNode*);    //remove a StorageNode from the near_storage_node of Node
  static void remove_near_user(Node*, User*);   //remove a User from the near_suser of Node
  static User* new_user();
  
  // functions
  static void initialize_toolbox(); // TODO
  static bool is_node_active(unsigned int /*node_id*/);
  static unsigned int get_node_id();
  static int get_ideal_soliton_distribution_degree();
  static int get_robust_soliton_distribution_degree();  // still to implement!
  static MyTime get_random_processing_time();
  // Return a uniform random offset a node must wait when it finds the channel busy.
  // The value is uniformly distributed between 1ns and MAX_OFFSET ns
  // static MyTime get_retransmission_offset();  // TODO remove
  static MyTime get_tx_offset();
  static MyTime get_tx_offset_ping();
  static void remove_sensor(unsigned int /*sensor_id*/);

 private:
  /*  This timetable contains pairs of the type:
        - key = node_id
        - value = time at which the node is going to be "left free"
  */
  static map<unsigned int, MyTime> timetable_;
  static MyTime current_time_; // to keep track of the time
  static unsigned int node_id_; 

  // TODO remove everything following
  static const int MEAN_PROCESSING_TIME = 100000; // 100us, in nano-seconds
  static constexpr double STD_DEV_PROCESSING_TIME = 1000000; // 1ms, in nano-seconds

  // TODO: remove this from here! Should be more accessible from a user
  static const MyTime MAX_OFFSET = 1000000; // 1ms, in nano-seconds

  static int n_;  // number of storage nodes in the network
  static int k_;  // number of sensors in the network
  static int C1_; // parameter to set the max number of hops of a source block
  static int bits_for_measure_; // number of bits to represent a measure
  static int bits_for_id_; // number of bits to represent a node identifier
  static int bits_for_phy_mac_overhead_; // number of bits for PHY and MAC overhead
  static int bits_for_measure_id_;  // number of bits to represent a measure id
  static int bits_for_hop_counter_; // number of bits to represent the hop_counter
  static int max_msg_hops_; // maximum number of steps a message can do from the source, in the RW
  static double channel_bit_rate_;  // bit rate of the wireless channel, in bit/seconds
  static int ping_frequency_; // frequency at which sensors do "ping"
  static int check_sensors_frequency_; // frequency at which node chek if its supervisioned sensors are alive
  static double user_velocity_;    //velocity of the user in the network [m/s]
  static long user_update_time_; //frequency at which we move the users
  static int tx_range_;
  static int space_precision_;
  static int square_size_;  
};

#endif