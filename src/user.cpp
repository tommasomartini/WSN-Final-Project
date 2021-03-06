#include <iostream>
#include <fstream>
#include <math.h>
#include <random>
#include <algorithm>    // std::find, to check whether an element is in a vector

#include "user.h"
#include "sensor_node.h"
#include "storage_node.h"
#include "node_info_message.h"
#include "user_info_message.h"
#include "outdated_measure.h"

using namespace std;

#define PI 3.14159265

User::User() : Node() {
	speed_ = 0;
	direction_ = 0;
}

User::User(unsigned int node_id) : Node (node_id) {
	speed_ = 0;
	direction_ = 0;
}

User::User(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
	default_random_engine generator = MyToolbox::generator_;
	uniform_real_distribution<double> speed_distrib(0.5, 1.5);
	uniform_int_distribution<int> direction_distrib(0, 359);
	speed_ = speed_distrib(generator);
	direction_ = direction_distrib(generator);
}


/**************************************
    Public methods
 **************************************/

vector<Event> User::move() {
	vector<Event> new_events;

	if ((decoding_succeeded && event_queue_.empty()) || covered_distance_ > 500) {
//		cout << round(MyToolbox::current_time_ / pow(10, 9)) << " ------------------------------------------------------------------ u" << node_id_ << " replaced" << endl;
		return MyToolbox::replace_user(node_id_);
	}

	if (int(nodes_info_.size()) >= MyToolbox::num_storage_nodes_) {
//		cout << "n" << node_id_;
//		for (auto& aa : nodes_info_) {
//			cout << " -";
//			for (auto& bb : aa.second.sources_) {
//				unsigned int sns = bb.sensor_id_;
//				unsigned msr = bb.measure_id_;
//				cout << " (s" << sns << "," << msr << ")";
//			}
//			cout << endl;
//		}
//		exit(0);
//		cout << "u" << node_id_ << " failed. Ho " << nodes_info_.size() << " output symbols " << MyToolbox::current_time_ / (pow(10, 9) * 3600) << endl;
//		return MyToolbox::replace_user(node_id_);
	}

	vector<unsigned int> caches_to_reinsert;
	for (vector<unsigned int>::iterator cache_it = interrogated_caches_.begin(); cache_it != interrogated_caches_.end(); cache_it++) {	// for each cache I queried smoe data
		if (near_storage_nodes_.find(*cache_it) == near_storage_nodes_.end()) {		// this cache is no more my neighbor
			caches_to_reinsert.push_back(*cache_it);	// when it will be my neighbor again I can query him
		}
	}
	for (vector<unsigned int>::iterator cache_it = caches_to_reinsert.begin(); cache_it != caches_to_reinsert.end(); cache_it++) {	// for each cache I queried smoe data
		vector<unsigned int>::iterator to_remove_it = find(interrogated_caches_.begin(), interrogated_caches_.end(), *cache_it);
		interrogated_caches_.erase(to_remove_it);
	}

	default_random_engine generator = MyToolbox::generator_;
	uniform_int_distribution<int> distribution(-5, 5);  // I can have a deviation in the range -10°, +10°
	int deviation = distribution(generator);
	direction_ += deviation;	// change a bit my direction
	double dist = speed_ * (MyToolbox::user_observation_time_ * 1.0 / pow(10, 9));
	covered_distance_ += dist;

	double new_x = x_coord_ + dist * sin(direction_ * PI / 180) /* MyToolbox::space_precision_*/;	// compute my new position
	double new_y = y_coord_ + dist * cos(direction_ * PI / 180) /* MyToolbox::space_precision_*/;

	// I could also let the user go out! Just comment the following block of code!
	bool inside_area = false;
	while (!inside_area) {	// if I am going outside the area...
		if (new_x < 0 || new_x > MyToolbox::square_size_ || new_y < 0 || new_y > MyToolbox::square_size_) { // I am going out from the area
			direction_ += 30; // rotate of 30 degree in clockwise sense
			new_x = x_coord_ + dist * sin(direction_ * PI / 180) /* MyToolbox::space_precision_*/;
			new_y = y_coord_ + dist * cos(direction_ * PI / 180) /* MyToolbox::space_precision_*/;
		} else {
			inside_area = true;
		}
	}

	x_coord_ = new_x; // update the user's position
	y_coord_ = new_y;
	data_collector->record_user_movement(node_id_, dist, speed_);
	MyToolbox::set_close_nodes(this);   // set new storage nodes and users

	if (!decoding_succeeded) {	// not yet decoded all the input symbols
		for (map<unsigned int, StorageNode*>::iterator node_it = near_storage_nodes_.begin(); node_it != near_storage_nodes_.end(); node_it++) {
			if (find(interrogated_caches_.begin(), interrogated_caches_.end(), node_it->first) == interrogated_caches_.end()) {	// if I haven't already queried this node
				MyTime event_time = MyToolbox::current_time_ + MyToolbox::get_tx_offset();
				Event hello_event(event_time, Event::event_type_cache_gets_user_hello);
				Message* empty_msg = new Message();
				empty_msg->message_type_ = Message::message_type_user_hello;
				empty_msg->set_receiver_node_id(node_it->first);
				empty_msg->set_sender_node_id(node_id_);
				hello_event.set_agent(node_it->second);
				hello_event.set_agent_id(node_it->first);
				hello_event.set_message(empty_msg);
				new_events.push_back(hello_event);
				interrogated_caches_.push_back(node_it->first);
				data_collector->record_user_query(node_id_, node_it->first, true);
			}
		}

		if (MyToolbox::intra_user_communication_) {
			for (map<unsigned int, User*>::iterator user_it = near_users_.begin(); user_it != near_users_.end(); user_it++) {
				if (find(interrogated_users_.begin(), interrogated_users_.end(), user_it->first) == interrogated_users_.end()) {	// if I haven't already queried this node
//					bernoulli_distribution distribution(0.2);	// interrogate another user with probability 1 / 5
//					if (distribution(MyToolbox::generator_)) {
					if (MyToolbox::users_map_.find(user_it->first)->second.decoding_succeeded == false) {
						MyTime event_time = MyToolbox::current_time_ + MyToolbox::get_tx_offset(); //* 100 ;
						Event hello_event(event_time, Event::event_type_user_gets_user_hello);
						Message* empty_msg = new Message();
						empty_msg->message_type_ = Message::message_type_user_hello;
						empty_msg->set_receiver_node_id(user_it->first);
						empty_msg->set_sender_node_id(node_id_);
						hello_event.set_message(empty_msg);
						hello_event.set_agent(user_it->second);
						hello_event.set_agent_id(user_it->first);
						new_events.push_back(hello_event);
						interrogated_users_.push_back(user_it->first);
						data_collector->record_user_req_user(node_id_);
					}
					//					}
				}
			}
		}
	}

	MyTime event_time = MyToolbox::current_time_ + MyToolbox::user_observation_time_;
	Event new_event(event_time, Event::event_type_user_moves);	// set the new move_user event
	new_event.set_agent(this);
	new_event.set_agent_id(node_id_);
	new_events.push_back(new_event);

	return new_events;
}

