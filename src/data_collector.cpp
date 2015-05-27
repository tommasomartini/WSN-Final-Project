#include "data_collector.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"

#include <iostream>

using namespace std;

DataCollector::DataCollector() {
}

void DataCollector::report() {

	cout << " > Report <" << endl;

	// Blacklist
	if (blacklist_register_.size() > 0) {
		double avg_bl_time = 0;
		double avg_bl_life_time = 0;
		int bl_not_crossed = 0;
		int bl_counter = 0;
		double avg_bl_hops = 0;
		int num_wrong_hops_bl = 0;
		int num_bl = blacklist_register_.size();
		for (map<unsigned int, BlacklistInfo>::iterator it = blacklist_register_.begin(); it != blacklist_register_.end(); it++) {
			avg_bl_life_time += it->second.travel_duration_;
			avg_bl_hops += it->second.hop_number_;
			if (it->second.hop_number_ < MyToolbox::max_num_hops_) {
				num_wrong_hops_bl++;
			}
			if (it->second.crossed_the_network_) {
				bl_counter++;
				avg_bl_time += it->second.spreading_duration_;
			} else {
				bl_not_crossed++;
			}
		}
		avg_bl_time = avg_bl_time / bl_counter;
		avg_bl_life_time = avg_bl_life_time / blacklist_register_.size();
		avg_bl_hops = avg_bl_hops / blacklist_register_.size();

		cout << "+ BLACKLIST" << endl;
		cout << " - Avg blacklist spreding time: " << avg_bl_time * 1. / pow(10, 9) << "s" << endl;
		cout << " - Avg blacklist life time: " << avg_bl_life_time * 1. / pow(10, 9) << "s (" << avg_bl_hops << " hops on avg out of " << MyToolbox::max_num_hops_ << ", " << num_wrong_hops_bl << " wrong hops)" << endl;
		cout << " - " << bl_not_crossed << " blacklists (" << bl_not_crossed * 1. / num_bl * 100 << "%) did not cross the network" << endl;
	}

	// Mesures
	if (measures_register_.size() > 0) {
		double avg_msr_time = 0;
		double avg_msr_life_time = 0;
		int avg_not_crossed = 0;
		int avg_counter = 0;
		double avg_msr_hops = 0;
		int num_wrong_hops_msr = 0;
		int num_msrs = measures_register_.size();
		for (map<MeasureKey, MeasureInfo>::iterator it = measures_register_.begin(); it != measures_register_.end(); it++) {
			avg_msr_life_time += it->second.travel_duration_;
			avg_msr_hops += it->second.hop_number_;
			if (it->second.hop_number_ < MyToolbox::max_num_hops_) {
				num_wrong_hops_msr++;
			}
			if (it->second.crossed_the_network_) {
				avg_counter++;
				avg_msr_time += it->second.spreading_duration_;
			} else {
				avg_not_crossed++;
			}
		}
		avg_msr_time = avg_msr_time / avg_counter;
		avg_msr_life_time = avg_msr_life_time / measures_register_.size();
		avg_msr_hops = avg_msr_hops / measures_register_.size();

		cout << "+ MEASURE" << endl;
		cout << " - Avg measure spreding time: " << avg_msr_time * 1. / pow(10, 9) << "s" << endl;
		cout << " - Avg measure life time: " << avg_msr_life_time * 1. / pow(10, 9) << "s (" << avg_msr_hops << " hops on avg out of " << MyToolbox::max_num_hops_ << ", " << num_wrong_hops_msr << " wrong hops)" << endl;
		cout << " - " << avg_not_crossed << " measures (" << avg_not_crossed * 1. / num_msrs * 100 << "%) did not cross the network" << endl;
	}

	//User
	if (user_register_.size() > 0) {
		double avg_user_dec_time = 0;
		double avg_dec_num_steps = 0;
		double avg_dec_distance = 0;
		int decoding_number = 0;
		int num_usrs = user_register_.size();
		for (map<unsigned int, UserInfo>::iterator it = user_register_.begin(); it != user_register_.end(); it++) {
			if (it->second.decoded_) {
				decoding_number++;
				avg_user_dec_time += it->second.decoding_duration_;
				avg_dec_num_steps += it->second.decoding_steps_;
				avg_dec_distance += it->second.decoding_distance_;
			}
		}
		avg_user_dec_time = avg_user_dec_time / decoding_number;
		avg_dec_num_steps = avg_dec_num_steps / decoding_number;
		avg_dec_distance = avg_dec_distance / decoding_number;

		cout << "+ USER" << endl;
		cout << " - Avg decoding time: " << avg_user_dec_time * 1. / pow(10, 9) << "s" << endl;
		cout << " - Avg decoding distance: " << avg_dec_distance << "m" << endl;
		cout << " - Avg decoding number of steps: " << avg_dec_num_steps << endl;
		cout << " - " << decoding_number << " users out of " << num_usrs << " (" << decoding_number * 100.0 / num_usrs << "%) decoded the messages" << endl;
	}

	cout << "+ SOLITON CHECK" << endl;
	int zero_counter = 0;
	int one_counter = 0;
	int miss_counter = 0;
	for (map<unsigned int, int>::iterator it = num_stored_measures_per_cache_.begin(); it != num_stored_measures_per_cache_.end(); it++) {
		int lt_deg = MyToolbox::storage_nodes_map_.find(it->first)->second.LT_degree_;
		//		cout << " Node " << it->first << " (d = " << lt_deg << ") stores " << it->second << " measures" << endl;
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
		cout << " " << zero_counter << " nodes store no measures" << endl;
	}
	if (one_counter > 0) {
		cout << " " << one_counter << " nodes store 1 measure" << endl;
	}
	if (miss_counter > 0) {
		cout << " " << miss_counter << " nodes store a wrong number of measures" << endl;
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
		measures_register_.find(key)->second.hop_number_++;
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
//		cout << "Blacklist " << sensor_id_ << " added" << endl;
	} else {
		cout << "Error! Trying to remove a sensor twice!" << endl;
		exit(0);
	}
}

