#include "data_collector.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"

#include <iostream>
#include <fstream>

using namespace std;

DataCollector::DataCollector() {
}

void DataCollector::report() {

	cout << " > Report <" << endl;
	cout << "+ " << MyToolbox::num_storage_nodes_ << " storage nodes" << endl;
	cout << "+ " << MyToolbox::num_sensors_ << " sensors with " << MyToolbox::num_measures_for_sensor_ << " measures each" << endl;
	cout << "+ Intra-user communcation";
	  if (MyToolbox::intra_user_communication_) {
		  cout << " activated" << endl;
	  } else {
		  cout << " not activated" << endl;
	  }

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

	// Soliton Check
	cout << "+ SOLITON CHECK" << endl;
	int zero_counter = 0;
	int one_counter = 0;
	int miss_counter = 0;
	for (map<unsigned int, int>::iterator it = num_stored_measures_per_cache_.begin(); it != num_stored_measures_per_cache_.end(); it++) {
		int lt_deg = MyToolbox::storage_nodes_map_.find(it->first)->second.LT_degree_;
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
		cout << " " << zero_counter << " (" << zero_counter * 100.0 / MyToolbox::num_storage_nodes_ << "%) nodes store no measures" << endl;
	}
	if (one_counter > 0) {
		cout << " " << one_counter << " (" << one_counter * 100.0 / MyToolbox::num_storage_nodes_ << "%) nodes store 1 measure" << endl;
	}
	if (miss_counter > 0) {
		cout << " " << miss_counter << " (" << miss_counter * 100.0 / MyToolbox::num_storage_nodes_ << "%) nodes store a wrong number of measures" << endl;
	}
	cout << " " << num_stored_measures_per_cache_.size() - zero_counter - one_counter << " nodes store more than one measure" << endl;

	// Cache content
//	cout << "+ CACHE CONTENT" << endl;
//	for (map<unsigned int, StorageNode>::iterator node_it = MyToolbox::storage_nodes_map_.begin(); node_it != MyToolbox::storage_nodes_map_.end(); node_it++) {
//		StorageNode cache = node_it->second;
//		int measure = int(cache.xored_measure_);
//		map<unsigned int, unsigned int> msrs = cache.last_measures_;
//		cout << " - Node " << node_it->first << " stores " << measure << " from " << msrs.size() << " sensors: ";
//		for (map<unsigned int, unsigned int>::iterator msr_it = msrs.begin(); msr_it != msrs.end(); msr_it++) {
//			cout << " (s" << msr_it->first << ", " << msr_it->second << ")";
//		}
//		cout << endl;
//	}

	//User
	if (user_register_.size() > 0) {
		double avg_user_dec_time = 0;
		double avg_dec_num_steps = 0;
		double avg_dec_distance = 0;
		double avg_interr_caches = 0;
		double avg_interr_users = 0;
		int decoding_number = 0;
		int num_usrs = user_register_.size();
		for (map<unsigned int, UserInfo>::iterator it = user_register_.begin(); it != user_register_.end(); it++) {
			if (it->second.decoded_) {
				decoding_number++;
				avg_user_dec_time += it->second.decoding_duration_;
				avg_dec_num_steps += it->second.decoding_steps_;
				avg_dec_distance += it->second.decoding_distance_;
				avg_interr_caches += it->second.num_interrogated_caches_;
				avg_interr_users += it->second.num_interrogated_users_;
			}
		}
		avg_user_dec_time = avg_user_dec_time / decoding_number;
		avg_dec_num_steps = avg_dec_num_steps / decoding_number;
		avg_dec_distance = avg_dec_distance / decoding_number;
		avg_interr_caches = avg_interr_caches / decoding_number;
		avg_interr_users = avg_interr_users / decoding_number;

		cout << "+ USER" << endl;
		cout << " - Avg decoding time: " << avg_user_dec_time * 1. / pow(10, 9) << "s" << endl;
		cout << " - Avg decoding distance: " << avg_dec_distance << "m" << endl;
		cout << " - Avg decoding number of steps: " << avg_dec_num_steps << endl;
		cout << " - Avg number of interrogated caches: " << avg_interr_caches << " out of " << MyToolbox::num_storage_nodes_
						<< " (" << avg_interr_caches * 100.0 / MyToolbox::num_storage_nodes_ << "%)" << endl;
		cout << " - Avg number of interrogated users: " << avg_interr_users << " out of " << MyToolbox::num_users_
								<< " (" << avg_interr_users * 100.0 / MyToolbox::num_users_ << "%)" << endl;
		cout << " - Avg number of interrogated nodes: " << avg_interr_caches + avg_interr_users << " (minimum = " << MyToolbox::num_sensors_ << ")" << endl;
		cout << " - " << decoding_number << " users out of " << num_usrs << " (" << decoding_number * 100.0 / num_usrs << "%) decoded the messages" << endl;
		cout << " - Decoding errors:" << endl;
		for (map<unsigned int, UserInfo>::iterator user_it = user_register_.begin(); user_it != user_register_.end(); user_it++) {
			string s;
			bool errors = 0;
			map<MeasureKey, int> results = user_it->second.decoding_result_;
			for (auto& res : results) {
				if (res.second == 0) {
					errors++;
					s = s + " (s" + to_string(res.first.sensor_id_) + "," + to_string(res.first.measure_id_) + ")";
				}
			}
			if (errors > 0) {
				cout << "  " << s << endl;
			}
		}

//		cout << " - User list:" << endl;
//		for (map<unsigned int, UserInfo>::iterator user_it = user_register_.begin(); user_it != user_register_.end(); user_it++) {
//			unsigned int id = user_it->first;
//			double speed = user_it->second.speed_;
//			cout << "  User " << id << " speed " << speed << endl;
//		}

//		cout << " - User replacements:" << endl;
//		map<unsigned int, unsigned int> replacements_copy = user_replacement_register_;
//		vector<unsigned int> to_remove_entries;
//		bool stop = false;
//		while (!stop) {	// while there is still some users...
//			to_remove_entries.clear();	// clear the entries to remove
//			unsigned int replaced_user = replacements_copy.begin()->first;
//			to_remove_entries.push_back(replaced_user);
//			cout << "   - " << replaced_user;
//			while (replacements_copy.find(replaced_user) != replacements_copy.end()) {	// while this user has been replaced...
//				unsigned int replacing_user = replacements_copy.find(replaced_user)->second;	// get the user whoe repaced it
//				to_remove_entries.push_back(replacing_user);
//				cout << " -> " << replacing_user;	// print it
//				replaced_user = replacing_user;	// now let's see if this user has been replaced again
//			}
//			cout << endl;
//			for (vector<unsigned int>::iterator it = to_remove_entries.begin(); it != to_remove_entries.end(); it++) {	// for each entry to remove
//				replacements_copy.erase(*it);	// remove it
//			}
//			if (replacements_copy.empty()) {
//				stop = true;
//			} else {
//				stop = false;
//			}
//		}

//		ofstream myfile("user_replacement.txt", ios::app);
//		if (myfile.is_open()) {
//			map<unsigned int, unsigned int> replacements_copy = user_replacement_register_;
//			vector<unsigned int> to_remove_entries;
//			bool stop = false;
//			while (!stop) {	// while there is still some users...
//				to_remove_entries.clear();	// clear the entries to remove
//				unsigned int replaced_user = replacements_copy.begin()->first;
//				to_remove_entries.push_back(replaced_user);
//				myfile << "   - " << replaced_user;
//				while (replacements_copy.find(replaced_user) != replacements_copy.end()) {	// while this user has been replaced...
//					unsigned int replacing_user = replacements_copy.find(replaced_user)->second;	// get the user whoe repaced it
//					to_remove_entries.push_back(replacing_user);
//					myfile << " -> " << replacing_user;	// print it
//					replaced_user = replacing_user;	// now let's see if this user has been replaced again
//				}
//				myfile << endl;
//				for (vector<unsigned int>::iterator it = to_remove_entries.begin(); it != to_remove_entries.end(); it++) {	// for each entry to remove
//					replacements_copy.erase(*it);	// remove it
//				}
//				if (replacements_copy.empty()) {
//					stop = true;
//				} else {
//					stop = false;
//				}
//			}
//			myfile.close();
//		}
//		else cout << "Unable to open file";
	}
}