vector<Event> User::receive_node_data(NodeInfoMessage* node_info_msg) {
	vector<Event> new_events;

	if (decoding_succeeded) {	// I already decoded the measure and I am just doing some of the operations AFTER decoding
		delete node_info_msg;
		return new_events;
	}

	for (vector<unsigned int>::iterator dead_sns_it = node_info_msg->dead_sensors_.begin(); dead_sns_it != node_info_msg->dead_sensors_.end(); dead_sns_it++) {	// for each dead sensor id in this node info message
		if (find(dead_sensors_.begin(), dead_sensors_.end(), *dead_sns_it) == dead_sensors_.end()) {	// if I don't have this id in my blacklist...
			dead_sensors_.push_back(*dead_sns_it);	// ...add it!
		}
	}

	if (nodes_info_.find(node_info_msg->node_id_) == nodes_info_.end()) {	// if it is the first time I get a message from this cache
		data_collector->record_user_rx(node_id_);	// record it to the data collector
	}

	OutputSymbol curr_output_symbol_(MyToolbox::current_time_, node_info_msg->output_message_, node_info_msg->sources_, node_info_msg->outdated_measures_);	// create a new output symbol
	pair<unsigned int, OutputSymbol> new_output_symbol(node_info_msg->node_id_, curr_output_symbol_);	// associate it to the cache
	map<unsigned int, OutputSymbol>::iterator info_it = nodes_info_.find(node_info_msg->node_id_);	// the entry belonging to this cache in my map
	if (info_it != nodes_info_.end()) {	// if there is already an entry related to this cache...
		nodes_info_.erase(info_it);	// ...remove it!
	}
	nodes_info_.insert(new_output_symbol);	// insert the info from the cache into the map

	// Update the measure id
	for (vector<MeasureKey>::iterator msr_key_it_ = node_info_msg->sources_.begin(); msr_key_it_ != node_info_msg->sources_.end(); msr_key_it_++) {	// for each of the measure just received...
		unsigned int current_sns_id = (*msr_key_it_).sensor_id_;	// id of the sensor which generated this measure
		unsigned int current_msr_id = (*msr_key_it_).measure_id_;	// id of the measure
		if (updated_sensors_measures_.find(current_sns_id) == updated_sensors_measures_.end()) {	// never received a measure from this sensor
			updated_sensors_measures_.insert(pair<unsigned int, unsigned int>(current_sns_id, current_msr_id));	// add the new sensor and the relative measure
		} else {	// already received a measure from this sensor -> update it if the new measure is newer
			unsigned int current_updated_msr_id = updated_sensors_measures_.find(current_sns_id)->second;	// the most updated measure I have from this sensor
			if (current_msr_id > current_updated_msr_id) {	// the just received measure is more recent than the one I had
				updated_sensors_measures_.find(current_sns_id)->second = current_msr_id;	// replace the old measure with the new one, just received
			}
		}
	}

//	cout << "@@u" << node_id_ << " rx from c" << node_info_msg->get_sender_node_id() << endl;

	// I have stored all the info brought by this node info msg, I don't need it anymore, I can release it
	delete node_info_msg;

	if (int(nodes_info_.size()) < MyToolbox::num_sensors_) {	// if I have less output symbols than input it's not possible to complete message passing...
		return new_events;	// ...return and do not do anything more: I have to wait for other output symbols	// FIXME activate this
	}

	bool msg_passing_ok = message_passing();
	if (msg_passing_ok && data_collector->check_user_decoding(decoded_symbols_)) {	// message passing succeeded: I have decoded all the symbols
//		cout << "u" << node_id_ << "__" << endl;
		decoding_succeeded = true;	// from now on do not accept other caches' answers
		data_collector->record_user_decoding(node_id_, decoded_symbols_);
		if (MyToolbox::backward_communication_) {
			for (map<unsigned int, OutputSymbol>::iterator out_sym_it = nodes_info_.begin(); out_sym_it != nodes_info_.end(); out_sym_it++) {	// for each cache which answered me...
				if (near_storage_nodes_.find(out_sym_it->first) != near_storage_nodes_.end()) {	// if this cache is among my neighbours...
//					unsigned char ad_hoc_msg = 0;	// ...xored message I have to send him...
//					int xor_counter = 0;	// ...how many measure I did xor
					map<unsigned int, unsigned char> replacements;	// data to replace
					vector<MeasureKey> outdated_tmp = out_sym_it->second.outdated_;
					map<unsigned int, vector<unsigned int>> update_info;
					for (vector<MeasureKey>::iterator out_it = outdated_tmp.begin(); out_it != outdated_tmp.end(); out_it++) {	// for each (pure) measure the cache needs
						MeasureKey outdated_msr_key = *out_it;	// key of the outdated measure the cache wants to erase
						unsigned int outdated_sns_id = out_it->sensor_id_;	// sensor id of this outdated measure
						unsigned int outdated_msr_id = out_it->measure_id_;	// measure id of this outdated measure
						bool have_decoded_this_measure = decoded_symbols_.find(outdated_msr_key) != decoded_symbols_.end();
						bool measure_of_dead_sensor = find(dead_sensors_.begin(), dead_sensors_.end(), outdated_sns_id) != dead_sensors_.end();
						bool have_newer_msr_for_this_sns = false;
						unsigned int newest_msr_for_this_sns = updated_sensors_measures_.find(outdated_sns_id)->second;
						if (newest_msr_for_this_sns > outdated_msr_id) {
							have_newer_msr_for_this_sns = true;
						}
						/*
						 * When I do the message passing, could I decode only some of the measure I collected? Don't know... Just in case I state that the
						 * message passing is ok only if the most recent measures I have have been decoded!
						 * Then I could  not have the "pure" measures a cache is looking for. Better to check it!
						 */
						if (have_decoded_this_measure) {	// if I have decoded this measure
							if (measure_of_dead_sensor) {	// if this node is dead I want just to remove this measure
								unsigned char out_data = decoded_symbols_.find(*out_it)->second;	// get the data he wants
//								ad_hoc_msg ^= out_data;	// xor only the data of the outdated measure, in this way I am erasing the outdated measure
//								xor_counter++;	// count how many measure I am xoring
								replacements.insert(pair<unsigned int, unsigned char>(outdated_sns_id, out_data));
								vector<unsigned int> inner_vec;
								inner_vec.push_back(outdated_msr_id);	// store the id of the measure to remove
								inner_vec.push_back(0);		// store 0 because I don't want to replace this measure
								inner_vec.push_back(0);		// store a random number, I won't use this value
								update_info.insert(pair<unsigned int, vector<unsigned int>>(outdated_sns_id, inner_vec));	// add this entry to the map
							} else if (have_newer_msr_for_this_sns) { // otherwise, if this sensor is not dead and I have a newer measure I want to update it
								/*
								 * Scenario: the cache A has (s10, 1), but receives (s10, 3). There is (s10, 2) missing.
								 * Cahce A passes me (s10, 1), but tells me this is an outdated measure. Anyway I only receive measure 1 of sensor 10,
								 * then (s10, 1) is the most update measure I know for sensor 10.
								 * It does not make any sense to pass cache A again MY most updated measure, because it is equal to his!
								 * Moreover, it does not neither make any sense to erase this measure from the cache! Better leave that another user,
								 * who decoded (s10, 3), updates cache A.
								 */
								unsigned char out_data = decoded_symbols_.find(*out_it)->second;	// get the data he wants
//								ad_hoc_msg ^= out_data;	// xor only the data of the outdated measure, in this way I am erasing the outdated measure
//								xor_counter++;	// count how many measure I am xoring
								MeasureKey key(outdated_sns_id, newest_msr_for_this_sns);	// create a key
								unsigned char up_data = decoded_symbols_.find(key)->second;		// get the data related to the most update measure
//								ad_hoc_msg ^= up_data;	// xor the updated data so that I now have an updated measure

								unsigned char replacement_data = out_data ^ up_data;	// xor old and new measure;
								replacements.insert(pair<unsigned int, unsigned char>(outdated_sns_id, replacement_data));

								vector<unsigned int> inner_vec;
								inner_vec.push_back(outdated_msr_id);	// store the id of the measure to remove
								inner_vec.push_back(1);		// store 1 because I want to replace this measure
								inner_vec.push_back(newest_msr_for_this_sns);		// store the id of the newest measure I have for this sensor
								update_info.insert(pair<unsigned int, vector<unsigned int>>(outdated_sns_id, inner_vec));	// add this entry to the map
							} else {	// sensor is not dead, but I don't have a newer measure
								// do nothing
							}
						} else {	// I have NOT decoded this measure

						}
					}
					if (replacements.size() > 0) {	// if I have at least one measure to remove or update
						OutdatedMeasure* outdated_measure = new OutdatedMeasure(replacements, update_info);	// create an outdated measure message
						vector<Event> curr_events = send(out_sym_it->first, outdated_measure);		// send it
						for (vector<Event>::iterator event_it = curr_events.begin(); event_it != curr_events.end(); event_it++) {	// add the returned events to the list new_events
							new_events.push_back(*event_it);
						}
					}
				} else {	// cache not in my neighborhood

				}
			}
		}
	} else {	// message passing failed: symbols not decoded...
		// do nothing and wait to try message passing again...
	}

	return new_events;
}

