#include "data_collector.h"
#include "my_toolbox.h"

#include <iostream>

using namespace std;

DataCollector::DataCollector() {
//  avg_user_tx_delay = 0;
}

void DataCollector::print_data() {
//	string data = "no data";
//	for (map<pair<unsigned int, unsigned int>, vector<unsigned int>>::iterator it = msr_register.begin(); it != msr_register.end(); it++) {
//	  pair<unsigned int, unsigned int> p = it->first;
//	  vector<unsigned int> v = it->second;
//	  cout << "(msr " << p.first << ", sns " << p.second;
//	  int counter_seen = 0;
//	  int counter_held = 0;
//	  int counter_tot = 0;
//	  for (vector<unsigned int>::iterator it_v = v.begin(); it_v != v.end(); it_v++) {
//	    if (*it_v == 1) {
//	      counter_seen++;
//	      counter_tot++;
//	    } else if (*it_v == 2) {
//	      counter_held++;
//	      counter_tot++;
//	    }
//	  }
//	  cout << "): seen " << counter_seen << ", held " << counter_held << " (tot " << counter_tot << ") | ";
//	}
//	cout << endl;
//	cout << data << endl;
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// create the key
	MeasureInfo measureInfo;
	map<unsigned int, StorageNode*> cache_map = MyToolbox::storage_nodes_map_ptr_;	// take the map of the storage nodes
	map<unsigned int, int> node_map;	// create the node map to associate to this key
	measureInfo.born_time_ = MyToolbox::current_time_;
	measureInfo.node_map_ = node_map;
//	for (map<unsigned int, Node*>::iterator node_it = cache_map.begin(); node_it != cache_map.end(); node_it++) { 	// fill the map with each cache id and set the value to 0 by default
//		unsigned int curr_node_id = node_it->first;
//		node_map.insert(pair<unsigned int, int>(curr_node_id, 0));
//	}
	measures_register.insert(pair<MeasureKey, MeasureInfo>(key, measureInfo));	// store this measure entry
	cout << "Measure (s" << sns_id << ", " << msr_id << ") added" << endl;
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register.find(key) != measures_register.end()) {	// if this measure is still in the register
		if (!measures_register.find(key)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = measures_register.find(key)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this measure
				cout << "Cache " << cache_id << " saw (s" << sns_id << ", " << msr_id << ")" << endl;
				measures_register.find(key)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = measures_register.find(key)->second.node_map_.size();	// how many cache saw this measure
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this measure at least once
				measures_register.find(key)->second.crossed_the_network_ = true;
				measures_register.find(key)->second.spreading_time_ = MyToolbox::current_time_;
				MyTime spreading_time = measures_register.find(key)->second.spreading_time_ - measures_register.find(key)->second.born_time_;
				cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") crossed all the networks in " << spreading_time * 1. / 1000000000 << "s" << endl;
				//			msr_register2.erase(key);	// remove this measure
			}
		}
	}
}

void DataCollector::erase_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register.find(key) != measures_register.end()) {	// if this measure is still in the register
		measures_register.find(key)->second.death_time_ = MyToolbox::current_time_;
		MyTime life_time = measures_register.find(key)->second.death_time_ - measures_register.find(key)->second.born_time_;
		cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") was alive for " << life_time * 1. / 1000000000 << "s" << endl;
		measures_register.erase(key);	// remove this measure
	} else {	// I cannot find the measure
		cout << "Error! Measure (s" << sns_id << ", " << msr_id << ") lost!" << endl;
	}
}
