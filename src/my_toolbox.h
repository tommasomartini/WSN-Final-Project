/*  
  This class provides some tools statically accessible from every class;
*/

#ifndef __MY_TOOLBOX_H__   
#define __MY_TOOLBOX_H__  

#include <vector> 
#include <map> 
#include <random> // generation of random variables -> require -std=c++11

#include "agent.h"

//class Node;
class SensorNode;
class StorageNode;
class User;

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

  struct OutputSymbol {
	  unsigned char xored_msg_;
	  std::vector<MeasureKey> sources_;	// keys of the measures which compose the xor
	  std::vector<MeasureKey> outdated_;	// keys of the outdated measures (subset of sources)
	  OutputSymbol(unsigned char xored_msg, std::vector<MeasureKey> sources, std::vector<MeasureKey> outdated) {
		  xored_msg_ = xored_msg;
		  sources_ = sources;
		  outdated_ = outdated;
	  }
  };

  // TODO replace all the unsigned char with this
  struct MeasureData {
    unsigned int data_;
    MeasureData(unsigned int data) {
      data_ = data;
    }
  };

  typedef unsigned long MyTime;
  typedef unsigned int IdType;

  //  Global values
  static int num_storage_nodes_;
  static int num_sensors_;
  static int num_users_;

  static int num_bits_for_measure_;

  static double bitrate_;
  static double bit_error_prob_;
  static double tx_range_;

  static MyTime ping_frequency_;
  static MyTime check_sensors_frequency_;

  static double C1_;
  static int max_num_hops_;

  static int square_size_;
  static int space_precision_;

  static double user_velocity_;
  static MyTime user_update_time_;

  static MyTime processing_time_;

  static MyTime max_tx_offset_;

  static MyTime user_observation_time_;

  static MyTime max_measure_generation_delay_;
  static double sensor_failure_prob_;

  // Maps <node_id, node_reference>
  static std::map<unsigned int, SensorNode> sensors_map_;
  static std::map<unsigned int, StorageNode> storage_nodes_map_;
  static std::map<unsigned int, User> users_map_;

  static std::map<unsigned int, MyTime> timetable_;  // says when a node gets free

  static MyTime current_time_; // to keep track of the time

  static std::default_random_engine generator_;

  // functions
  static void initialize_toolbox(); // TODO
  static bool is_node_active(unsigned int /*node_id*/);
  static unsigned int get_node_id();
  static int get_ideal_soliton_distribution_degree();
  static int get_robust_soliton_distribution_degree();  // still to implement!
  static MyTime get_random_processing_time();
  static void set_close_nodes(User* user);
  static MyTime get_tx_offset();
  static void remove_sensor(unsigned int /*sensor_id*/);

  static std::string int2nodetype(unsigned int);	// TODO just for debug
  static int check_clouds();
  static int check_clouds2();
  static bool sensor_connected();

 private:
  static unsigned int node_id_; 
};

#endif