vector<Event> User::receive_user_data(UserInfoMessage* user_info_msg) {
	vector<Event> new_events;

	data_collector->record_user_rx_from_user(node_id_);

	if (!MyToolbox::intra_user_communication_) {
		cerr << "Error! Received a message from a user when intra-user communication is not active!" << endl;
		exit(0);
	}

	if (decoding_succeeded) {	// if I already made the decoding...
		delete user_info_msg;
		return new_events;	// ...ignore this message
	}

	for (vector<unsigned int>::iterator dead_sns_it = user_info_msg->dead_sensors_.begin(); dead_sns_it != user_info_msg->dead_sensors_.end(); dead_sns_it++) {	// for each dead sensor id in this node info message
		if (find(dead_sensors_.begin(), dead_sensors_.end(), *dead_sns_it) == dead_sensors_.end()) {	// if I don't have this id in my blacklist...
			dead_sensors_.push_back(*dead_sns_it);	// ...add it!
		}
	}

//	cout << "##u" << node_id_ << " rx from u" << user_info_msg->get_sender_node_id() << ". Have " << nodes_info_.size() << " msrs" << endl;
	for (map<unsigned int, OutputSymbol>::iterator sym_it = user_info_msg->symbols_.begin(); sym_it != user_info_msg->symbols_.end(); sym_it++) {	// for each symbol passed by the other user
		OutputSymbol out_sym = sym_it->second;	// symbol in exam
		map<unsigned int, OutputSymbol>::iterator info_it = nodes_info_.find(sym_it->first);	// the entry belonging to this cache in my map
		if (info_it == nodes_info_.end()) {	// if there is NOT an entry related to this cache (keep it anyway)

			nodes_info_.insert(pair<unsigned int, OutputSymbol>(sym_it->first, out_sym));	// ...insert it into the map

//			cout << " insert symbol from cache c" << sym_it->first;
//			for (auto& aa : out_sym.sources_) {
//				unsigned int sns = aa.sensor_id_;
//				unsigned msr = aa.measure_id_;
//				cout << " (s" << sns << "," << msr << ")";
//			}
//			cout << endl;

			// Update the measure id
			for (vector<MeasureKey>::iterator msr_key_it_ = out_sym.sources_.begin(); msr_key_it_ != out_sym.sources_.end(); msr_key_it_++) {	// for each of the measure just received...
				unsigned int current_sns_id = (*msr_key_it_).sensor_id_;	// id of the sensor which generated this measure
				unsigned int current_msr_id = (*msr_key_it_).measure_id_;	// id of the measure
				if (updated_sensors_measures_.find(current_sns_id) == updated_sensors_measures_.end()) {	// never received a measure from this sensor
					updated_sensors_measures_.insert(pair<unsigned int, unsigned int>(current_sns_id, current_msr_id));	// add the new sensor and the relative measure
				} else {	// already received a measure from this sensor -> update it if the new measure is newer
					unsigned int current_updated_msr_id = updated_sensors_measures_.find(current_sns_id)->second;	// the most updated measure I have from this sensor
					if (current_msr_id > current_updated_msr_id) {	// the just received measure is more recent than the one I had
						updated_sensors_measures_.find(current_sns_id)->second = current_msr_id;	// replace the old measure with the new one, just received
					}
				}
			}
		} else {	// if there already is an entry related to this cache (keep it only if updated)
			MyTime his_sym_time = out_sym.generation_time_;		// time this symbol has been generated
			MyTime my_sym_time = info_it->second.generation_time_;		// time my symbol has been generated
			if (his_sym_time > my_sym_time) {	// if his measure is more recent keep it
				nodes_info_.erase(info_it->first);
				nodes_info_.insert(pair<unsigned int, OutputSymbol>(sym_it->first, out_sym));	// ...insert it into the map

				// Update the measure id
				for (vector<MeasureKey>::iterator msr_key_it_ = out_sym.sources_.begin(); msr_key_it_ != out_sym.sources_.end(); msr_key_it_++) {	// for each of the measure just received...
					unsigned int current_sns_id = (*msr_key_it_).sensor_id_;	// id of the sensor which generated this measure
					unsigned int current_msr_id = (*msr_key_it_).measure_id_;	// id of the measure
					if (updated_sensors_measures_.find(current_sns_id) == updated_sensors_measures_.end()) {	// never received a measure from this sensor
						updated_sensors_measures_.insert(pair<unsigned int, unsigned int>(current_sns_id, current_msr_id));	// add the new sensor and the relative measure
					} else {	// already received a measure from this sensor -> update it if the new measure is newer
						unsigned int current_updated_msr_id = updated_sensors_measures_.find(current_sns_id)->second;	// the most updated measure I have from this sensor
						if (current_msr_id > current_updated_msr_id) {	// the just received measure is more recent than the one I had
							updated_sensors_measures_.find(current_sns_id)->second = current_msr_id;	// replace the old measure with the new one, just received
						}
					}
				}
			}
		}
	}
//	cout << "now have " << nodes_info_.size() << " msrs" << endl;

	// I have stored all the info brought by this node info msg, I don't need it anymore, I can release it
	delete user_info_msg;

	if (int(nodes_info_.size()) < MyToolbox::num_sensors_) {	// if I have less output symbols than input it's not possible to complete message passing...
		return new_events;	// ...return and do not do anything more: I have to wait for other output symbols
	}

	bool msg_passing_ok = message_passing();
	if (msg_passing_ok && data_collector->check_user_decoding(decoded_symbols_)) {	// message passing succeeded: I have decoded all the symbols

//		cout << "u" << node_id_ << "__" << endl;

		decoding_succeeded = true;	// from now on do not accept other caches' answers
		data_collector->record_user_decoding(node_id_, decoded_symbols_);

		if (MyToolbox::backward_communication_) {
			for (map<unsigned int, OutputSymbol>::iterator out_sym_it = nodes_info_.begin(); out_sym_it != nodes_info_.end(); out_sym_it++) {	// for each cache which answered me...
				if (near_storage_nodes_.find(out_sym_it->first) != near_storage_nodes_.end()) {	// if this cache is among my neighbors...
//					unsigned char ad_hoc_msg = 0;	// ...xored message I have to send him...
//					int xor_counter = 0;	// ...how many measure I did xor
					map<unsigned int, unsigned char> replacements;	// data to replace

					vector<MeasureKey> outdated_tmp = out_sym_it->second.outdated_;
					map<unsigned int, vector<unsigned int>> update_info;
					for (vector<MeasureKey>::iterator out_it = outdated_tmp.begin(); out_it != outdated_tmp.end(); out_it++) {	// for each (pure) measure the cache needs
						MeasureKey outdated_msr_key = *out_it;	// key of the outdated measure the cache wants to erase
						unsigned int outdated_sns_id = out_it->sensor_id_;	// sensor id of this outdated measure
						unsigned int outdated_msr_id = out_it->measure_id_;	// measure id of this outdated measure
						bool have_decoded_this_measure = decoded_symbols_.find(outdated_msr_key) != decoded_symbols_.end();
						bool measure_of_dead_sensor = find(dead_sensors_.begin(), dead_sensors_.end(), outdated_sns_id) != dead_sensors_.end();
						bool have_newer_msr_for_this_sns = false;
						unsigned int newest_msr_for_this_sns = updated_sensors_measures_.find(outdated_sns_id)->second;
						if (newest_msr_for_this_sns > outdated_msr_id) {
							have_newer_msr_for_this_sns = true;
						}
						/*
						 * When I do the message passing, could I decode only some of the measure I collected? Don't know... Just in case I state that the
						 * message passing is ok only if the most recent measures I have have been decoded!
						 * Then I could  not have the "pure" measures a cache is looking for. Better to check it!
						 */
						if (have_decoded_this_measure) {	// if I have decoded this measure
							if (measure_of_dead_sensor) {	// if this node is dead I want just to remove this measure
								unsigned char out_data = decoded_symbols_.find(*out_it)->second;	// get the data he wants
//								ad_hoc_msg ^= out_data;	// xor only the data of the outdated measure, in this way I am erasing the outdated measure
//								xor_counter++;	// count how many measure I am xoring

								replacements.insert(pair<unsigned int, unsigned char>(outdated_sns_id, out_data));

								vector<unsigned int> inner_vec;
								inner_vec.push_back(outdated_msr_id);	// store the id of the measure to remove
								inner_vec.push_back(0);		// store 0 because I don't want to replace this measure
								inner_vec.push_back(0);		// store a random number, I won't use this value
								update_info.insert(pair<unsigned int, vector<unsigned int>>(outdated_sns_id, inner_vec));	// add this entry to the map
							} else if (have_newer_msr_for_this_sns) { // otherwise, if this sensor is not dead and I have a newer measure I want to update it
								/*
								 * Scenario: the cache A has (s10, 1), but receives (s10, 3). There is (s10, 2) missing.
								 * Cache A passes me (s10, 1), but tells me this is an outdated measure. Anyway I only receive measure 1 of sensor 10,
								 * then (s10, 1) is the most update measure I know for sensor 10.
								 * It does not make any sense to pass cache A again MY most updated measure, because it is equal to his!
								 * Moreover, it does not neither make any sense to erase this measure from the cache! Better leave that another user,
								 * who decoded (s10, 3), updates cache A.
								 */
								unsigned char out_data = decoded_symbols_.find(*out_it)->second;	// get the data he wants
//								ad_hoc_msg ^= out_data;	// xor only the data of the outdated measure, in this way I am erasing the outdated measure
//								xor_counter++;	// count how many measure I am xoring
								MeasureKey key(outdated_sns_id, newest_msr_for_this_sns);	// create a key
								unsigned char up_data = decoded_symbols_.find(key)->second;		// get the data related to the most update measure
//								ad_hoc_msg ^= up_data;	// xor the updated data so that I now have an updated measure

								unsigned char replacement_data = out_data ^ up_data;	// xor old and new measure;
								replacements.insert(pair<unsigned int, unsigned char>(outdated_sns_id, replacement_data));

								vector<unsigned int> inner_vec;
								inner_vec.push_back(outdated_msr_id);	// store the id of the measure to remove
								inner_vec.push_back(1);		// store 1 because I want to replace this measure
								inner_vec.push_back(newest_msr_for_this_sns);		// store the id of the newest measure I have for this sensor
								update_info.insert(pair<unsigned int, vector<unsigned int>>(outdated_sns_id, inner_vec));	// add this entry to the map
							} else {	// sensor is not dead, but I don't have a newer measure
								// do nothing
							}
						} else {	// I have NOT decoded this measure
						}
					}
					if (replacements.size() > 0) {	// if I have at least one measure to remove or update for this cache
						OutdatedMeasure* outdated_measure = new OutdatedMeasure(replacements, update_info);	// create an outdated measure message
						vector<Event> curr_events = send(out_sym_it->first, outdated_measure);		// send it
						for (vector<Event>::iterator event_it = curr_events.begin(); event_it != curr_events.end(); event_it++) {	// add the returned events to the list new_events
							new_events.push_back(*event_it);
						}
					}
//					if (xor_counter > 0) {	// if I have at least one measure to remove or update for this cache
//						OutdatedMeasure* outdated_measure = new OutdatedMeasure(ad_hoc_msg, update_info);	// create an outdated measure message
//						vector<Event> curr_events = send(out_sym_it->first, outdated_measure);		// send it
//						for (vector<Event>::iterator event_it = curr_events.begin(); event_it != curr_events.end(); event_it++) {	// add the returned events to the list new_events
//							new_events.push_back(*event_it);
//						}
//					}
				} else {	// cache not in my neighborhood
					// do nothing
				}
			}
		}
	} else {	// message passing failed: symbols not decoded...
		// do nothing and wait to try message passing again...
	}

	return new_events;
}

