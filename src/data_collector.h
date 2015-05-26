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
	  MyTime born_time_ = 0;	// instants the message was started spreading
	  MyTime spreading_time_ = 0;	// instant the message crossed for the first time all the nodes
	  MyTime death_time_ = 0;	// instant its hop counter went over
	  MyTime spreading_duration_ = 0;	// interval the message took to reach every node (spreading_time_ - born_time_)
	  MyTime travel_duration_ = 0;	// interval the message was around (death_time_ - born_time_)
	  bool crossed_the_network_ = false;
  };

 public:
  DataCollector();

  std::map<unsigned int, int> num_stored_measures_per_cache_;
  std::map<MeasureKey, MeasureInfo> measures_register;

  void report();
  double graph_density();
  void update_num_msr_per_cache(unsigned int, int);
  void add_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void record_msr(unsigned int /*msr id*/, unsigned int /*sns id*/, unsigned int /*cache id*/, unsigned int /*sym*/);
  void erase_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);

 private:
};

#endif
