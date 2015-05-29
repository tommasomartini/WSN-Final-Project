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
#include "reinit_query.h"
#include "reinit_response.h"

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
	//	cout << "Node " << node_id_ << " received measure (s" << measure->get_source_sensor_id() << ", " << measure->get_measure_id() << ") at " << MyToolbox::current_time_ << endl;
	unsigned int source_id = measure->get_source_sensor_id();  // measure from sensor source_id
	if (measure->get_measure_type() == Measure::measure_type_new) { // new measure from a new sensor
		//			cout << "Node " << node_id_ << " received NEW measure (s" << measure->get_source_sensor_id() << ", " << measure->get_measure_id() << "), value: " << int(measure->measure_) << endl;
		if (find(ignore_new_list.begin(), ignore_new_list.end(), measure->get_source_sensor_id()) == ignore_new_list.end()) {	// ignore all the measures of the sensors in the ignore list
			//				cout << " not in my ignore list. Now it is in my ignore list" << endl;
			ignore_new_list.push_back(source_id);	// I should process a new measure for each sensor just once
			//				if (last_measures_.find(source_id) == last_measures_.end()) {  // not yet received a msg from this sensor

			indeces_counter_++;
			if (indeces_pointer_ < int(indeces_msr_to_keep_.size()) && indeces_counter_ == indeces_msr_to_keep_.at(indeces_pointer_)) {	// I have to keep this measure
				indeces_pointer_++;
				xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
				last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
				data_collector->update_num_msr_per_cache(node_id_, last_measures_.size());
				//					cout << "  >ACCEPT< this measure. Store: " << int(xored_measure_) << endl;
			} else {	// do NOT keep this measure
				//					cout << "  do >NOT< accept" << endl;
			}

			/*
				int k = MyToolbox::num_sensors_;
				int d = LT_degree_;
				default_random_engine gen = MyToolbox::generator_;
				bernoulli_distribution bernoulli_distrib(d * 1. / k);
				if (bernoulli_distrib(gen)) { // accept the new msg with probability d/k
					xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
					last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
					data_collector->update_num_msr_per_cache(node_id_, last_measures_.size());
					cout << "  >ACCEPT< this measure. Store: " << int(xored_measure_) << endl;
				} else {
					cout << "  do >NOT< accept" << endl;
				}
			 */
		} else {
			//				cout << " in my ignore list" << endl;
		}
	} else if (measure->get_measure_type() == Measure::measure_type_update) { // update measure from a sensor: always accept it, if I'm collecting this sensor's measures
		//			cout << "Node " << node_id_ << " received UPDATE measure (s" << measure->get_source_sensor_id() << ", " << measure->get_measure_id() << "), value: " << int(measure->measure_) << endl;
		if (last_measures_.find(source_id) != last_measures_.end()) {  // already received a msg from this sensor
			//				cout << " already received a measure from this sensor" << endl;
			// if the received measure is one unit higher than the stored one everything is ok, otherwise I missed a measure and I have to re-initialize the system
			if (measure->get_measure_id() == last_measures_.at(source_id) + 1) {	// actual new measure for me and received in order: I have to update
				xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
				last_measures_.find(source_id)->second = measure->get_measure_id();  // save this measure // save this message as the last received message from sensor source_id
				//					cout << "  in order measure. Now store: " << int(xored_measure_) << endl;
			} else if (measure->get_measure_id() <= last_measures_.at(source_id)) {	// already receive this update measure
				//					cout << "  old measure. Do nothing."	<< endl;
				// do nothing
			} else {	// out of order update measure
				MeasureKey outdated_measure_key(source_id, last_measures_.find(source_id)->second);	// create the key of the outdatet measure I have
				outdated_measure_keys_.push_back(outdated_measure_key);	// store it in the list of the outdated measures
				last_measures_.erase(source_id);	// I don't follow this sensor anymore
				data_collector->update_num_msr_per_cache(node_id_, last_measures_.size());
				//					cout << "  out of order measure!" << endl;
			}
		} else {
			//				cout << " I don't hold measures from this sensor" << endl;
		}
	}

	measure->increase_hop_counter();	// increase the hop-counter
	int hop_limit = MyToolbox::max_num_hops_;
	if (measure->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
//		cout << " forward message" << endl;
		new_events = send(get_random_neighbor(), measure);	// propagate it!
	} else {	// the message must not be forwarded again
//		cout << " do NOT forward the message" << endl;
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
//	ofstream myfile("move_user.txt", ios::app);
//	if (myfile.is_open()) {
//		myfile << "->Node" << node_id_ << " receives user request" << endl;
//		myfile.close();
//	}
	//	else cout << "Unable to open file";

//	cout << "Cache " << node_id_ << " receives request of user " << sender_user_id << endl;

	vector<Event> new_events;
	if (!reinit_mode_) {	// if in reinit mode ignore users' requests
		vector<MeasureKey> keys;
		for (map<unsigned int, unsigned int>::iterator msr_it = last_measures_.begin(); msr_it != last_measures_.end(); msr_it++) {
			MeasureKey key(msr_it->first, msr_it->second);
			keys.push_back(key);
		}
		if (keys.size() > 0) {
			NodeInfoMessage* node_info_msg = new NodeInfoMessage(node_id_, xored_measure_, keys, outdated_measure_keys_, my_blacklist_);
			//			ofstream myfile("move_user.txt", ios::app);
			//			if (myfile.is_open()) {
			//				myfile << "->Node" << node_id_ << " invia " << xored_measure_ << endl;
			//				myfile.close();
			//			}
			//			else cout << "Unable to open file";
			new_events = send(sender_user_id, node_info_msg);
		} else {
			//			ofstream myfile("move_user.txt", ios::app);
			//			if (myfile.is_open()) {
			//				myfile << "->Node" << node_id_ << " nothing to send" << endl;
			//				myfile.close();
			//			}
			//			else cout << "Unable to open file";
		}
	}
	return new_events;
}