vector<Event> User::try_retx(Message* message) {
	vector<Event> new_events;
	new_events = re_send(message);
	return new_events;
}

/*  This user receives a "beep" from another user, asking him to send him his measures
 */
vector<Event> User::receive_user_request(unsigned int next_user_id) {
	vector<Event> new_events;
	if (!decoding_succeeded) {
		if (!MyToolbox::intra_user_communication_) {
			cerr << "Error! Received a request from a user when intra-user communication is not active!" << endl;
			exit(0);
		}
		if (near_users_.find(next_user_id) != near_users_.end()) {	// this user is still among my neighbors
			if (int(nodes_info_.size()) > 0) {	// if I have something to pass
				int M = MyToolbox::max_num_measures_;
				if (int(nodes_info_.size()) <= M) {	// I have M measures or less
					UserInfoMessage* user_info_msg = new UserInfoMessage(nodes_info_, dead_sensors_);	// send all
					new_events = send(next_user_id, user_info_msg);
				} else {	// I have more than M measures
					map<unsigned int, OutputSymbol> symbols_to_send;	// map of the symbols I have to send
					map<unsigned int, OutputSymbol>::iterator my_it = nodes_info_.begin();	// iterator pointing to the first element
					int steps = rand() % (nodes_info_.size() - M);	// take a random starting point between 0 and (last - M)
					for (int i = 0; i < steps; i++) {	// take the iterator to the starting point
						my_it++;
					}
					for (int i = 0; i < M; i++) {	// for each of the following M symbols
						symbols_to_send.insert(pair<unsigned int, OutputSymbol>(my_it->first, my_it->second));	// insert it in the list
						my_it++;	// increment the iterator
					}
					UserInfoMessage* user_info_msg = new UserInfoMessage(symbols_to_send, dead_sensors_);
					new_events = send(next_user_id, user_info_msg);		// send it
				}
			}
		}
	}
	return new_events;
}

