#include <iostream>
#include <fstream>
#include <algorithm>    // std::max
#include <iostream>
#include <math.h>
#include <random>

#include "storage_node.h"
#include "my_toolbox.h"
#include "sensor_node.h"
#include "user.h"
#include "measure.h"
#include "blacklist_message.h"
#include "node_info_message.h"
#include "outdated_measure.h"

using namespace std;

StorageNode::StorageNode() : Node () {
	LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();
	xored_measure_ = 0;
	set_measure_indeces();
}

StorageNode::StorageNode(unsigned int node_id) : Node (node_id) {
	LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();
	xored_measure_ = 0;
	set_measure_indeces();
}

StorageNode::StorageNode(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
	if (MyToolbox::num_sensors_ < 3) {
		LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();
	} else {
		LT_degree_ = MyToolbox::get_robust_soliton_distribution_degree();
		if (LT_degree_ > MyToolbox::num_sensors_ || LT_degree_< 0) {
			cerr << "Error with LT_degrees_: " << LT_degree_ <<endl;
		}
	}
	xored_measure_ = 0;
	set_measure_indeces();
}


/**************************************
    Event execution methods
 **************************************/
/*  Receive a measure either from a sensor or from another cache node
 */
vector<Event> StorageNode::receive_measure(Measure* measure) {
	vector<Event> new_events;
	data_collector->record_msr(measure->measure_id_, measure->source_sensor_id_, node_id_, 1);
	unsigned int source_id = measure->source_sensor_id_;  // measure from sensor source_id
	if (measure->get_measure_type() == Measure::measure_type_new) { // new measure from a new sensor
		if (find(ignore_new_list.begin(), ignore_new_list.end(), measure->source_sensor_id_) == ignore_new_list.end()) {	// ignore all the measures of the sensors in the ignore list
			ignore_new_list.push_back(source_id);	// I should process a new measure for each sensor just once
			indeces_counter_++;
			if (indeces_pointer_ < int(indeces_msr_to_keep_.size()) && indeces_counter_ == indeces_msr_to_keep_.at(indeces_pointer_)) {	// I have to keep this measure
				indeces_pointer_++;
				xored_measure_ = xored_measure_ ^ measure->measure_;  // save the new xored message
				SensorInfo sensor_info;
				sensor_info.alive_ = true;
				sensor_info.following_ = true;
				sensor_info.sensor_id_ = source_id;
				sensor_info.most_recent_key_ = MeasureKey(source_id, measure->measure_id_);
				stored_measures_.insert(pair<unsigned int, SensorInfo>(source_id, sensor_info));
				data_collector->update_num_msr_per_cache(node_id_, stored_measures_.size());

				if (!check_consistency()) {
					cerr << "Cache " << node_id_ << " has no consistent xor" << endl;
					exit(0);
				}

//				if (stored_measures_.size() % 2 == 0 && int(xored_measure_) != 0) {
//					cerr << "Node "<< node_id_ << " rx new. SM size = " << stored_measures_.size() << ", xor = " << int(xored_measure_) << endl;
//				}
//				if (stored_measures_.size() % 2 == 1 && int(xored_measure_) == 0) {
//					cerr << "Node "<< node_id_ << " rx new. SM size = " << stored_measures_.size() << ", xor = " << int(xored_measure_) << endl;
//				}
			}
		}
	} else if (measure->get_measure_type() == Measure::measure_type_update) { // update measure from a sensor: always accept it, if I'm collecting this sensor's measures
		if (stored_measures_.find(source_id) != stored_measures_.end()) {	// I'm following this sensor
			if (stored_measures_.find(source_id)->second.following_) {  // and I am synchronized with it
				unsigned int last_measure_id = stored_measures_.find(source_id)->second.most_recent_key_.measure_id_;
				// if the received measure is one unit higher than the stored one everything is ok, otherwise I missed a measure and I have to re-initialize the system
				if (measure->measure_id_ == last_measure_id + 1) {	// actual new measure for me and received in order: I have to update
					xored_measure_ = xored_measure_ ^ measure->measure_;  // save the new xored message
					MeasureKey new_key(source_id, measure->measure_id_);
					stored_measures_.find(source_id)->second.most_recent_key_ = new_key;
					data_collector->update_num_msr_per_cache(node_id_, stored_measures_.size());

					if (!check_consistency()) {
						cerr << "Cache " << node_id_ << " has no consistent xor" << endl;
						exit(0);
					}

//					if (stored_measures_.size() % 2 == 0 && int(xored_measure_) != 0) {
//						cerr << "Node "<< node_id_ << " rx up. SM size = " << stored_measures_.size() << ", xor = " << int(xored_measure_) << endl;
//					}
//					if (stored_measures_.size() % 2 == 1 && int(xored_measure_) == 0) {
//						cerr << "Node "<< node_id_ << " rx up. SM size = " << stored_measures_.size() << ", xor = " << int(xored_measure_) << endl;
//					}
					//				cout << "Cache " << node_id_ << ": " << int(xored_measure_) << endl;
				} else if (measure->measure_id_ > last_measure_id + 1) {	// out of order update measure
					stored_measures_.find(source_id)->second.following_ = false;
					if (last_msr_without_backward_.find(source_id) == last_msr_without_backward_.end()) {	// not yet this sensor
						// If I didn't have the backward, last_msr_id would be the last msr id from this sensor
						last_msr_without_backward_.insert(pair<unsigned int, unsigned int>(source_id, last_measure_id));
					}
				}
			}
		}
	}

	measure->increase_hop_counter();	// increase the hop-counter
	int hop_limit = MyToolbox::max_num_hops_;
	if (measure->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
		new_events = send(get_random_neighbor(), measure);	// propagate it!
	} else {	// the message must not be forwarded again
		data_collector->erase_msr(measure->measure_id_, measure->source_sensor_id_);
		delete measure;	// this measure will be no more used
	}

	return new_events;
}

