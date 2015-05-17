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

 public:
  DataCollector();

//  std::vector<int> user_tx_delay;
//  int user_tx_delay;

//  std::vector<std::pair<unsigned int, unsigned int>> measures;	// measure identified by measure id and sensor id, in this order
//  std::map<unsigned int, int> cache_msr;
  std::map<std::pair<unsigned int, unsigned int>, std::vector<unsigned int>> msr_register;
  std::map<MeasureKey, std::map<unsigned int, int>> msr_register2;

  void print_data();
  void add_msr(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void add_msr2(unsigned int /*msr id*/, unsigned int /*sns id*/);
  void record_msr(unsigned int /*msr id*/, unsigned int /*sns id*/, unsigned int /*cache id*/, unsigned int /*sym*/);
  void record_msr2(unsigned int /*msr id*/, unsigned int /*sns id*/, unsigned int /*cache id*/, unsigned int /*sym*/);

 private:
  long double avg_user_tx_delay;
};

#endif