/**************************************
    Private methods
 **************************************/

bool User::message_passing() {
	bool message_passing_succeeded = true;

	// Create a copy of the node info to work with
	map<unsigned int, OutputSymbol> info = nodes_info_;

	map<MeasureKey, unsigned char> resolved_symbols;
	map<MeasureKey, unsigned char> released_symbols;
	vector<unsigned int> links_to_remove;
	while (true) {
		released_symbols.clear();   // released symbols are different at each step
		links_to_remove.clear();	// and so are the links I have to remove when I release an output symbol
		for (map<unsigned int, OutputSymbol>::iterator info_it = info.begin(); info_it != info.end(); info_it++) {	// scan the output symbols...
			OutputSymbol curr_output_symbol = info_it->second;
			if (curr_output_symbol.sources_.size() == 1) {  // ...to find the ones with output degree 1
				// store the resolved symbol
				resolved_symbols.insert(pair<MeasureKey, unsigned char>(*(curr_output_symbol.sources_.begin()), curr_output_symbol.xored_msg_));
				released_symbols.insert(pair<MeasureKey, unsigned char>(*(curr_output_symbol.sources_.begin()), curr_output_symbol.xored_msg_));
				links_to_remove.push_back(info_it->first);
				// Note that using a map if I try to insert two pairs with the same key, the second insertion does not succeed. In this way
				// I am guaranteed to insert only one entry for each sns id in the lists and not to have duplicated symbols.
			} else if (curr_output_symbol.sources_.empty()) {  // no more links to this node
				links_to_remove.push_back(info_it->first); // if an output symbol does not have any output link anymore it is useless, let's remove it!
			}
		}

		// Erase from the (copy of the) info the row relative to the removed links. I don't need that symbol anymore
		for (vector<unsigned int>::iterator links_it = links_to_remove.begin(); links_it != links_to_remove.end(); links_it++) {
			info.erase(*links_it);
		}

		// Now we have to XOR the just released output symbols with the output symbols containing them
		if (released_symbols.size() > 0) {  // at least one symbol has been released
			for(map<MeasureKey, unsigned char>::iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // for each released symbol...
				MeasureKey current_msr_key = released_iterator->first;	// id of the sensor who produced the released symbol and id of the measure
				unsigned char current_value = released_iterator->second;	// xored measure of this sensor
				// for each output symbol (use an iterator to use a pointer and modify the actual values!):
				for (map<unsigned int, OutputSymbol>::iterator out_sym_it = info.begin(); out_sym_it != info.end(); out_sym_it++) {	// ...for each output symbol not yet released (still in the copied buffer)...
					vector<MeasureKey>::iterator key_vec_it = find(out_sym_it->second.sources_.begin(), out_sym_it->second.sources_.end(), current_msr_key);	// ...look for this key in its key vector
					if (key_vec_it != out_sym_it->second.sources_.end()) {	// if its xored message is also formed by this measure...
						unsigned char new_xor = current_value ^ out_sym_it->second.xored_msg_;  // ...XOR the released symbol with the current output one...
						out_sym_it->second.xored_msg_ = new_xor; // ...replace it...
						out_sym_it->second.sources_.erase(key_vec_it);	// ...and remove the link
					}
				}
			}
		} else {  // no symbol released at this round
			if (info.empty()) {  // if no other symbols
				for (map<unsigned int, unsigned int>::iterator up_it = updated_sensors_measures_.begin(); up_it != updated_sensors_measures_.end(); up_it++) {
					MeasureKey key(up_it->first, up_it->second);
					if (resolved_symbols.find(key) == resolved_symbols.end()) {		// key not present among the decoded symbols
						message_passing_succeeded = false;
						break;
					}
				}
				decoded_symbols_ = resolved_symbols;	// store the decoded measures
				break;
			} else {
				message_passing_succeeded = false;
				break;
			}
		}
	}
	return message_passing_succeeded;
}

