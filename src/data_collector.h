#ifndef __DATA_COLLECTOR_H__   
#define __DATA_COLLECTOR_H__   

#include <vector>
#include <map>

#include "my_toolbox.h"
//class MyToolbox;

//using namespace std;

class DataCollector {
 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  struct MeasureInfo {
	  std::map<unsigned int, int> node_map_;
	  MyTime born_time_;
	  MyTime spreading_time_;
	  MyTime death_time_;
	  bool crossed_the_network_ = false;
  };

 public:
  DataCollector();

//  std::vector<int> user_tx_delay;
//  int user_tx_delay;

//  std::vector<std::pair<unsigned int, unsigned int>> measures;	// measure identified by measure id and sensor id, in this order
//  std::map<unsigned int, int> cache_msr;
  std::map<MeasureKey, MeasureInfo> measures_register;

  void print_data();
  void add_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void record_msr(unsigned int /*msr id*/, unsigned int /*sns id*/, unsigned int /*cache id*/, unsigned int /*sym*/);
  void erase_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);

 private:
  long double avg_user_tx_delay;
};

#endif