/*  Occasionally I check if the sensors I am supervising are OK
 */
vector<Event> StorageNode::check_sensors() {
//	cout << "Node " << node_id_ << " checks pings" << endl;
	vector<Event> new_events;
	if (keep_checking_sensors_) {
		vector<unsigned int> expired_sensors;	// list of sensor ids which didn't answer for 3 times in a row

		for (auto& x : supervised_map_){	// for each sensor in my supervised list...
			unsigned int curr_sns_id = x.first;
			if(x.second + (3 * MyToolbox::ping_frequency_) < MyToolbox::current_time_){  // ...if it didn't answer for 3 times...
				my_blacklist_.push_back(curr_sns_id);	// ...put it in my blacklist...
				expired_sensors.push_back(curr_sns_id);	// ...and in a list I use to update the structures
				if (last_measures_.find(curr_sns_id) != last_measures_.end()) {	// if I'm storing a measure from this sensor
					unsigned int curr_sns_msr_id = last_measures_.find(curr_sns_id)->second;	// take the id of the measure I'm storing of this sensor
					MeasureKey key(curr_sns_id, curr_sns_msr_id);	// create a key
					outdated_measure_keys_.push_back(key);	// store the key into the vector of the outdated measure
					last_measures_.erase(curr_sns_id);
				}
				data_collector->register_broken_sensor(x.first);
				MyToolbox::remove_sensor(x.first);
				//			cout << " s" << x.first << " dead" << endl;
			} else {
				//			cout << " s" << x.first << " alive" << endl;
			}
		}
		// remove from the supervisioned_map the dead sensors
		for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) {
			supervised_map_.erase(*it);
		}
		//	cout << " " << expired_sensors.size() << " sensors dead" << endl;
		//	cout << " new size of supervisioned map: " << supervised_map_.size() << endl;

		if (expired_sensors.size() > 0) {	// if there are some expired sensors I have to spread this info
			BlacklistMessage* list = new BlacklistMessage(expired_sensors);
			list->message_type_= Message::message_type_blacklist;
			new_events = send(get_random_neighbor(), list);
			//		cout << "Time " << MyToolbox::current_time_ * 1. / pow(10, 9) << endl;
			//		cout << "@ @ @ Node " << node_id_ << " generate a blacklist with";
			//		for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) {
			//			cout << " " << *it;
			//		}
			//		cout << endl;
			//		cout << "Curr time " << MyToolbox::current_time_ << endl;
			//		cout << "Last ping " << supervised_map_.find(*(expired_sensors.begin()))->second << endl;
			//		cout << "ping time " << MyToolbox::ping_frequency_ << endl;
			//		cout << "expir time " << (supervised_map_.find(*(expired_sensors.begin()))->second) + 3 * MyToolbox::ping_frequency_ << endl;
		}

		//	ping_check_counter_++;
		//	if (ping_check_counter_ < num_ping_checks_) {
		Event new_event(MyToolbox::current_time_ + MyToolbox::check_sensors_frequency_, Event::event_type_cache_checks_sensors);
		new_event.set_agent(this);
		new_events.push_back(new_event);
		//	}
	}
	return new_events;
}