/*  I have already tried to send a message to someone, but I failed. Now I try again!
 */
vector<Event> StorageNode::try_retx(Message* message) {
	vector<Event> new_events;
	new_events = re_send(message);
	return new_events;
}

/*  A sensor is telling me it is alive
 */
void StorageNode::receive_ping(unsigned int sensor_id) {
//	cout << "Cache " << node_id_ << " gets pings from " << sensor_id << " time: " << MyToolbox::current_time_ << endl;
	// If it is the first time I receive a ping from a sensor it means that that sensor wants me to be his supervisor. I save it in my supervisor map
	if (supervised_map_.find(sensor_id) == supervised_map_.end()) {	// first time a sensor contacts me
		supervised_map_.insert(std::pair<unsigned int, MyTime>(sensor_id, MyToolbox::current_time_));
//		cout << "Node " << node_id_ << ": FIRST ping from " << sensor_id << endl;
//		cout << " size supervisioned map:" << supervised_map_.size() << endl;
	} else {	// this sensor already has contacted me
		supervised_map_.find(sensor_id)->second = MyToolbox::current_time_;
//		cout << "Node " << node_id_ << ": ANOtHER ping from " << sensor_id << endl;
	}
}

/*  A user asks me to send him my data
 */
vector<Event> StorageNode::receive_user_request(unsigned int sender_user_id) {
	vector<Event> new_events;
	if (near_users_.find(sender_user_id) != near_users_.end()) {	// if this user is still among my neighbors
		vector<MeasureKey> sources;
		vector<MeasureKey> outdated_measures;
		vector<unsigned int> dead_sensors;
		for (map<unsigned int, SensorInfo>::iterator info_it = stored_measures_.begin(); info_it != stored_measures_.end(); info_it++) {
			SensorInfo sns_info = info_it->second;
			sources.push_back(sns_info.most_recent_key_);
			if (!sns_info.following_) {
//				cout << "cache" << node_id_ << " inserito sensore " << sns_info.sensor_id_ << " in blacklist" << endl;
				outdated_measures.push_back(sns_info.most_recent_key_);
				if (!sns_info.alive_) {
					dead_sensors.push_back(info_it->first);
				}
			}
		}
		NodeInfoMessage* node_info_msg = new NodeInfoMessage(node_id_, xored_measure_, sources, outdated_measures, dead_sensors);
		new_events = send(sender_user_id, node_info_msg);
	}
	return new_events;
}

