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
	  vector<unsigned int> v = it->second;
	  cout << "(msr " << p.first << ", sns " << p.second;
	  int counter_seen = 0;
	  int counter_held = 0;
	  int counter_tot = 0;
	  for (vector<unsigned int>::iterator it_v = v.begin(); it_v != v.end(); it_v++) {
	    if (*it_v == 1) {
	      counter_seen++;
	      counter_tot++;
	    } else if (*it_v == 2) {
	      counter_held++;
	      counter_tot++;
	    }
	  }
	  cout << "): seen " << counter_seen << ", held " << counter_held << " (tot " << counter_tot << ") | ";
	}
	cout << endl;
//	cout << data << endl;
}

void DataCollector::add_msr2(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// create the key
	map<unsigned int, Node*> cache_map = *(MyToolbox::storage_nodes_map_ptr);	// take the map of the storage nodes
	map<unsigned int, int> node_map;	// create the node map to associate to this key
//	for (map<unsigned int, Node*>::iterator node_it = cache_map.begin(); node_it != cache_map.end(); node_it++) { 	// fill the map with each cache id and set the value to 0 by default
//		unsigned int curr_node_id = node_it->first;
//		node_map.insert(pair<unsigned int, int>(curr_node_id, 0));
//	}
	msr_register2.insert(pair<MeasureKey, map<unsigned int, int>>(key, node_map));	// store this measure entry
	cout << "misura (s" << sns_id << ", " << msr_id << ") aggiunta" << endl;
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id) {
  vector<unsigned int> v(MyToolbox::num_storage_nodes);
  for (int i = 0; i < v.size(); i++) {
    v.at(i) = 0;
  }
  msr_register.insert(pair<pair<unsigned int, unsigned int>, vector<unsigned int>>(pair<unsigned int, unsigned int>(msr_id, sns_id), v));
}

void DataCollector::record_msr2(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (msr_register2.find(key) != msr_register2.end()) {	// if this measure is still in the register
		map<unsigned int, int> node_map = msr_register2.find(key)->second;	// get the map associated with this measure
		if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this measure
			msr_register2.find(key)->second.insert(pair<unsigned int, int>(cache_id, 1));
		} else {	// this node already saw this measure
			// do nothing
		}
		int num_visits = msr_register2.find(key)->second.size();	// how many cache saw this measure
		if (num_visits == MyToolbox::num_storage_nodes) {	// if every cache saw this measure at least once
			cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") crossed all the networks!" << endl;
			msr_register2.erase(key);	// remove this measure
		}
	}
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
  unsigned int ind = cache_id - MyToolbox::num_sensors;
  map<pair<unsigned int, unsigned int>, vector<unsigned int>>::iterator iter = msr_register.find(pair<unsigned int, unsigned int>(msr_id, sns_id));
  vector<unsigned int>* vv = &(iter->second);
  vector<unsigned int>::iterator v_it = vv->begin();
  for (int i = 0; i < ind; i++) {
    v_it++;
  }
  if (*v_it == 0 || (*v_it == 1 && sym == 2)) {
	//vv->insert(v_it, sym);
	  *v_it = sym;
  }

//  print_data();
}