void DataCollector::record_bl(unsigned int cache_id, unsigned int sensor_id) {
	if (blacklist_register_.find(sensor_id) != blacklist_register_.end()) {	// if this blacklist is still in the register
		blacklist_register_.find(sensor_id)->second.hop_number_++;
		if (!blacklist_register_.find(sensor_id)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = blacklist_register_.find(sensor_id)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this blacklist
//				cout << "Cache " << cache_id << " saw bl" << sensor_id << endl;
				blacklist_register_.find(sensor_id)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = blacklist_register_.find(sensor_id)->second.node_map_.size();	// how many cache saw this blacklist
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this blacklist at least once
				blacklist_register_.find(sensor_id)->second.crossed_the_network_ = true;
				blacklist_register_.find(sensor_id)->second.spreading_time_ = MyToolbox::current_time_;
				blacklist_register_.find(sensor_id)->second.spreading_duration_ = blacklist_register_.find(sensor_id)->second.spreading_time_ - blacklist_register_.find(sensor_id)->second.born_time_;
//				cout << "Blacklist " << sensor_id << " crossed all the networks in " << blacklist_register_.find(sensor_id)->second.spreading_duration_ * 1. / 1000000000 << "s" << endl;
			}
		}
	} else {
		cout << "Error! Trying to record a blacklist not in the register!" << endl;
		exit(0);
	}
}

void DataCollector::erase_bl(unsigned int sensor_id) {
	if (blacklist_register_.find(sensor_id) != blacklist_register_.end()) {	// if this blacklist is still in the register
		blacklist_register_.find(sensor_id)->second.death_time_ = MyToolbox::current_time_;
		blacklist_register_.find(sensor_id)->second.travel_duration_ = blacklist_register_.find(sensor_id)->second.death_time_ - blacklist_register_.find(sensor_id)->second.born_time_;
//		cout << "Blacklist bl" << sensor_id << " was alive for " << blacklist_register_.find(sensor_id)->second.travel_duration_ * 1. / 1000000000 << "s" << endl;
	} else {	// I cannot find the blacklist
		cout << "Error! BLacklist bl" << sensor_id << " lost!" << endl;
	}
}

void DataCollector::record_user_movement(unsigned int user_id, double distance) {
	if (user_register_.find(user_id) == user_register_.end()) {	// if the user is not in the register
		UserInfo user_info;
		user_info.born_time_ = MyToolbox::current_time_;
		user_register_.insert(pair<unsigned int, UserInfo>(user_id, user_info));
//		cout << "user " << user_id << " added to register" << endl;
	} else {	// if the user is already in the register
//		cout << "Error! I'm trying to insert the same user twice!" << endl;
		user_register_.find(user_id)->second.covered_distance_ += distance;
		user_register_.find(user_id)->second.num_steps_++;
	}
}

void DataCollector::record_user_rx(unsigned int user_id) {
	if (user_register_.find(user_id) != user_register_.end()) {	// if the user is in the register
		user_register_.find(user_id)->second.num_rx_node_info_++;
	} else {	// if the user is not in the register
		cout << "Error! I'm trying to update a user not in the register!" << endl;
	}
}

void DataCollector::record_user_decoding(unsigned int user_id) {
	if (user_register_.find(user_id) != user_register_.end()) {	// if the user is in the register
		if (!user_register_.find(user_id)->second.decoded_) {	// first time this user decodes
			user_register_.find(user_id)->second.decoding_time_ = MyToolbox::current_time_;
			user_register_.find(user_id)->second.decoding_distance_ = user_register_.find(user_id)->second.covered_distance_;
			user_register_.find(user_id)->second.decoding_steps_ = user_register_.find(user_id)->second.num_steps_;
			user_register_.find(user_id)->second.decoding_duration_ = user_register_.find(user_id)->second.decoding_time_ - user_register_.find(user_id)->second.born_time_;
			user_register_.find(user_id)->second.decoded_ = true;
		} else {	// this user already decoded
			cout << "Error! This user already decoded!" << endl;
		}

	} else {	// if the user is not in the register
		cout << "Error! I'm trying to update a user not in the register!" << endl;
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
