#include "data_collector.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"

#include <iostream>

using namespace std;

DataCollector::DataCollector() {
}

void DataCollector::report() {
	MyTime avg_time = 0;
	int not_crossed = 0;
	int counter = 0;
	int num_msrs = measures_register.size();
	for (map<MeasureKey, MeasureInfo>::iterator it = measures_register.begin(); it != measures_register.end(); it++) {
		if (it->second.crossed_the_network_) {
			counter++;
			avg_time += it->second.spreading_duration_;
		} else {
			not_crossed++;
		}
	}
	avg_time = avg_time / counter;

	cout << " > Report <" << endl;
	cout << "- Avg spreding time: " << avg_time * 1. / pow(10, 9) << endl;
	cout << "- " << not_crossed << " messages (" << not_crossed * 1. / num_msrs * 100 << "%) did not cross the network" << endl;
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// create the key
	MeasureInfo measureInfo;
	map<unsigned int, StorageNode> cache_map = MyToolbox::storage_nodes_map_;	// take the map of the storage nodes
//	map<unsigned int, int> node_map;	// create the node map to associate to this key
	measureInfo.born_time_ = MyToolbox::current_time_;
//	measureInfo.node_map_ = node_map;
	measures_register.insert(pair<MeasureKey, MeasureInfo>(key, measureInfo));	// store this measure entry
	cout << "Measure (s" << sns_id << ", " << msr_id << ") added" << endl;
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register.find(key) != measures_register.end()) {	// if this measure is still in the register
		if (!measures_register.find(key)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = measures_register.find(key)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this measure
//				cout << "Cache " << cache_id << " saw (s" << sns_id << ", " << msr_id << ")" << endl;
				measures_register.find(key)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = measures_register.find(key)->second.node_map_.size();	// how many cache saw this measure
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this measure at least once
				measures_register.find(key)->second.crossed_the_network_ = true;
				measures_register.find(key)->second.spreading_time_ = MyToolbox::current_time_;
				measures_register.find(key)->second.spreading_duration_ = measures_register.find(key)->second.spreading_time_ - measures_register.find(key)->second.born_time_;
				cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") crossed all the networks in " << measures_register.find(key)->second.spreading_duration_ * 1. / 1000000000 << "s" << endl;
				//			msr_register2.erase(key);	// remove this measure
			}
		}
	}
}

void DataCollector::erase_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register.find(key) != measures_register.end()) {	// if this measure is still in the register
		measures_register.find(key)->second.death_time_ = MyToolbox::current_time_;
		measures_register.find(key)->second.travel_duration_ = measures_register.find(key)->second.death_time_ - measures_register.find(key)->second.born_time_;
		cout << "Measure (s" << key.sensor_id_ << ", " << key.measure_id_ << ") was alive for " << measures_register.find(key)->second.travel_duration_ * 1. / 1000000000 << "s" << endl;
//		measures_register.erase(key);	// remove this measure
	} else {	// I cannot find the measure
		cout << "Error! Measure (s" << sns_id << ", " << msr_id << ") lost!" << endl;
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
