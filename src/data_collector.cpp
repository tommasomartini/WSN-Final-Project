#include "data_collector.h"
#include "my_toolbox.h"

#include <iostream>

using namespace std;

DataCollector::DataCollector() {
//  avg_user_tx_delay = 0;
}

void DataCollector::print_data() {
	string data = "no data";
	for (map<pair<unsigned int, unsigned int>, vector<unsigned int>>::iterator it = msr_register.begin(); it != msr_register.end(); it++) {
	  pair<unsigned int, unsigned int> p = it->first;
	  cout << "msr id " << p.first << ", sns id " << p.second;
	  vector<unsigned int> v = it->second;
	  int counter = 0;
	  for (vector<unsigned int>::iterator it_v = v.begin(); it_v != v.end(); it_v++) {
	    if (*it_v > 0) {
	      counter++;
	    }
	  }
	  cout << ", tot " << counter;
	}
	cout << endl;
//	cout << data << endl;
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id) {
  vector<unsigned int> v(MyToolbox::num_storage_nodes);
  for (int i = 0; i < v.size(); i++) {
    v.at(i) = 0;
  }
  msr_register.insert(pair<pair<unsigned int, unsigned int>, vector<unsigned int>>(pair<unsigned int, unsigned int>(msr_id, sns_id), v));
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
  unsigned int ind = cache_id - MyToolbox::num_sensors;
  map<pair<unsigned int, unsigned int>, vector<unsigned int>>::iterator iter = msr_register.find(pair<unsigned int, unsigned int>(msr_id, sns_id));
  vector<unsigned int>* vv = &(iter->second);
  vector<unsigned int>::iterator v_it = vv->begin();
  for (int i = 0; i < ind; i++) {
    v_it++;
  }
  vv->insert(v_it, sym);
}
