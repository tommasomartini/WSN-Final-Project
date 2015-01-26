#ifndef __DATA_COLLECTOR_H__   
#define __DATA_COLLECTOR_H__   

#include <vector>
#include <map>

#include "my_toolbox.h"

using namespace std;

class DataCollector {
 private:
  typedef MyToolbox::MyTime MyTime;

 public:
  vector<MyTime> user_tx_delay;

 private:
  long double avg_user_tx_delay;
};

#endif