/*  Occasionally I check if the sensors I am supervising are OK
 */
vector<Event> StorageNode::check_sensors() {
	vector<Event> new_events;
	if (keep_checking_sensors_) {
		vector<unsigned int> expired_sensors;	// list of sensor ids which didn't answer for 3 times in a row
		for (auto& x : supervised_map_){	// for each sensor in my supervised list...
			unsigned int curr_sns_id = x.first;
			if(x.second + (3 * MyToolbox::ping_frequency_) < MyToolbox::current_time_){  // ...if it didn't answer for 3 times...
				expired_sensors.push_back(curr_sns_id);	// ...and in a list I use to update the structures
				if (stored_measures_.find(curr_sns_id) != stored_measures_.end()) {	// if I'm storing a measure from this sensor
					stored_measures_.find(curr_sns_id)->second.alive_ = false;
					stored_measures_.find(curr_sns_id)->second.following_ = false;
				}
				data_collector->register_broken_sensor(x.first);
				MyToolbox::remove_sensor(x.first);
			}
		}
		// remove from the supervisioned_map the dead sensors
		for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) {
			supervised_map_.erase(*it);
		}

		if (expired_sensors.size() > 0) {	// if there are some expired sensors I have to spread this info
			BlacklistMessage* list = new BlacklistMessage(expired_sensors);
			list->message_type_= Message::message_type_blacklist;
			new_events = send(get_random_neighbor(), list);
		}

		//	ping_check_counter_++;
		//	if (ping_check_counter_ < num_ping_checks_) {
		Event new_event(MyToolbox::current_time_ + MyToolbox::check_sensors_frequency_, Event::event_type_cache_checks_sensors);
		new_event.set_agent(this);
		new_event.set_agent_id(node_id_);
		new_events.push_back(new_event);
		//	}
	}
	return new_events;
}

/*  I received a blacklist message: this message contains the measure I have to remove
 */
vector<Event> StorageNode::spread_blacklist(BlacklistMessage* list) {
	vector<Event> new_events;
	for (vector<unsigned int>::iterator sns_it = list->sensor_ids_.begin(); sns_it != list->sensor_ids_.end(); sns_it++) {
		data_collector->record_bl(node_id_, *sns_it);
	}

	vector<unsigned int> expired_sensors = list->sensor_ids_;
	for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) { 	// for each id in the blacklist...
		if (stored_measures_.find(*it) != stored_measures_.end()) {	// if I have this sensor
			stored_measures_.find(*it)->second.alive_ = false;
		}
	}

	list->increase_hop_counter();
	int hop_limit = MyToolbox::max_num_hops_;
	if (list->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
		unsigned int next_node_index = get_random_neighbor();
		list->message_type_ = Message::message_type_blacklist;
		new_events = send(next_node_index, list);
	} else {
		for (vector<unsigned int>::iterator sns_it = list->sensor_ids_.begin(); sns_it != list->sensor_ids_.end(); sns_it++) {
			data_collector->erase_bl(*sns_it);
		}
		delete list;
	}
	return new_events;
}