void DataCollector::update_num_msr_per_cache(unsigned int cache_id, int num_msrs) {
	if (num_stored_measures_per_cache_.find(cache_id) != num_stored_measures_per_cache_.end()) {	// cache in the map
		num_stored_measures_per_cache_.erase(cache_id);
	}
	num_stored_measures_per_cache_.insert(pair<unsigned int, int>(cache_id, num_msrs));
}

void DataCollector::add_msr(unsigned int msr_id, unsigned int sns_id, unsigned char data) {
	MeasureKey key(sns_id, msr_id);	// create the key
	MeasureInfo measureInfo;
	measureInfo.born_time_ = MyToolbox::current_time_;
	measures_register_.insert(pair<MeasureKey, MeasureInfo>(key, measureInfo));	// store this measure entry
	measure_data_register_.insert(pair<MeasureKey, unsigned char>(key, data));
	if (measure_data_all_register_.find(sns_id) == measure_data_all_register_.end()) {	// sensor not yet in the register
		map<unsigned int, unsigned char> msr_map;
		msr_map.insert(pair<unsigned int, unsigned char>(msr_id, data));
		measure_data_all_register_.insert(pair<unsigned int, map<unsigned int, unsigned char>>(sns_id, msr_map));
	} else {	// sensor already in the register
		pair<map<unsigned int, unsigned char>::iterator, bool> res_pair;
		res_pair = measure_data_all_register_.find(sns_id)->second.insert(pair<unsigned int, unsigned char>(msr_id, data));
		if (!res_pair.second) {	// check if the element is truly inserted
			cerr << "Data collector (add_msr): measure " << msr_id << " of sensor " << sns_id << " already in the register" << endl;
			exit(0);
		}
	}
}

