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

 public:
  DataCollector();

//  std::vector<int> user_tx_delay;
  int user_tx_delay;

  std::vector<std::pair<unsigned int, unsigned int>> measures;	// measure identified by measure id and sensr id, in this order
  std::map<unsigned int, int> cache_msr;

  void print_data();

 private:
  long double avg_user_tx_delay;
};

#endif