void StorageNode::refresh_xored_data3(OutdatedMeasure* refresh_message){
	map<unsigned int, unsigned char> replacements = refresh_message->replacements_;	// download the data replacements
 	map<unsigned int, vector<unsigned int>> update_info = refresh_message->update_infos_;	// download the info structure

 	if (!MyToolbox::backward_communication_) {
 		cerr << "Error! Received a message from a user when backward communication is not active!" << endl;
 		exit(0);
 	}

 	cout << "Cache " << node_id_ << endl;
 	for (auto& inf : stored_measures_) {
 		unsigned int sns_id = inf.second.sensor_id_;
 		cout << " - s" << sns_id << " msr " << inf.second.most_recent_key_.measure_id_;
 		if (!inf.second.alive_) {
 			cout << " dead" << endl;
 		} else if (!inf.second.following_) {
 			cout << " outdated" << endl;
 		} else {
 			cout << " ok" << endl;
 		}
 	}

 	cout << " updating" << endl;
 	int num_updated_data = 0;
	map<unsigned int, vector<unsigned int>>::iterator info_it = update_info.begin();	// extract the iterator
	for (; info_it != update_info.end(); info_it++) {	// for each sensor of the structure
		bool can_update = true;	// can I update the info relative to this sensor?
		cout << "  sensor " << info_it->first << endl;
		if (stored_measures_.find(info_it->first) == stored_measures_.end()) {	// this sensor is not composing my xor
			cout << "   no this sensor in my stored measures" << endl;
			can_update = false;
		}
		unsigned int my_measure_id = stored_measures_.find(info_it->first)->second.most_recent_key_.measure_id_;	// measure id I have from this sensor
		unsigned int to_remove_measure_id = info_it->second.at(0);	// first field of the vector: measure id I am trying to remove
		cout << "  trying to remove msr" << to_remove_measure_id << endl;
		if (my_measure_id != to_remove_measure_id) {	// not the measure composing my xor
			cout << "   don't have this measure" << endl;
			can_update = false;
		}
		if (stored_measures_.find(info_it->first)->second.following_) {	// I am following this sensor. It is ok, no need to remove the measure
			cout << "   following this sensor" << endl;
			can_update = false;
		}
		if (info_it->second.at(1) == 0) {	// I want to erase this measure without replacing (the sensor should be dead)
			cout << "   just erase" << endl;
			if (stored_measures_.find(info_it->first)->second.alive_) {	// this sensor is alive
				cout << "    sensor alive" << endl;
				can_update = false;
			}
		} else if (info_it->second.at(1) == 1) {	// I want to replace this measure
			cout << "   replace" << endl;
			if (!stored_measures_.find(info_it->first)->second.alive_) {	// this sensor is not alive anymore. Makes no sense to replace its measure
				cout << "    sensor dead" << endl;
				can_update = false;
			}
			unsigned int new_measure_id = info_it->second.at(2);	// id of the new measure I am inserting
			cout << "   replace with " << new_measure_id << endl;
			if (new_measure_id < my_measure_id) {	// I am trying to insert an older measure
				cout << "    not a more recent measure" << endl;
				can_update = false;
			}
		}

		if (can_update) {	// if I can perform the xor while preserving consistency
			cout << "  can update" << endl;

			num_updated_data++;

			unsigned char replacement = replacements.find(info_it->first)->second;	// the data I have to xor to perform the replacement
			xored_measure_ ^= replacement;
			bool replaced = info_it->second.at(1) == 1;	// did I removed or replaced this measure?
			if (!replaced) {	// I just removed it
				stored_measures_.erase(info_it->first);		// erase the measure info from my list
			} else if (replaced) {	// I replaced this measure
				unsigned int new_measure_id = info_it->second.at(2);	// take the id of the just inserted measure
				stored_measures_.find(info_it->first)->second.following_ = true;
				stored_measures_.find(info_it->first)->second.most_recent_key_.measure_id_ = new_measure_id;	// replace the measure id in the most recent key
			}
			data_collector->update_num_msr_per_cache(node_id_, stored_measures_.size());
		}
	}

	cout << "c" << node_id_ << " updated " << num_updated_data << " measures out of " << replacements.size() << endl;

	if (!check_consistency()) {
		cerr << "Cache " << node_id_ << " has no consistent xor" << endl;
		exit(0);
	}
//	cout << " end method" << endl;

	delete refresh_message;
}