void DataCollector::record_msr(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned int sym) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register_.find(key) != measures_register_.end()) {	// if this measure is in the register
		measures_register_.find(key)->second.hop_number_++;
		if (!measures_register_.find(key)->second.crossed_the_network_) {	// this measure didn't cross all the network yet
			map<unsigned int, int> node_map = measures_register_.find(key)->second.node_map_;	// get the map associated with this measure
			if (node_map.find(cache_id) == node_map.end()) {	// first time this node sees this measure
				measures_register_.find(key)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = measures_register_.find(key)->second.node_map_.size();	// how many caches saw this measure
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this measure at least once
				measures_register_.find(key)->second.crossed_the_network_ = true;
				measures_register_.find(key)->second.spreading_time_ = MyToolbox::current_time_;
				MyTime spr_duration = measures_register_.find(key)->second.spreading_time_ - measures_register_.find(key)->second.born_time_;
				measures_register_.find(key)->second.spreading_duration_ = spr_duration;

				ofstream msr_spreading_file("./../data_simulation_folder/measure_spreading.txt", ios::app);
				if (msr_spreading_file.is_open()) {
					msr_spreading_file << MyToolbox::current_time_ << ",(s" << sns_id << "-" << msr_id << ")," << spr_duration << endl;
					msr_spreading_file.close();
				}
			}
		}
	} else {
		cout << "Error! Trying to record a measure not in the register!" << endl;
	}
}

//void DataCollector::record_stored_measure(unsigned int msr_id, unsigned int sns_id, unsigned int cache_id, unsigned char received_data, unsigned char stored_data) {
//	if (all_stored_measures_register_.find(cache_id) == all_stored_measures_register_.end()) {	// cache not yet in the register
//		MeasureKey key(sns_id, msr_id);
//		pair<unsigned char, unsigned char> data(received_data, stored_data);
//		map<MeasureKey, pair<unsigned char, unsigned char>> msr_map;
//		pair<MeasureKey, pair<unsigned char, unsigned char>> data_entry(key, data);
//		msr_map.insert(data_entry);
//		pair<unsigned int, map<MeasureKey, pair<unsigned char, unsigned char>>> cache_entry(cache_id, msr_map);
//		all_stored_measures_register_.insert(cache_entry);
//	} else {	// cache already in the register
//		MeasureKey key(sns_id, msr_id);
//		pair<unsigned char, unsigned char> data(received_data, stored_data);
//		pair<MeasureKey, pair<unsigned char, unsigned char>> data_entry(key, data);
//		pair<map<MeasureKey, pair<unsigned char, unsigned char>>::iterator, bool> res_pair;
//		res_pair = all_stored_measures_register_.find(cache_id)->second.insert(data_entry);
//
//		if (!res_pair.second) {	// check if the element is truly inserted
//			cerr << "Data collector (record_stored_measure): measure " << msr_id << " of sensor " << sns_id << " already in the register for chace " << cache_id << endl;
//			exit(0);
//		}
//	}
//}

