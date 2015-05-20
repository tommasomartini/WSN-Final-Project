#include <iostream>
#include <algorithm>    // std::max
#include <iostream>
#include <math.h>
#include <random>

#include "storage_node.h"
#include "my_toolbox.h"
#include "measure.h"
#include "blacklist_message.h"
#include "outdated_measure.h"
#include "storage_node_message.h"
#include "reinit_query.h"
#include "reinit_response.h"

using namespace std;

/**************************************
    Event execution methods
 **************************************/
/*  Receive a measure either from a sensor or from another cache node
 */
vector<Event> StorageNode::receive_measure(Measure* measure) {
	vector<Event> new_events;
	data_collector->record_msr(measure->measure_id_, measure->source_sensor_id_, node_id_, 1);
//	bool out_of_order_msr = false;
	unsigned int source_id = measure->get_source_sensor_id();  // measure from sensor source_id
	if (!reinit_mode_) {	// if in reinit mode I only spread the measure
		if (find(ignore_new_list.begin(), ignore_new_list.end(), measure->get_source_sensor_id()) == ignore_new_list.end()) {	// ignore all the measures of the sensors in the ignore list
			if (measure->get_measure_type() == Measure::measure_type_new) { // new measure from a new sensor: accept it wp d/k
				ignore_new_list.push_back(source_id);	// I shoud process a new measure for each sensor just once
				if (last_measures_.find(source_id) == last_measures_.end()) {  // not yet received a msg from this sensor
//					data_collector->record_msr(measure->get_measure_id(), measure->get_source_sensor_id(), node_id_, 1);
					int k = MyToolbox::num_sensors_;
					int d = LT_degree_;
					default_random_engine gen = MyToolbox::generator_;
					bernoulli_distribution bernoulli_distrib(d * 1. / k);
					if (bernoulli_distrib(gen)) { // accept the new msg with probability d/k
//						cout << node_id_ << " misura nuova <" << measure->get_measure_id() << ", s" << measure->get_source_sensor_id() << "> ... KEEP!" << endl;
						xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
						last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
//						data_collector->record_msr(measure->get_measure_id(), measure->get_source_sensor_id(), node_id_, 2);
					} else {
//						cout << node_id_ << " misura nuova <" << measure->get_measure_id() << ", s" << measure->get_source_sensor_id() << "> ... IGNORE!" << endl;
//						ignore_new_list.push_back(measure->get_source_sensor_id());
					}
					//      last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
				}
			} else if (measure->get_measure_type() == Measure::measure_type_update) { // update measure from a sensor: always accept it, if I'm collecting this sensor's measures
//				cout << node_id_ << " misura update <" << measure->get_measure_id() << ", s" << measure->get_source_sensor_id() << ">" << endl;
				if (last_measures_.find(source_id) != last_measures_.end()) {  // already received a msg from this sensor
					// if the received measure is one unit higher than the stored one everything is ok, otherwise I missed a measure and I have to re-initialize the system
					if (measure->get_measure_id() == last_measures_.at(source_id) + 1) {	// actual new measure for me and received in order: I have to update
						xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
						last_measures_.find(source_id)->second = measure->get_measure_id();  // save this measure // save this message as the last received message from sensor source_id
//						cout << "Update measure from sensor " << measure->get_source_sensor_id() << "(msr id = " << measure->get_measure_id() << ")" << endl;
					} else if (measure->get_measure_id() <= last_measures_.at(source_id)) {	// already receive this update measure
						// do nothing
					} else {	// out of order update measure
//						out_of_order_msr = true;	// after propagating I will have to reinitialize or stopping following the node

						MeasureKey outdated_measure_key(source_id, last_measures_.find(source_id)->second);	// create the key of the outdatet measure I have
						outdated_measure_keys_.push_back(outdated_measure_key);	// store it in the list of the outdated measures
						last_measures_.erase(source_id);	// I don't follow this sensor anymore
					}
				}
			}
		}
	}

	measure->increase_hop_counter();	// increase the hop-counter
	int hop_limit = MyToolbox::max_num_hops_;
	if (measure->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
		new_events = send2(get_random_neighbor(), measure);	// propagate it!
	} else {	// the message must not be forwarded again
		data_collector->erase_msr(measure->measure_id_, measure->source_sensor_id_);
//		cout << "measure (s" << measure->source_sensor_id_ << ", " << measure->measure_id_ << ") stops" << endl;
//		data_collector->print_data();
	}
//	if (out_of_order_msr) {	// after propagating I have to reinitialize
////		// Policy 1: reinitialize
////		vector<Event> other_events = reinitialize();	// reinitialize and get some new events
////		new_events.insert(new_events.end(), other_events.begin(), other_events.end());	// append the new events
//
//		// Policy 2: quit following this sensor
//
//	}

	return new_events;
} 