/*  A user informs me about what measures are obsolete
 */
void StorageNode::refresh_xored_data2(OutdatedMeasure* refresh_message){
 	map<unsigned int, vector<unsigned int>> update_info = refresh_message->update_infos_;	// download the info structure

 	if (!MyToolbox::backward_communication_) {
 		cerr << "Error! Received a message from a user when backward communication is not active!" << endl;
 		exit(0);
 	}

	map<unsigned int, vector<unsigned int>>::iterator info_it = update_info.begin();	// extract the iterator
 	bool can_update = true;
	for (; info_it != update_info.end(); info_it++) {	// for each sensor of the structure
		if (stored_measures_.find(info_it->first) == stored_measures_.end()) {	// this sensor is not composing my xor
			can_update = false;
			break;
		}
		unsigned int my_measure_id = stored_measures_.find(info_it->first)->second.most_recent_key_.measure_id_;	// measure id I have from this sensor
		unsigned int to_remove_measure_id = info_it->second.at(0);	// first field of the vector: measure id I am trying to remove
		if (my_measure_id != to_remove_measure_id) {	// not the measure composing my xor
			can_update = false;
			break;
		}
		if (stored_measures_.find(info_it->first)->second.following_) {	// I am following this sensor. It is ok, no need to remove the measure
			can_update = false;
			break;
		}
		if (info_it->second.at(1) == 0) {	// I want to erase this measure without replacing
			if (stored_measures_.find(info_it->first)->second.alive_) {	// this sensor is alive
				can_update = false;
				break;
			}
		} else if (info_it->second.at(1) == 1) {	// I want to replace this measure
			if (!stored_measures_.find(info_it->first)->second.alive_) {	// this sensor is not alive anymore. Makes no sense to replace its measure
				can_update = false;
				break;
			}
			unsigned int new_measure_id = info_it->second.at(2);	// id of the new measure I am inserting
			if (new_measure_id < my_measure_id) {	// I am trying to insert an older measure
				can_update = false;
				break;
			}
		}
	}

	if (can_update) {	// if I can perform the xor while preserving consistency
		xored_measure_ ^= refresh_message->xored_data_;
		for (info_it = update_info.begin(); info_it != update_info.end(); info_it++) {	// for each sensor of the structure
			bool replaced = info_it->second.at(1) == 1;	// did I removed or replaced this measure?
			if (!replaced) {	// I just removed it
				stored_measures_.erase(info_it->first);		// erase the measure info from my list
			} else if (replaced) {	// I replaced this measure
				unsigned int new_measure_id = info_it->second.at(2);	// take the id of the just inserted measure
				stored_measures_.find(info_it->first)->second.following_ = true;
				stored_measures_.find(info_it->first)->second.most_recent_key_.measure_id_ = new_measure_id;	// replace the measure id in the most recent key
			}
		}
		data_collector->update_num_msr_per_cache(node_id_, stored_measures_.size());
	}

	if (!check_consistency()) {
		cerr << "Cache " << node_id_ << " has no consistent xor" << endl;
		exit(0);
	}
//	cout << " end method" << endl;

	delete refresh_message;
}

map<unsigned int, unsigned int> StorageNode::get_most_recent_measures() {
	map<unsigned int, unsigned int> my_most_recent_measures;
	for (map<unsigned int, SensorInfo>::iterator info_it = stored_measures_.begin(); info_it != stored_measures_.end(); info_it++) {	// for each stored mesaure
		unsigned int sns_id = info_it->second.most_recent_key_.sensor_id_;
		unsigned int msr_id = info_it->second.most_recent_key_.measure_id_;
		my_most_recent_measures.insert(pair<unsigned int, unsigned int>(sns_id, msr_id));
	}
	return my_most_recent_measures;
}