//void DataCollector::erase_stored_measure(std::vector<MeasureKey>, unsigned int cache_id, unsigned char stored_data) {
//	if (all_stored_measures_register_.find(cache_id) != all_stored_measures_register_.end()) {	// if the cache is in the register
//
//	} else {	// the cache is not in the register
//		cerr << "(DataCollector::erase_stored_measure): cache " << cache_id << " not in the register" << endl;
//	}
//}

void DataCollector::erase_msr(unsigned int msr_id, unsigned int sns_id) {
	MeasureKey key(sns_id, msr_id);	// key associated with this measure
	if (measures_register_.find(key) != measures_register_.end()) {	// if this measure is still in the register
		measures_register_.find(key)->second.death_time_ = MyToolbox::current_time_;
		measures_register_.find(key)->second.travel_duration_ = measures_register_.find(key)->second.death_time_ - measures_register_.find(key)->second.born_time_;
	} else {	// I cannot find the measure

	}
}

void DataCollector::register_broken_sensor(unsigned int sensor_id_) {
	if (blacklist_register_.find(sensor_id_) == blacklist_register_.end()) {	// if there is not such an entry
		BlacklistInfo bl_info;
		bl_info.born_time_ = MyToolbox::current_time_;
		blacklist_register_.insert(pair<unsigned int, BlacklistInfo>(sensor_id_, bl_info));
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
				blacklist_register_.find(sensor_id)->second.node_map_.insert(pair<unsigned int, int>(cache_id, 1));
			} else {	// this node already saw this measure
				// do nothing
			}
			int num_visits = blacklist_register_.find(sensor_id)->second.node_map_.size();	// how many cache saw this blacklist
			if (num_visits == MyToolbox::num_storage_nodes_) {	// if every cache saw this blacklist at least once
				blacklist_register_.find(sensor_id)->second.crossed_the_network_ = true;
				blacklist_register_.find(sensor_id)->second.spreading_time_ = MyToolbox::current_time_;
				blacklist_register_.find(sensor_id)->second.spreading_duration_ = blacklist_register_.find(sensor_id)->second.spreading_time_ - blacklist_register_.find(sensor_id)->second.born_time_;
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

void DataCollector::record_user_movement(unsigned int user_id, double distance, double speed) {
	if (user_register_.find(user_id) == user_register_.end()) {	// if the user is not in the register
		UserInfo user_info;
		user_info.born_time_ = MyToolbox::current_time_;
		user_info.speed_ = speed;
		user_register_.insert(pair<unsigned int, UserInfo>(user_id, user_info));
//		cout << "user " << user_id << " added to register" << endl;
	} else {	// if the user is already in the register
//		cout << "Error! I'm trying to insert the same user twice!" << endl;
		user_register_.find(user_id)->second.covered_distance_ += distance;
		user_register_.find(user_id)->second.num_steps_++;
		if (user_register_.find(user_id)->second.speed_ != speed) {
			cerr << "DataCollector::record_user_movement: speeds of user " << user_id << " do not coincide!" << endl;
			exit(0);
		}
	}
}

void DataCollector::record_user_rx(unsigned int user_id) {
	if (user_register_.find(user_id) != user_register_.end()) {	// if the user is in the register
		user_register_.find(user_id)->second.num_rx_node_info_++;
	} else {	// if the user is not in the register
		cout << "Error! I'm trying to update a user not in the register!" << endl;
	}
}

void DataCollector::record_user_decoding(unsigned int user_id, map<MeasureKey, unsigned char> decoded_symbols) {
	if (user_register_.find(user_id) != user_register_.end()) {	// if the user is in the register
		if (!user_register_.find(user_id)->second.decoded_) {	// first time this user decodes
			user_register_.find(user_id)->second.decoding_time_ = MyToolbox::current_time_;
			double dec_dist = user_register_.find(user_id)->second.covered_distance_;
			user_register_.find(user_id)->second.decoding_distance_ = dec_dist;
			user_register_.find(user_id)->second.decoding_steps_ = user_register_.find(user_id)->second.num_steps_;
			MyTime dec_duration = user_register_.find(user_id)->second.decoding_time_ - user_register_.find(user_id)->second.born_time_;
			user_register_.find(user_id)->second.decoding_duration_ = dec_duration;
			user_register_.find(user_id)->second.decoded_ = true;

			ofstream user_decoding_file("./../data_simulation_folder/user_decoding.txt", ios::app);
			if (user_decoding_file.is_open()) {
				user_decoding_file << MyToolbox::current_time_ << ",u" << user_id << "," << dec_duration << "," << dec_dist << endl;
				user_decoding_file.close();
			}

			// Check the correct decoding
			for (map<MeasureKey, unsigned char>::iterator symb_it = decoded_symbols.begin(); symb_it != decoded_symbols.end(); symb_it++) {	// for each decoded symbol
				MeasureKey this_key = symb_it->first;
				unsigned char my_data = symb_it->second;
				unsigned char actual_data = measure_data_register_.find(this_key)->second;
				if (my_data == actual_data) {
					user_register_.find(user_id)->second.decoding_result_.insert(pair<MeasureKey, int>(this_key, 1));
				} else {
					user_register_.find(user_id)->second.decoding_result_.insert(pair<MeasureKey, int>(this_key, 0));
				}
			}
		} else {	// this user already decoded
			cout << "Error! This user already decoded!" << endl;
		}

	} else {	// if the user is not in the register
		cout << "Error! I'm trying to update a user not in the register!" << endl;
	}
}

void DataCollector::record_user_query(unsigned int user_id, unsigned int queried_node_id, bool is_cache) {
	if (user_register_.find(user_id) != user_register_.end()) {	// if the user is in the register
		if (!user_register_.find(user_id)->second.decoded_) {	// first time this user decodes
			if (is_cache) {
				user_register_.find(user_id)->second.num_interrogated_caches_++;
			} else {	// is another user
				user_register_.find(user_id)->second.num_interrogated_users_++;
			}
		} else {	// this user already decoded
			cout << "Error! This user already decoded!" << endl;
		}

	} else {	// if the user is not in the register
		cout << "Error! I'm trying to update a user not in the register!" << endl;
	}
}

void DataCollector::record_user_replacement(unsigned int replaced, unsigned int replacing) {
	if (user_replacement_register_.find(replaced) == user_replacement_register_.end()) {	// replaced user not yet in the register
		user_replacement_register_.insert(pair<unsigned int, unsigned int>(replaced, replacing));
	} else {	// replaced user already in the register
		cerr << "DataCollector::record_user_replacement: user " << replaced << " already in the replacement register" << endl;
		exit(0);
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

bool DataCollector::check_measure_consistency(vector<MeasureKey> measure_keys, unsigned char stored_data) {
	unsigned char my_xor = 0;
	for (vector<MeasureKey>::iterator key_it = measure_keys.begin(); key_it != measure_keys.end(); key_it++) {
		unsigned int sns_id = key_it->sensor_id_;
		unsigned int msr_id = key_it->measure_id_;
		if (measure_data_all_register_.find(sns_id) == measure_data_all_register_.end()) {
			cerr << "DataCollector::check_measure_consistency - sensor " << sns_id << " not in the register" << endl;
			exit(0);
		}
		if (measure_data_all_register_.find(sns_id)->second.find(msr_id) == measure_data_all_register_.find(sns_id)->second.end()) {
			cerr << "DataCollector::check_measure_consistency - measure " << msr_id << " of sensor " << sns_id << " not in the register" << endl;
			exit(0);
		}
		unsigned char msr_data = measure_data_all_register_.find(sns_id)->second.find(msr_id)->second;
		my_xor ^= msr_data;
	}
	if (stored_data != my_xor) {
		return false;
	}
	return true;
}

bool DataCollector::check_user_decoding(map<MeasureKey, unsigned char> decoded_symbols) {
	for (map<MeasureKey, unsigned char>::iterator sym_it = decoded_symbols.begin(); sym_it != decoded_symbols.end(); sym_it++) {
		MeasureKey key = sym_it->first;
		unsigned int sns_id = key.sensor_id_;
		unsigned int msr_id = key.measure_id_;
		unsigned char user_data = sym_it->second;
		if (measure_data_all_register_.find(sns_id) == measure_data_all_register_.end()) {
			cerr << "DataCollector::check_user_decoding - sensor " << sns_id << " not in the register" << endl;
			exit(0);
		}
		if (measure_data_all_register_.find(sns_id)->second.find(msr_id) == measure_data_all_register_.find(sns_id)->second.end()) {
			cerr << "DataCollector::check_user_decoding - measure " << msr_id << " of sensor " << sns_id << " not in the register" << endl;
			exit(0);
		}
		unsigned char actual_data = measure_data_all_register_.find(sns_id)->second.find(msr_id)->second;
		if (user_data != actual_data) {
			return false;
		}
	}
	return true;
}