/*  I have already tried to send a message to someone, but I failed. Now I try again!
 */
// FIXME to deprecate
vector<Event> StorageNode::try_retx(Message* message, unsigned int next_node_id) {
	vector<Event> new_events;
	new_events = re_send(message);
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
// TODO to be deprecated
void StorageNode::set_supervision_map_(int sensor_id, int new_time){
	// If it is the first time I receive a ping from a sensor it means that that sensor wants me to be his supervisor. I save it in my supervisor map
	if (supervisioned_map_.find(sensor_id) == supervisioned_map_.end()){
		supervisioned_map_.insert(std::pair<int, int>(sensor_id, new_time));
	}
	else {
		supervisioned_map_.find(sensor_id)->second = new_time;
	}
}


/*  A sensor is telling me it is alive
 */
void StorageNode::receive_hello(unsigned int sensor_id) {
	cout << "Cache " << node_id_ << " gets pings from " << sensor_id << endl;
	// If it is the first time I receive a ping from a sensor it means that that sensor wants me to be his supervisor. I save it in my supervisor map
	if (supervisioned_map_.find(sensor_id) == supervisioned_map_.end()){
		supervisioned_map_.insert(std::pair<int, int>(sensor_id, MyToolbox::current_time_));
	}
	else {
		supervisioned_map_.find(sensor_id)->second = MyToolbox::current_time_;
	}
}

/*  A user asks me to send him my data
 */
vector<Event> StorageNode::receive_user_request(unsigned int sender_user_id) {
	vector<Event> new_events;
	if (!reinit_mode_) {	// if in reinit mode ignore users' requests
//		vector<unsigned int> my_sensor_ids;
//		for (map<unsigned int, unsigned int>::iterator it = last_measures_.begin(); it != last_measures_.end(); it++) {
//			my_sensor_ids.push_back(it->first);
//		}
		map<unsigned int, unsigned int> msrs_info = last_measures_;
		StorageNodeMessage msg(xored_measure_, msrs_info);
		Node* next_node = MyToolbox::users_map_ptr_->find(sender_user_id)->second;
//		msg.set_receiver_node_id(next_node->get_node_id()); // should be equal to sender_user_id
		new_events = send2(sender_user_id, &msg);
	}
	return new_events;
}

/*  Occasionally I check if the sensors I am supervising are OK
 */
vector<Event> StorageNode::check_sensors() {
	vector<Event> new_events;
	vector<unsigned int> expired_sensors;	// list of sensor ids which didn't answer for 3 times in a row

	for (auto& x : supervisioned_map_){	// for each sensor in my supervised list...
		if(x.second + (3 * MyToolbox::ping_frequency_) < MyToolbox::current_time_){  // ...if it didn't answer for 3 times...
			cout << "Sensor " << x.first << " dead" << endl;
			my_blacklist_.push_back(x.first);	// ...put it in my blacklist...
			expired_sensors.push_back(x.first);	// ...and in a list I use to update the structures
		}
	}
	// remove from the supervisioned_map the dead sensors
	for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) {
		supervisioned_map_.erase(*it);
//		supervisioned_map_.erase(supervisioned_map_.find(*it));
	}

	if (expired_sensors.size() > 0) {	// if there are some expired sensors I have to spread this info
		BlacklistMessage* list = new BlacklistMessage(expired_sensors);
		list->message_type_= Message::message_type_blacklist;
		new_events = send2(get_random_neighbor(), list);
	}

	if (supervisioned_map_.size() > 0) {	// if I have some supervisioned sensor check it in a while
		Event new_event(MyToolbox::current_time_ + MyToolbox::check_sensors_frequency_, Event::check_sensors);
		new_event.set_agent(this);
		new_events.push_back(new_event);
	}

	return new_events;
}

/*  I received a blacklist message: this message contains the measure I have to remove
 */