///*  A user informs me about what measures are obsolete
// */
//void StorageNode::refresh_xored_data(OutdatedMeasure* refresh_message){
//	cout << "Cache " << node_id_ << " rx rfresh message" << endl;
//
//	cout << "Ref message removed: " << endl;
//	for (auto& elem : refresh_message->removed_) {
//		cout << " (s" << elem.sensor_id_ << "," << elem.measure_id_ << ")";
//	}
//	cout << endl;
//	cout << "Ref message inserted: " << endl;
//	for (auto& elem : refresh_message->inserted_) {
//		cout << " (s" << elem.sensor_id_ << "," << elem.measure_id_ << ")";
//	}
//	cout << endl;
//
//
//	cout << "Cache " << node_id_ << " BEFORE stores: " << int(xored_measure_) << endl;
//	cout << " Last:" << endl;
//	for (map<unsigned int, unsigned int>::iterator last_it = last_measures_.begin(); last_it != last_measures_.end(); last_it++) {
//		cout << " - (s" << last_it->first << "," << last_it->second << ")" << endl;
//	}
//	cout << " Outdated:" << endl;
//	for (vector<MeasureKey>::iterator outdated_it = outdated_measure_keys_.begin(); outdated_it != outdated_measure_keys_.end(); outdated_it++) {
//		cout << " - (s" << outdated_it->sensor_id_ << "," << outdated_it->measure_id_ << ")";
//		if (find(my_blacklist_.begin(), my_blacklist_.end(), outdated_it->sensor_id_) != my_blacklist_.end()) {
//			cout << " -> dead sensor";
//		}
//		cout << endl;
//	}
//
//	vector<MeasureKey> removed = refresh_message->removed_;		// list of the measure I have removed because outdated
//	vector<MeasureKey> inserted = refresh_message->inserted_;		// list of the measure I have inserted
//	bool can_update = true;
//	for (vector<MeasureKey>::iterator rem_it = removed.begin(); rem_it != removed.end(); rem_it++) {	// for each measure I could erase
//		if (find(outdated_measure_keys_.begin(), outdated_measure_keys_.end(), *rem_it) == outdated_measure_keys_.end()) {	// if it is NOT among the measure I have to erase
//			can_update = false;
//			break;
//		}
//	}
//	for (vector<MeasureKey>::iterator ins_it = inserted.begin(); ins_it != inserted.end(); ins_it++) {	// for each measure I could add
//		if (find(my_blacklist_.begin(), my_blacklist_.end(), ins_it->sensor_id_) == my_blacklist_.end()) {	// if this sensor is NOT in my blacklist I cannot erase this measure
//			can_update = false;
//			break;
//		}
//	}
//	if (can_update) {	// if I can perform the xor while preserving consistency
//		xored_measure_ ^= refresh_message->xored_data_;
//		for (vector<MeasureKey>::iterator rem_it = removed.begin(); rem_it != removed.end(); rem_it++) {	// for each measure I have erased
//			outdated_measure_keys_.erase(find(outdated_measure_keys_.begin(), outdated_measure_keys_.end(), *rem_it));	// remove it from my outdated list: these data are no more outdated
//		}
//		for (vector<MeasureKey>::iterator ins_it = inserted.begin(); ins_it != inserted.end(); ins_it++) {	// for each measure I have added
//			last_measures_.insert(pair<unsigned int, unsigned int>(ins_it->sensor_id_, ins_it->measure_id_));	// add this measure to the list of the measure I have in my xor
//		}
//	}
//
//	cout << "Cache " << node_id_ << " AFTER stores: " << int(xored_measure_) << endl;
//	cout << " Last:" << endl;
//	for (map<unsigned int, unsigned int>::iterator last_it = last_measures_.begin(); last_it != last_measures_.end(); last_it++) {
//		cout << " - (s" << last_it->first << "," << last_it->second << ")" << endl;
//	}
//	cout << " Outdated:" << endl;
//	for (vector<MeasureKey>::iterator outdated_it = outdated_measure_keys_.begin(); outdated_it != outdated_measure_keys_.end(); outdated_it++) {
//		cout << " - (s" << outdated_it->sensor_id_ << "," << outdated_it->measure_id_ << ")";
//		if (find(my_blacklist_.begin(), my_blacklist_.end(), outdated_it->sensor_id_) != my_blacklist_.end()) {
//			cout << " -> dead sensor";
//		}
//		cout << endl;
//	}
//
//	delete refresh_message;
//}