/*  I received a blacklist message: this message contains the measure I have to remove
 */
vector<Event> StorageNode::spread_blacklist(BlacklistMessage* list) {
	vector<Event> new_events;
//	cout << "Node " << node_id_ << " receives bl " << endl;
	for (vector<unsigned int>::iterator sns_it = list->sensor_ids_.begin(); sns_it != list->sensor_ids_.end(); sns_it++) {
//		cout << " " << *sns_it << endl;
		data_collector->record_bl(node_id_, *sns_it);
	}
	if (!reinit_mode_) {	// if in reinit mode just spread the blacklist, but don't look at it
		vector<unsigned int> expired_sensors = list->sensor_ids_;
		for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) { 	// for each id in the blacklist...
			bool msr_from_this_sns = last_measures_.find(*it) != last_measures_.end();	// Do I have a measure from this sensor?
			bool not_yet_in_my_blacklist = find(my_blacklist_.begin(), my_blacklist_.end(), *it) == my_blacklist_.end();	// Is it NOT already in my blacklist?
			if (msr_from_this_sns && not_yet_in_my_blacklist) {	// if so...
				my_blacklist_.push_back(*it);	// ...put its id in my blacklist too
				unsigned int msr_id = last_measures_.find(*it)->second;	// then pick the id of the last measure I received from this dead sensor...
				MeasureKey key(*it, msr_id);	// ...make a key of the pair...
				outdated_measure_keys_.push_back(key);	// ...and store the pair into my outdated measure key vector
				last_measures_.erase(*it);	// quit following this sensor
			}
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

/*  A user informs me about what measures are obsolete
 */
void StorageNode::refresh_xored_data(OutdatedMeasure* refresh_message){
	cout << "Cache " << node_id_ << " rx rfresh message" << endl;

	cout << "Ref message removed: " << endl;
	for (auto& elem : refresh_message->removed_) {
		cout << " (s" << elem.sensor_id_ << "," << elem.measure_id_ << ")";
	}
	cout << endl;
	cout << "Ref message inserted: " << endl;
	for (auto& elem : refresh_message->inserted_) {
		cout << " (s" << elem.sensor_id_ << "," << elem.measure_id_ << ")";
	}
	cout << endl;


	cout << "Cache " << node_id_ << " BEFORE stores: " << int(xored_measure_) << endl;
	cout << " Last:" << endl;
	for (map<unsigned int, unsigned int>::iterator last_it = last_measures_.begin(); last_it != last_measures_.end(); last_it++) {
		cout << " - (s" << last_it->first << "," << last_it->second << ")" << endl;
	}
	cout << " Outdated:" << endl;
	for (vector<MeasureKey>::iterator outdated_it = outdated_measure_keys_.begin(); outdated_it != outdated_measure_keys_.end(); outdated_it++) {
		cout << " - (s" << outdated_it->sensor_id_ << "," << outdated_it->measure_id_ << ")";
		if (find(my_blacklist_.begin(), my_blacklist_.end(), outdated_it->sensor_id_) != my_blacklist_.end()) {
			cout << " -> dead sensor";
		}
		cout << endl;
	}

	vector<MeasureKey> removed = refresh_message->removed_;		// list of the measure I have removed because outdated
	vector<MeasureKey> inserted = refresh_message->inserted_;		// list of the measure I have inserted
	bool can_update = true;
	for (vector<MeasureKey>::iterator rem_it = removed.begin(); rem_it != removed.end(); rem_it++) {	// for each measure I could erase
		if (find(outdated_measure_keys_.begin(), outdated_measure_keys_.end(), *rem_it) == outdated_measure_keys_.end()) {	// if it is NOT among the measure I have to erase
			can_update = false;
			break;
		}
	}
	for (vector<MeasureKey>::iterator ins_it = inserted.begin(); ins_it != inserted.end(); ins_it++) {	// for each measure I could add
		if (find(my_blacklist_.begin(), my_blacklist_.end(), ins_it->sensor_id_) == my_blacklist_.end()) {	// if this sensor is NOT in my blacklist I cannot erase this measure
			can_update = false;
			break;
		}
	}
	if (can_update) {	// if I can perform the xor while preserving consistency
		xored_measure_ ^= refresh_message->xored_data_;
		for (vector<MeasureKey>::iterator rem_it = removed.begin(); rem_it != removed.end(); rem_it++) {	// for each measure I have erased
			outdated_measure_keys_.erase(find(outdated_measure_keys_.begin(), outdated_measure_keys_.end(), *rem_it));	// remove it from my outdated list: these data are no more outdated
		}
		for (vector<MeasureKey>::iterator ins_it = inserted.begin(); ins_it != inserted.end(); ins_it++) {	// for each measure I have added
			last_measures_.insert(pair<unsigned int, unsigned int>(ins_it->sensor_id_, ins_it->measure_id_));	// add this measure to the list of the measure I have in my xor
		}
	}

	cout << "Cache " << node_id_ << " AFTER stores: " << int(xored_measure_) << endl;
	cout << " Last:" << endl;
	for (map<unsigned int, unsigned int>::iterator last_it = last_measures_.begin(); last_it != last_measures_.end(); last_it++) {
		cout << " - (s" << last_it->first << "," << last_it->second << ")" << endl;
	}
	cout << " Outdated:" << endl;
	for (vector<MeasureKey>::iterator outdated_it = outdated_measure_keys_.begin(); outdated_it != outdated_measure_keys_.end(); outdated_it++) {
		cout << " - (s" << outdated_it->sensor_id_ << "," << outdated_it->measure_id_ << ")";
		if (find(my_blacklist_.begin(), my_blacklist_.end(), outdated_it->sensor_id_) != my_blacklist_.end()) {
			cout << " -> dead sensor";
		}
		cout << endl;
	}

	delete refresh_message;
}

vector<Event> StorageNode::receive_reinit_query(unsigned int sender_user_id) {
	vector<Event> new_events;
	unsigned char to_pass_xored_measure = 0;
	map<unsigned int, unsigned int> to_pass_last_msrs;
	vector<unsigned int> to_pass_blacklist;
	if (!reinit_mode_) {	// I'm "stable", I can pass my data
		to_pass_xored_measure = xored_measure_;
		to_pass_last_msrs = last_measures_;
		to_pass_blacklist = my_blacklist_;
	}
	ReinitResponse* reinit_response = new ReinitResponse();
	reinit_response->xored_measure_ = to_pass_xored_measure;
	reinit_response->last_measures_ = to_pass_last_msrs;
	reinit_response->blacklist_ = to_pass_blacklist;

	new_events = send(sender_user_id, reinit_response);

	return new_events;
}

vector<Event> StorageNode::receive_reinit_response() {
	vector<Event> new_events;

	return new_events;
}


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
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//			cout << " him not free" << endl;
			MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
			try_again_event.set_agent(this);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else {  // sender and receiver both idle, can send the message
			// Compute the message time
			MyTime processing_time = MyToolbox::get_random_processing_time();
			unsigned int num_total_bits = message->get_message_size();
			MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
			if (message->message_type_ == Message::message_type_reinit_query) {
				processing_time = 0;
			}
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
			case Message::message_type_reinit_query: {
				this_event_type = Event::event_type_cache_gets_reinit_query;
				agent = near_storage_nodes_.find(next_node_id)->second;
				break;
			}
			case Message::message_type_reinit_response: {
				this_event_type = Event::event_type_cache_gets_reinit_response;
				agent = near_storage_nodes_.find(next_node_id)->second;
				break;
			}
			default:
				break;
			}
			Event receive_message_event(new_schedule_time, this_event_type);
			receive_message_event.set_agent(agent);
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
		cout << " " << next_node_id << " non più vicino" << endl;
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
		case Message::message_type_reinit_query: {
			give_up = true;
			break;
		}
		case Message::message_type_reinit_response: {
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
				MyTime schedule_time = (message->message_type_ == Message::message_type_reinit_query ? 0 : MyToolbox::get_random_processing_time()) + MyToolbox::get_tx_offset();
				event_queue_.front().set_time(schedule_time);	// change the schedule time of the message I am trying to send

				Event try_again_event(schedule_time, Event::event_type_cache_re_send);
				try_again_event.set_agent(this);
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
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//		cout << " him not free!" << endl;
		MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::event_type_cache_re_send);
		try_again_event.set_agent(this);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else {  // sender and receiver both idle, can send the message
//		cout << " everybody free!" << endl;
		// Compute the message time
		MyTime processing_time = MyToolbox::get_random_processing_time();
		unsigned int num_total_bits = message->get_message_size();
		MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
		if (message->message_type_ == Message::message_type_reinit_query) {
			processing_time = 0;
		}
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
		case Message::message_type_reinit_query: {
			this_event_type = Event::event_type_cache_gets_reinit_query;
			agent = near_storage_nodes_.find(next_node_id)->second;
			break;
		}
		default:
			break;
		}
		Event receive_message_event(new_schedule_time, this_event_type);
		receive_message_event.set_agent(agent);
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
			next_send_event.set_message(event_queue_.front().get_message());
			new_events.push_back(next_send_event); // schedule the next event
		}
		return new_events;
	}
}