bool User::CRC_check(Message message) {
	return true;
}

// this method is only for the first send!
vector<Event> User::send(unsigned int next_node_id, Message* message) {
	vector<Event> new_events;

	// Set sender and receiver
	message->set_receiver_node_id(next_node_id);
	message->set_sender_node_id(node_id_);

	if (!event_queue_.empty()) {  // already some pending event -> does not generate new eventsk
//		cout << " coda piena" << endl;
		Event event_to_enqueue(0, Event::event_type_user_re_send);	// execution time does not matter now...
		event_to_enqueue.set_agent(this);
		event_to_enqueue.set_agent_id(node_id_);
		event_to_enqueue.set_message(message);
		event_queue_.push(event_to_enqueue);
	} else {  // no pending events
//		cout << " coda vuota" << endl;
		map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
		MyTime current_time = MyToolbox::current_time_;  // current time of the system
		MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
		MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
		if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
//			cout << " io occupato" << endl;
			MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::event_type_user_re_send);
			try_again_event.set_agent(this);
			try_again_event.set_agent_id(node_id_);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//			cout << " lui occupato" << endl;
			MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::event_type_user_re_send);
			try_again_event.set_agent(this);
			try_again_event.set_agent_id(node_id_);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else {  // sender and receiver both idle, can send the message
//			cout << " tutti liberi" << endl;
			// Compute the message time
			MyTime processing_time = MyToolbox::get_random_processing_time();
			unsigned int num_total_bits = message->get_message_size();
			MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
			MyTime new_schedule_time = current_time + processing_time + transfer_time;
			// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
			// Just in case I want to give priority to some particular message...
			Event::EventTypes this_event_type;
			Agent* my_agent;
			switch (message->message_type_) {
			case Message::message_type_user_info_for_cache: {
				this_event_type = Event::event_type_cache_receives_user_info;
				my_agent = near_storage_nodes_.find(next_node_id)->second;
				break;
			}
			case Message::message_type_user_info_for_user: {
				this_event_type = Event::event_type_user_receives_user_data;
				my_agent = near_users_.find(next_node_id)->second;
				break;
			}
			default:
				break;
			}
			Event receive_message_event(new_schedule_time, this_event_type);
			receive_message_event.set_agent(my_agent);
			receive_message_event.set_agent_id(next_node_id);
			receive_message_event.set_message(message);
			new_events.push_back(receive_message_event);

			// Update the timetable
			MyToolbox::timetable_.find(node_id_)->second = new_schedule_time; // update my available time
			for (map<unsigned int, SensorNode*>::iterator sns_it = near_sensors_.begin(); sns_it != near_sensors_.end(); sns_it++) {
				MyToolbox::timetable_.find(sns_it->first)->second = new_schedule_time;
			}
			for (map<unsigned int, StorageNode*>::iterator cache_it = near_storage_nodes_.begin(); cache_it != near_storage_nodes_.end(); cache_it++) {
				MyToolbox::timetable_.find(cache_it->first)->second = new_schedule_time;
			}
			for (map<unsigned int, User*>::iterator user_it = near_users_.begin(); user_it != near_users_.end(); user_it++) {
				MyToolbox::timetable_.find(user_it->first)->second = new_schedule_time;
			}

			// If I am here the queue was empty and it is still empty! I have to do nothing on the queue!
		}
	}
	return new_events;
}