/**************************************
    Private methods
 **************************************/

// this method is only for the first send!
vector<Event> StorageNode::send(unsigned int next_node_id, Message* message) {
	vector<Event> new_events;

	// Set sender and receiver
	message->set_receiver_node_id(next_node_id);
	message->set_sender_node_id(node_id_);

	if (!event_queue_.empty()) {  // already some pending event -> does not generate new events
//		cout << " queue not empty" << endl;
		Event event_to_enqueue(0, Event::event_type_cache_re_send);	// execution time does not matter now...
		event_to_enqueue.set_agent(this);
		event_to_enqueue.set_agent_id(node_id_);
		event_to_enqueue.set_message(message);
		event_queue_.push(event_to_enqueue);
	} else {  // no pending events
		map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
		MyTime current_time = MyToolbox::current_time_;  // current time of the system
		MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
		MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
		if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
//			cout << " me not free" << endl;
			MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
			try_again_event.set_agent(this);
			try_again_event.set_agent_id(node_id_);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//			cout << " him not free" << endl;
			MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
			try_again_event.set_agent(this);
			try_again_event.set_agent_id(node_id_);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else {  // sender and receiver both idle, can send the message
			// Compute the message time
			MyTime processing_time = MyToolbox::get_random_processing_time();
			unsigned int num_total_bits = message->get_message_size();
			MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
			MyTime new_schedule_time = current_time + processing_time + transfer_time;
			// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
			// Just in case I want to give priority to some particular message...
			Event::EventTypes this_event_type;
			Agent* agent;
			switch (message->message_type_) {
			case Message::message_type_measure: {
				this_event_type = Event::event_type_cache_receives_measure;
				agent = near_storage_nodes_.find(next_node_id)->second;
				break;
			}
			case Message::message_type_blacklist: {
				this_event_type = Event::event_type_cache_gets_blacklist;
				agent = near_storage_nodes_.find(next_node_id)->second;
				break;
			}
			case Message::message_type_cache_info_for_user: {
				this_event_type = Event::event_type_user_receives_node_data;
//				agent = &(MyToolbox::users_map_.find(next_node_id)->second);
				agent = near_users_.find(next_node_id)->second;
				break;
			}
			default:
				break;
			}
			Event receive_message_event(new_schedule_time, this_event_type);
			receive_message_event.set_agent(agent);
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

vector<Event> StorageNode::re_send(Message* message) {
	vector<Event> new_events;

	unsigned int next_node_id = message->get_receiver_node_id();
//	cout << "Node " << node_id_ << " resend a " << next_node_id << endl;
	if (near_storage_nodes_.find(next_node_id) == near_storage_nodes_.end() && near_users_.find(next_node_id) == near_users_.end()) {	// my neighbor there is no longer
//		cout << " " << next_node_id << " non più vicino" << endl;
		bool give_up = false;	// I could gie up transmitting: it depends on the message type!
		switch (message->message_type_) {
		case Message::message_type_measure: {
			give_up = false;
			break;
		}
		case Message::message_type_blacklist: {
			give_up = false;
			break;
		}
		case Message::message_type_cache_info_for_user: {
			give_up = true;
			break;
		}
		default:
			give_up = true;
			break;
		}
		if (give_up) {	// do not try to tx this message, pass to the following one
//			cout << " give up!" << endl;
			event_queue_.pop();	// remove this event from the queue, I'm not going to execute it anymore
			if (event_queue_.empty()) {	// if the queue is now empty...
				return new_events;	// return an empty vector, I don't have new events to schedule
			} else {
				Message* new_message = event_queue_.front().get_message();
				return re_send(new_message);
			}
		} else {	// I cannot give up! Find another node to spread the message
//			cout << " no NOT give up!" << endl;
			next_node_id = get_random_neighbor();	// find another neighbour. Only cache!
			if (next_node_id == 0) {	// no more neighbours, I'm isolated. Postpone my delivery
				MyTime schedule_time = MyToolbox::get_random_processing_time() + MyToolbox::get_tx_offset();
				event_queue_.front().set_time(schedule_time);	// change the schedule time of the message I am trying to send

				Event try_again_event(schedule_time, Event::event_type_cache_re_send);
				try_again_event.set_agent(this);
				try_again_event.set_agent_id(node_id_);
				try_again_event.set_message(message);
				// FIXME: uncomment the following line to make the node try to send undefinitely. If the line is commented, if the node is left alone it does not generate new events ever
//				new_events.push_back(try_again_event);
				return new_events;	// return, there's no more I can do!
			}
			// if next_node_id is a valid id...
			message->set_receiver_node_id(next_node_id);	// set the new node id and go on
		}
	}

	// If I arrive here I have a neighbour to whom I can try to send

	map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
	MyTime current_time = MyToolbox::current_time_;  // current time of the system
	MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
	MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
	if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
//		cout << " me not free!" << endl;
		MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
		try_again_event.set_agent(this);
		try_again_event.set_agent_id(node_id_);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//		cout << " him not free!" << endl;
		MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
		try_again_event.set_agent(this);
		try_again_event.set_agent_id(node_id_);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else {  // sender and receiver both idle, can send the message
//		cout << " everybody free!" << endl;
		// Compute the message time
		MyTime processing_time = MyToolbox::get_random_processing_time();
		unsigned int num_total_bits = message->get_message_size();
		MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
		MyTime new_schedule_time = current_time + processing_time + transfer_time;
		// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
		Event::EventTypes this_event_type;
		Agent* agent;
		switch (message->message_type_) {
		case Message::message_type_measure: {
			this_event_type = Event::event_type_cache_receives_measure;
			agent = near_storage_nodes_.find(next_node_id)->second;
			break;
		}
		case Message::message_type_blacklist: {
			this_event_type = Event::event_type_cache_gets_blacklist;
			agent = near_storage_nodes_.find(next_node_id)->second;
			break;
		}
		case Message::message_type_cache_info_for_user: {
			this_event_type = Event::event_type_user_receives_node_data;
//			agent = &(MyToolbox::users_map_.find(next_node_id)->second);
			agent = near_users_.find(next_node_id)->second;
			break;
		}
		default:
			break;
		}
		Event receive_message_event(new_schedule_time, this_event_type);
		receive_message_event.set_agent(agent);
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

void StorageNode::set_measure_indeces() {
	for (int i = 0; i < MyToolbox::num_sensors_; i++) {
		indeces_msr_to_keep_.push_back(i + 1);
	}

	default_random_engine generator = MyToolbox::generator_;
	for (int i = 1; i <= MyToolbox::num_sensors_ - LT_degree_; i++) {
		std::uniform_int_distribution<int> distribution(0, indeces_msr_to_keep_.size() - 1);
		int n = distribution(generator);
		vector<int>::iterator it = indeces_msr_to_keep_.begin();
		for (int j = 0; j < n; j++) {
			it++;
		}
		indeces_msr_to_keep_.erase(it);
	}

//	cout << "@Node " << node_id_ << " (d=" << LT_degree_ << ") :";
//	for (vector<int>::iterator it = indeces_msr_to_keep_.begin(); it != indeces_msr_to_keep_.end(); it++) {
//		cout << " " << *it;
//	}
//	cout << endl;
}

bool StorageNode::check_consistency() {
	vector<MeasureKey> keys;
	for (map<unsigned int, SensorInfo>::iterator info_it = stored_measures_.begin(); info_it != stored_measures_.end(); info_it++) {
		keys.push_back(info_it->second.most_recent_key_);
	}
	return data_collector->check_measure_consistency(keys, xored_measure_);
}