/*
 * Re-initialize all the variables but the supervisioned map.
 * Ask in a serial way my neighbours to send me their information and XOR them.
 * The LT degree is now used only in case a new sensor joins the network.
 */
vector<Event> StorageNode::reinitialize() {
	vector<Event> new_events;
	cout << "Storage node " << node_id_ << " reinitializing" << endl;

	reinit_mode_ = true;

	// Re-initialize (almost) everything
	LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();	// re-initialize the node degree
	xored_measure_ = 0;	// reinitialize the xored measure
	last_measures_.clear();	// erase all the last measures
	ignore_new_list.clear();	// erase all the ignore ids
	//my_blacklist_.clear();	// erase my blacklist	// not necessary!

//	// List of the neighbours to interrogate
//	vector<unsigned int> neighbours_to_query;
//	for (map<unsigned int, Node*>::iterator it = near_storage_nodes_->begin(); it != near_storage_nodes_->end(); it++) {
//		neighbours_to_query.push_back(it->first);
//	}
//
//	Node* next_node = near_storage_nodes_->begin()->second;
//	new_events = send(next_node, &reinit_query);

	ReinitQuery reinit_query;
	vector<Event> partial_event_list;	// events returned by the communication with each neighbour
	for (map<unsigned int, StorageNode*>::iterator it = near_storage_nodes_.begin(); it != near_storage_nodes_.end(); it++) {	// for each neighbour...
		partial_event_list = send(it->first, &reinit_query);	// ...send him the reinit request, or at least schedule it...
		new_events.insert(new_events.end(), partial_event_list.begin(), partial_event_list.end());	// ...add the events to the list of events...
		partial_event_list.clear();	// ...and set the list for the next round and neighbour
	}

	return new_events;
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
