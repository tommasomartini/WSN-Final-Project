#ifndef __DATA_COLLECTOR_H__   
#define __DATA_COLLECTOR_H__   

#include <vector>
#include <map>

#include "my_toolbox.h"

class DataCollector {
 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  struct MeasureInfo {
	  std::map<unsigned int, int> node_map_;
	  int hop_number_ = 0;
	  MyTime born_time_ = 0;	// instants the message was started spreading
	  MyTime spreading_time_ = 0;	// instant the message crossed for the first time all the nodes
	  MyTime death_time_ = 0;	// instant its hop counter went over
	  MyTime spreading_duration_ = 0;	// interval the message took to reach every node (spreading_time_ - born_time_)
	  MyTime travel_duration_ = 0;	// interval the message was around (death_time_ - born_time_)
	  bool crossed_the_network_ = false;
  };

  struct BlacklistInfo {
	  std::map<unsigned int, int> node_map_;
	  int hop_number_ = 0;
	  MyTime born_time_ = 0;	// instants the message was started spreading
	  MyTime spreading_time_ = 0;	// instant the message crossed for the first time all the nodes
	  MyTime death_time_ = 0;	// instant its hop counter went over
	  MyTime spreading_duration_ = 0;	// interval the message took to reach every node (spreading_time_ - born_time_)
	  MyTime travel_duration_ = 0;	// interval the message was around (death_time_ - born_time_)
	  bool crossed_the_network_ = false;
  };

  struct UserInfo {
	  int num_steps_ = 0;
	  double covered_distance_ = 0;
	  MyTime born_time_ = 0;
	  MyTime decoding_time_ = 0;
	  MyTime decoding_duration_ = 0;
	  double decoding_distance_ = 0;
	  int decoding_steps_ = 0;
	  int num_rx_node_info_ = 0;
	  bool decoded_ = false;
	  int num_interrogated_caches_ = 0;
	  int num_interrogated_users_ = 0;
  };

 public:
  DataCollector();

  std::map<unsigned int, int> num_stored_measures_per_cache_;
  std::map<MeasureKey, MeasureInfo> measures_register_;
  std::map<unsigned int, BlacklistInfo> blacklist_register_;
  std::map<unsigned int, UserInfo> user_register_;

  void report();
  double graph_density();
  void update_num_msr_per_cache(unsigned int, int);
  void register_broken_sensor(unsigned int);
  void record_bl(unsigned int /*node id*/, unsigned int /*sensor id*/);
  void erase_bl(unsigned int);
  void add_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void record_msr(unsigned int /*msr id*/, unsigned int /*sns id*/, unsigned int /*cache id*/, unsigned int /*sym*/);
  void erase_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void record_user_movement(unsigned int, double /*distance*/);
  void record_user_rx(unsigned int);
  void record_user_decoding(unsigned int);
  void record_user_query(unsigned int /*user id*/, unsigned int /*node or user id*/, bool /*is a node*/);

 private:
};

#endif