vector<Event> StorageNode::spread_blacklist(BlacklistMessage* list) {
	vector<Event> new_events;
	if (!reinit_mode_) {	// if in reinit mode just spread the blacklist, but don't look at it
		vector<unsigned int> expired_sensors = list->get_id_list3();
		for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) { 	// for each id in the blacklist...
			bool msr_from_this_sns = last_measures_.find(*it) != last_measures_.end();	// Do I have a measure from this sensor?
			bool not_yet_in_my_blacklist = find(my_blacklist_.begin(), my_blacklist_.end(), *it) == my_blacklist_.end();	// Is it NOT already in my blacklist?
			if (msr_from_this_sns && not_yet_in_my_blacklist) {	// if so...
				my_blacklist_.push_back(*it);	// ...put its id in my backlist too
				unsigned int msr_id = last_measures_.find(*it)->second;	// then pick the id of the last measure I received from this dead sensor...
				MeasureKey key(*it, msr_id);	// ...make a key of the pair...
				outdated_measure_keys_.push_back(key);	// ...and store the pair into my outdated measure key vector
			}
		}
	}
	int hop_limit = MyToolbox::max_num_hops_;
	if (list->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
		list->increase_hop_counter();
		unsigned int next_node_index = get_random_neighbor();
		map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
		for (int i = 0; i < next_node_index; i++) {
			node_iter++;
		}
		StorageNode *next_node = (StorageNode*)node_iter->second;
//		list->set_receiver_node_id(next_node->get_node_id());
		list->message_type_ = Message::message_type_blacklist;
		new_events = send2(next_node->get_node_id(), list);
	}
	return new_events;
}

/*  A user informs me about what measures are obsolete
 */
vector<Event> StorageNode::remove_mesure(OutdatedMeasure* message_to_remove){
	vector<Event> new_events;
	if (!reinit_mode_) {	// if in reinit mode ignore this message and just pass it forward
		map<unsigned int, unsigned char> outdated_measure = message_to_remove->get_outdaded_measure();
		for (map<unsigned int, unsigned char>::iterator it = outdated_measure.begin(); it != outdated_measure.end(); ++it) {	// for each measure in the list...
			if (last_measures_.find(it->first) != last_measures_.end()){	// ...if I have used that measure...
				xored_measure_ = xored_measure_ ^ it->second;		// ...remove it...
				last_measures_.erase(last_measures_.find(it->first)); 	// ...and erase the sensor id from my list as well
			}
			if (find(my_blacklist_.begin(), my_blacklist_.end(), it->first) != my_blacklist_.end()) {	// if I have the sensor in my blacklist erase it
				my_blacklist_.erase(find(my_blacklist_.begin(), my_blacklist_.end(), it->first));
			}
		}
	}
	int hop_limit = MyToolbox::max_num_hops_;
	if (message_to_remove->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
		message_to_remove->increase_hop_counter();
//		unsigned int next_node_index = rand() % near_storage_nodes_->size();
//		map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
//		for (int i = 0; i < next_node_index; i++) {
//			node_iter++;
//		}
//		StorageNode *next_node = (StorageNode*)node_iter->second;
////		    new_events = send2(next_node->get_node_id(), message_to_remove);	// my choice is not to flood the network with heavy messages
	}
	return new_events;
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

	new_events = send2(sender_user_id, reinit_response);

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
vector<Event> StorageNode::send2(unsigned int next_node_id, Message* message) {
	vector<Event> new_events;

	// Set sender and receiver
	message->set_receiver_node_id(next_node_id);
	message->set_sender_node_id(node_id_);

	if (!event_queue_.empty()) {  // already some pending event -> does not generate new events
		Event event_to_enqueue(0, Event::storage_node_try_to_send);	// execution time does not matter now...
		event_to_enqueue.set_agent(this);
		event_to_enqueue.set_message(message);
		event_queue_.push(event_to_enqueue);
	} else {  // no pending events
		map<unsigned int, MyTime>* timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
		MyTime current_time = MyToolbox::current_time_;  // current time of the system
		MyTime my_available_time = timetable->find(node_id_)->second; // time this node gets free (ME)
		MyTime next_node_available_time = timetable->find(next_node_id)->second;  // time next_node gets free
		if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
			MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
			try_again_event.set_agent(this);
			try_again_event.set_message(message);
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
			MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
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
			switch (message->message_type_) {
			case Message::message_type_measure: {
				this_event_type = Event::storage_node_receive_measure;
				break;
			}
			case Message::message_type_blacklist: {
				this_event_type = Event::blacklist_sensor;
				break;
			}
			case Message::message_type_remove_measure: {
				this_event_type = Event::remove_measure;
				break;
			}
			case Message::message_type_measures_for_user: {
				this_event_type = Event::user_receive_data;
				break;
			}
			case Message::message_type_reinit_query: {
				this_event_type = Event::storage_get_reinit_query;
				break;
			}
			case Message::message_type_reinit_response: {
				this_event_type = Event::storage_get_reinit_response;
				break;
			}
			default:
				break;
			}
			Event receive_message_event(new_schedule_time, this_event_type);
			receive_message_event.set_agent(near_storage_nodes_->find(next_node_id)->second);
			receive_message_event.set_message(message);
			new_events.push_back(receive_message_event);

			// Update the timetable
			timetable->find(node_id_)->second = new_schedule_time; // update my available time
			for (map<unsigned int, Node*>::iterator node_it = near_storage_nodes_->begin(); node_it != near_storage_nodes_->end(); node_it++) {
				timetable->find(node_it->first)->second = new_schedule_time;
			}
			MyToolbox::timetable_ = timetable;  // upload the updated timetable

			// If I am here the queue was empty and it is still empty! I have to do nothing on the queue!
		}
	}
	return new_events;
}

