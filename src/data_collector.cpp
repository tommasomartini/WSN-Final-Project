#include "data_collector.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"

#include <iostream>

using namespace std;

DataCollector::DataCollector() {
}

void DataCollector::report() {

	// Mesures
	MyTime avg_msr_time = 0;
	int avg_not_crossed = 0;
	int avg_counter = 0;
	int num_msrs = measures_register_.size();
	for (map<MeasureKey, MeasureInfo>::iterator it = measures_register_.begin(); it != measures_register_.end(); it++) {
		if (it->second.crossed_the_network_) {
			avg_counter++;
			avg_msr_time += it->second.spreading_duration_;
		} else {
			avg_not_crossed++;
		}
	}
	avg_msr_time = avg_msr_time / avg_counter;

	// Blacklist
	MyTime avg_bl_time = 0;
	int bl_not_crossed = 0;
	int bl_counter = 0;
	int num_bl = blacklist_register_.size();
	for (map<unsigned int, BlacklistInfo>::iterator it = blacklist_register_.begin(); it != blacklist_register_.end(); it++) {
		if (it->second.crossed_the_network_) {
			bl_counter++;
			avg_bl_time += it->second.spreading_duration_;
		} else {
			bl_not_crossed++;
		}
	}
	avg_bl_time = avg_bl_time / bl_counter;

	cout << " > Report <" << endl;
	cout << "- Avg measure spreding time: " << avg_msr_time * 1. / pow(10, 9) << endl;
	cout << "- " << avg_not_crossed << " measures (" << avg_not_crossed * 1. / num_msrs * 100 << "%) did not cross the network" << endl;

	cout << "- Avg blacklist spreding time: " << avg_bl_time * 1. / pow(10, 9) << endl;
	cout << "- " << bl_not_crossed << " blacklist (" << bl_not_crossed * 1. / num_bl * 100 << "%) did not cross the network" << endl;

	cout << "- Soliton check: " << endl;
	int zero_counter = 0;
	int one_counter = 0;
	int miss_counter = 0;
	for (map<unsigned int, int>::iterator it = num_stored_measures_per_cache_.begin(); it != num_stored_measures_per_cache_.end(); it++) {
		int lt_deg = MyToolbox::storage_nodes_map_.find(it->first)->second.LT_degree_;
		cout << "   Node " << it->first << " (d = " << lt_deg << ") stores " << it->second << " measures" << endl;
		if (it->second == 0) {
			zero_counter++;
		}
		if (it->second == 1) {
			one_counter++;
		}
		if (lt_deg != it->second) {
			miss_counter++;
		}
	}
	if (zero_counter > 0) {
		cout << "  " << zero_counter << " nodes store no measures" << endl;
	}
	if (one_counter > 0) {
		cout << "  " << one_counter << " nodes store 1 measure" << endl;
	}
	if (miss_counter > 0) {
		cout << "  " << miss_counter << " nodes store a wrong number of measures" << endl;
	}
}

void DataCollector::update_num_msr_per_cache(unsigned int cache_id, int num_msrs) {
	if (num_stored_measures_per_cache_.find(cache_id) != num_stored_measures_per_cache_.end()) {	// cache in the map
		num_stored_measures_per_cache_.erase(cache_id);
	}
	num_stored_measures_per_cache_.insert(pair<unsigned int, int>(cache_id, num_msrs));
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// create the key
	MeasureInfo measureInfo;
//	map<unsigned int, StorageNode> cache_map = MyToolbox::storage_nodes_map_;	// take the map of the storage nodes
//	map<unsigned int, int> node_map;	// create the node map to associate to this key
	measureInfo.born_time_ = MyToolbox::current_time_;
//	measureInfo.node_map_ = node_map;
	measures_register_.insert(pair<MeasureKey, MeasureInfo>(key, measureInfo));	// store this measure entry
//	cout << "Measure (s" << sns_id << ", " << msr_id << ") added" << endl;
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register_.find(key) != measures_register_.end()) {	// if this measure is still in the register
		if (!measures_register_.find(key)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = measures_register_.find(key)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this measure
//				cout << "Cache " << cache_id << " saw (s" << sns_id << ", " << msr_id << ")" << endl;
				measures_register_.find(key)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = measures_register_.find(key)->second.node_map_.size();	// how many cache saw this measure
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this measure at least once
				measures_register_.find(key)->second.crossed_the_network_ = true;
				measures_register_.find(key)->second.spreading_time_ = MyToolbox::current_time_;
				measures_register_.find(key)->second.spreading_duration_ = measures_register_.find(key)->second.spreading_time_ - measures_register_.find(key)->second.born_time_;
//				cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") crossed all the networks in " << measures_register_.find(key)->second.spreading_duration_ * 1. / 1000000000 << "s" << endl;
				//			msr_register2.erase(key);	// remove this measure
			}
		}
	} else {
		cout << "Error! Trying to record a measure not in the register!" << endl;
	}
}

