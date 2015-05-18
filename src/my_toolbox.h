/*  
  This class provides some tools statically accessible from every class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__  

#include <vector> 
#include <map> 
#include <random> // generation of random variables -> require -std=c++11

#include "agent.h"

class SensorNode;
class StorageNode;
class User;
class Node;

//class DataCollector;

using namespace std;

class MyToolbox {
 public:
  struct MeasureKey {
    unsigned int sensor_id_;
	unsigned int measure_id_;
	MeasureKey(unsigned int sns_id, unsigned int msr_id) {
      sensor_id_ = sns_id;
      measure_id_ = msr_id;
	}
	bool operator == (const MeasureKey y) {
	  return sensor_id_ == y.sensor_id_ && measure_id_ == y.measure_id_;
	}
	bool operator < (const MeasureKey &y) const {
	  if (sensor_id_ < y.sensor_id_) {
		  return true;
	  } else if (sensor_id_ > y.sensor_id_) {
		return false;
	  } else { 	// equal sensor ids, check the
		if (measure_id_ < y.measure_id_) {
		  return true;
		}
		return false;
	  }
	}
//	string toString() {
//		string str = "(s" + sensor_id_ + ", " + measure_id_ + ")";
//		return str;
//	}
  };

  // TODO replace all the unsigned char with this
  struct MeasureData {
    unsigned int data_;
    MeasureData(unsigned int data) {
      data_ = data;
    }
  };

  typedef unsigned long MyTime;

  static constexpr double kLightSpeed = 299792458; // meter / seconds

//  static DataCollector* dc;

  //  Global values
  static int num_storage_nodes;
  static int num_sensors;
  static int num_users;

  static int num_bits_for_measure;

  static double bitrate;
  static double bit_error_prob;
  static double tx_range;

  static MyTime ping_frequency;
  static MyTime check_sensors_frequency;

  static double C1;
  static int max_num_hops;

  static int square_size;
  static int space_precision;

  static double user_velocity;
  static MyTime user_update_time;

  static MyTime processing_time;

  static MyTime max_tx_offset;

  static MyTime user_observation_time;

  static MyTime max_measure_generation_delay;
  static double sensor_failure_prob;

  // Maps <node_id, node_reference>
  static map<unsigned int, Node*>* sensors_map_ptr;
  static map<unsigned int, Node*>* storage_nodes_map_ptr; 
  static map<unsigned int, Node*>* users_map_ptr; 

  // TODO all of these variables are public
  // getters
  static MyTime get_current_time() {return current_time_;}
  static map<unsigned int, MyTime> get_timetable() {return timetable_;}
  static default_random_engine get_random_generator() {return generator;}
  static int get_bits_for_measure() {return num_bits_for_measure;}
  static int get_space_precision() {return space_precision;}

  // setters
  static void set_current_time(MyTime);
  static void set_timetable(map<unsigned int, MyTime>);

  static void set_user_update_time();
  static void set_sensor_nodes(vector<SensorNode*>);
  static void set_storage_nodes(vector<StorageNode*>);
  static void set_users(vector<User*>);
  static void set_near_storage_node(Node*);
  static void set_near_user(Node*);
  
  static void remove_near_storage_node(Node*, StorageNode*);    //remove a StorageNode from the near_storage_node of Node
  static void remove_near_user(Node*, User*);   //remove a User from the near_suser of Node
//  static User* new_user();
  
  // functions
  static void initialize_toolbox(); // TODO
  static bool is_node_active(unsigned int /*node_id*/);
  static unsigned int get_node_id();
  static int get_ideal_soliton_distribution_degree();
  static int get_robust_soliton_distribution_degree();  // still to implement!
  static MyTime get_random_processing_time();
  static void set_close_nodes(User* user);
  // Return a uniform random offset a node must wait when it finds the channel busy.
  // The value is uniformly distributed between 1ns and MAX_OFFSET ns
  // static MyTime get_retransmission_offset();  // TODO remove
  static MyTime get_tx_offset();
  static void remove_sensor(unsigned int /*sensor_id*/);

  static string int2nodetype(unsigned int);	// TODO just for debug
  static int show_clouds();	// TODO just for debugging

 private:
  /*  This timetable contains pairs of the type:
        - key = node_id
        - value = time at which the node is going to be "left free"
  */
  static map<unsigned int, MyTime> timetable_;  // says when a node gets free
  static MyTime current_time_; // to keep track of the time
  static unsigned int node_id_; 
  static default_random_engine generator;
};

#endif