vector<Event> User::re_send(Message* message) {
	vector<Event> new_events;

	unsigned int next_node_id = message->get_receiver_node_id();
	if (near_storage_nodes_.find(next_node_id) == near_storage_nodes_.end() && near_users_.find(next_node_id) == near_users_.end()) {	// my neighbor there is no longer
//		cout << next_node_id << " non è più mio visino" << endl;
		event_queue_.pop();	// remove this event from the queue, I'm not going to execute it anymore
		if (event_queue_.empty()) {	// if the queue is now empty...
//			cout << " coda vuota" << endl;
			return new_events;	// return an empty vector, I don't have new events to schedule
		} else {
			Message* new_message = event_queue_.front().get_message();
//			cout << " nuovo messaggio di tipo " << new_message->message_type_ << endl;
			return re_send(new_message);
		}
	}

	// If I arrive here I have a neighbour to whom I can try to send

	map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
	MyTime current_time = MyToolbox::current_time_;  // current time of the system
	MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
	MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
	if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
//		cout << " io occupato" << endl;
		MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::event_type_user_re_send);
		try_again_event.set_agent(this);
		try_again_event.set_agent_id(node_id_);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//		cout << " lui occupato" << endl;
		MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::event_type_user_re_send);
		try_again_event.set_agent(this);
		try_again_event.set_agent_id(node_id_);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else {  // sender and receiver both idle, can send the message
//		cout << " tutti liberi" << endl;
		// Compute the message time
		MyTime processing_time = MyToolbox::get_random_processing_time();
		unsigned int num_total_bits = message->get_message_size();
		MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
		MyTime new_schedule_time = current_time + processing_time + transfer_time;
		// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
		Event::EventTypes this_event_type;
		Agent* my_agent;
		switch (message->message_type_) {
		case Message::message_type_user_info_for_cache: {
			this_event_type = Event::event_type_cache_receives_user_info;
			my_agent = near_storage_nodes_.find(next_node_id)->second;
//			cout << " msg type remove measure a cache " << next_node_id << endl;
			break;
		}
		case Message::message_type_user_info_for_user: {
			this_event_type = Event::event_type_user_receives_user_data;
			my_agent = near_users_.find(next_node_id)->second;
//			cout << " msg type intra a user " << next_node_id << endl;
			break;
		}
		default:
			cout << " msg type other" << endl;
			break;
		}
		Event receive_message_event(new_schedule_time, this_event_type);
		receive_message_event.set_agent(my_agent);
		receive_message_event.set_agent_id(next_node_id);
		receive_message_event.set_message(message);
		new_events.push_back(receive_message_event);

		// Update the timetable
		MyToolbox::timetable_.find(node_id_)->second = new_schedule_time; // update my available time
		for (map<unsigned int, SensorNode*>::iterator sns_it = near_sensors_.begin(); sns_it != near_sensors_.end(); sns_it++) {
			MyToolbox::timetable_.find(sns_it->first)->second = new_schedule_time;
		}
		for (map<unsigned int, StorageNode*>::iterator cache_it = near_storage_nodes_.begin(); cache_it != near_storage_nodes_.end(); cache_it++) {
			MyToolbox::timetable_.find(cache_it->first)->second = new_schedule_time;
		}
		for (map<unsigned int, User*>::iterator user_it = near_users_.begin(); user_it != near_users_.end(); user_it++) {
			MyToolbox::timetable_.find(user_it->first)->second = new_schedule_time;
		}

		// Update the event_queue_
		event_queue_.pop();	// remove the current send event, now successful
		if (!event_queue_.empty()) {  // if there are other events in the queue
			// I will be available, in the best case scenario, after new_schedule_time
			MyTime sched_time = new_schedule_time + MyToolbox::get_tx_offset();
			Event next_send_event(sched_time, event_queue_.front().get_event_type());
			next_send_event.set_agent(this);
			next_send_event.set_agent_id(node_id_);
			next_send_event.set_message(event_queue_.front().get_message());
			new_events.push_back(next_send_event); // schedule the next event
		}
		return new_events;
	}
}