void DataCollector::erase_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register_.find(key) != measures_register_.end()) {	// if this measure is still in the register
		measures_register_.find(key)->second.death_time_ = MyToolbox::current_time_;
		measures_register_.find(key)->second.travel_duration_ = measures_register_.find(key)->second.death_time_ - measures_register_.find(key)->second.born_time_;
//		cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") was alive for " << measures_register_.find(key)->second.travel_duration_ * 1. / 1000000000 << "s" << endl;
//		measures_register.erase(key);	// remove this measure
	} else {	// I cannot find the measure
//		cout << "Error! Measure (s" << sns_id << ", " << msr_id << ") lost!" << endl;
	}
}

void DataCollector::register_broken_sensor(unsigned int sensor_id_) {
	if (blacklist_register_.find(sensor_id_) == blacklist_register_.end()) {	// if there is not such an entry
		BlacklistInfo bl_info;
		bl_info.born_time_ = MyToolbox::current_time_;
		blacklist_register_.insert(pair<unsigned int, BlacklistInfo>(sensor_id_, bl_info));
		cout << "Blacklist " << sensor_id_ << " added" << endl;
	} else {
		cout << "Error! Trying to remove a sensor twice!" << endl;
	}
}

void DataCollector::record_bl(unsigned int cache_id, unsigned int sensor_id) {
	if (blacklist_register_.find(sensor_id) != blacklist_register_.end()) {	// if this blacklist is still in the register
		if (!blacklist_register_.find(sensor_id)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = blacklist_register_.find(sensor_id)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this blacklist
				cout << "Cache " << cache_id << " saw bl" << sensor_id << endl;
				blacklist_register_.find(sensor_id)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = blacklist_register_.find(sensor_id)->second.node_map_.size();	// how many cache saw this blacklist
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this blacklist at least once
				blacklist_register_.find(sensor_id)->second.crossed_the_network_ = true;
				blacklist_register_.find(sensor_id)->second.spreading_time_ = MyToolbox::current_time_;
				blacklist_register_.find(sensor_id)->second.spreading_duration_ = blacklist_register_.find(sensor_id)->second.spreading_time_ - blacklist_register_.find(sensor_id)->second.born_time_;
				cout << "Blacklist " << sensor_id << " crossed all the networks in " << blacklist_register_.find(sensor_id)->second.spreading_duration_ * 1. / 1000000000 << "s" << endl;
			}
		}
	} else {
		cout << "Error! Trying to record a blacklist not in the register!" << endl;
	}
}

void DataCollector::erase_bl(unsigned int sensor_id) {
	if (blacklist_register_.find(sensor_id) != blacklist_register_.end()) {	// if this blacklist is still in the register
		blacklist_register_.find(sensor_id)->second.death_time_ = MyToolbox::current_time_;
		blacklist_register_.find(sensor_id)->second.travel_duration_ = blacklist_register_.find(sensor_id)->second.death_time_ - blacklist_register_.find(sensor_id)->second.born_time_;
		cout << "Blacklist bl" << sensor_id << " was alive for " << blacklist_register_.find(sensor_id)->second.travel_duration_ * 1. / 1000000000 << "s" << endl;
	} else {	// I cannot find the blacklist
		cout << "Error! BLacklist bl" << sensor_id << " lost!" << endl;
	}
}

double DataCollector::graph_density() {
	double num_outer_edges = 0;
	map<unsigned int, StorageNode> node_map = MyToolbox::storage_nodes_map_;
	for (map<unsigned int, StorageNode>::iterator it = node_map.begin(); it != node_map.end(); it++) {
		double edges = it->second.near_storage_nodes_.size();
		num_outer_edges += edges;
	}
	double E = num_outer_edges / 2;
	double V = double(MyToolbox::num_storage_nodes_);
	return 2 * E / (V * (V - 1));
}