vector<Event> StorageNode::re_send(Message* message) {
	vector<Event> new_events;

	unsigned int next_node_id = message->get_receiver_node_id();
	if (near_storage_nodes_->find(next_node_id) == near_storage_nodes_->end()) {	// my neighbor there is no longer
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
		case Message::message_type_remove_measure: {
			give_up = false;
			break;
		}
		case Message::message_type_measures_for_user: {
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
			event_queue_.pop();	// remove this event from the queue, I'm not going to execute it anymore
			if (event_queue_.empty()) {	// if the queue is now empty...
				return new_events;	// return an empty vector, I don't have new events to schedule
			} else {
				Message* new_message = event_queue_.front().get_message();
				return re_send(new_message);
			}
		} else {	// I cannot give up! Find another node to spread the message
			next_node_id = get_random_neighbor();	// find another neighbour
			if (next_node_id == 0) {	// no more neighbours, I'm isolated. Postpone my delivery
				MyTime schedule_time = (message->message_type_ == Message::message_type_reinit_query ? 0 : MyToolbox::get_random_processing_time()) + MyToolbox::get_tx_offset();
				event_queue_.front().set_time(schedule_time);	// change the schedule time of the message I am trying to send

				Event try_again_event(schedule_time, Event::storage_node_try_to_send);
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

	map<unsigned int, MyTime>* timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
	MyTime current_time = MyToolbox::current_time_;  // current time of the system
	MyTime my_available_time = timetable->find(node_id_)->second; // time this node gets free (ME)
	MyTime next_node_available_time = timetable->find(next_node_id)->second;  // time next_node gets free
	if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
		MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
		try_again_event.set_agent(this);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
		MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
		try_again_event.set_agent(this);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
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
		Event::EventTypes this_event_type;
		switch (message->message_type_) {
		case Message::message_type_measure: {
			this_event_type = Event::storage_node_receive_measure;
			break;
		}
		case Message::message_type_blacklist: {
			this_event_type = Event::blacklist_sensor;
			break;
		}
		case Message::message_type_remove_measure: {
			this_event_type = Event::remove_measure;
			break;
		}
		case Message::message_type_measures_for_user: {
			this_event_type = Event::user_receive_data;
			break;
		}
		case Message::message_type_reinit_query: {
			this_event_type = Event::storage_get_reinit_query;
			break;
		}
		default:
			break;
		}
		Event receive_message_event(new_schedule_time, this_event_type);
		receive_message_event.set_agent(near_storage_nodes_->find(next_node_id)->second);
		receive_message_event.set_message(message);
		new_events.push_back(receive_message_event);

		// Update the timetable
		timetable->find(node_id_)->second = new_schedule_time; // update my available time
		for (map<unsigned int, Node*>::iterator node_it = near_storage_nodes_->begin(); node_it != near_storage_nodes_->end(); node_it++) {
			timetable->find(node_it->first)->second = new_schedule_time;
		}
		MyToolbox::timetable_ = timetable;  // upload the updated timetable

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
	for (map<unsigned int, Node*>::iterator it = near_storage_nodes_->begin(); it != near_storage_nodes_->end(); it++) {	// for each neighbour...
		partial_event_list = send2(it->first, &reinit_query);	// ...send him the reinit request, or at least schedule it...
		new_events.insert(new_events.end(), partial_event_list.begin(), partial_event_list.end());	// ...add the events to the list of events...
		partial_event_list.clear();	// ...and set the list for the next round and neighbour
	}

	return new_events;
}

//unsigned int StorageNode::get_random_neighbor() {
//	if (near_storage_nodes_->size() == 0) {	// I don't hve neighbours
//		return 0;
//	} else {
//		int next_node_index = rand() % near_storage_nodes_->size();
//		map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
//		for (int i = 0; i < next_node_index; i++) {
//			node_iter++;
//		}
//		StorageNode *next_node = (StorageNode*)node_iter->second;
//		return next_node->get_node_id();
//	}
//}
