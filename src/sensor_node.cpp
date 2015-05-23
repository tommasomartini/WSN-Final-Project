#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include <map>

#include "sensor_node.h"
#include "storage_node.h"
#include "measure.h"
#include "user.h"
//#include "data_collector.h"


/**************************************
    Constructors
**************************************/
SensorNode::SensorNode(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
  measure_id_ = 0;
  first_generated_measure_ = true;
  my_supervisor_id_ = -1;
}

/**************************************
    Event execution methods
**************************************/
/*  Return two events: a new measure generation from the same node and the reception of the measure by a storage node
*/
vector<Event> SensorNode::generate_measure() {

//	cout << "Going to generate a new one..." << MyToolbox::current_time_ << endl;

  vector<Event> new_events; // create the new events

  how_many_measures_++;

  old_measure_data = new_measure_data;
  new_measure_data = get_measure_data();  // generate a random measure
  measure_id_++;
  first_generated_measure_ = false;	// once I generate a measure, the next measure cannot be the first one

   /*
    2 events:
    - send the generated measure to another node. This may be successful or not, that is I can generate either a 
        storage_receive_measure event or a sensor_try_to_send_again_later event
    - with probability p I generate another measure (another sensor_generate_measure event), with probability 1-p the sensor
        breaks up and I genenerate a broken_sensor event.
  */
  Measure* measure = new Measure(0, measure_id_, node_id_, first_generated_measure_ ? Measure::measure_type_new : Measure::measure_type_update);
  unsigned int next_node_id = get_random_neighbor();
  new_events = send2(next_node_id, measure);
  bool generate_another_measure = true;
  default_random_engine generator = MyToolbox::generator_;
  bernoulli_distribution distribution(MyToolbox::sensor_failure_prob_);
  if (distribution(generator)) {
	  generate_another_measure = false;
  }
  // FIXME remove, just for debug
  if (how_many_measures_ > 3) {
	  generate_another_measure = false;
  }
  uniform_int_distribution<int> unif_distrib(MyToolbox::max_measure_generation_delay_ / 2000, MyToolbox::max_measure_generation_delay_ / 1000);	// between 5ms and 10ms
  MyTime rndm_time = MyToolbox::max_measure_generation_delay_;
  MyTime time_next_measure_or_failure = MyToolbox::current_time_ + rndm_time;
//  cout << "Next measure at time " << time_next_measure_or_failure << ": " << MyToolbox::current_time_ << " + " << rndm_time << endl;
  if (generate_another_measure) {
    Event next_measure_event(time_next_measure_or_failure, Event::sensor_generate_measure);
    next_measure_event.set_agent(this);
    new_events.push_back(next_measure_event);
  } else {
    Event failure_event(time_next_measure_or_failure, Event::broken_sensor);
    failure_event.set_agent(this);
    new_events.push_back(failure_event);
  }
  
  return new_events;
}

vector<Event> SensorNode::try_retx(Message* message) {
	vector<Event> new_events;
	new_events = re_send(message);
	return new_events;
}

vector<Event> SensorNode::sensor_ping2() {
	vector<Event> new_events;
	map<unsigned int, StorageNode*>::iterator supervisor_it = near_storage_nodes_.find(my_supervisor_id_);
	while (supervisor_it == near_storage_nodes_.end()) {	// until I don't find a valid neighbour...
		my_supervisor_id_ = get_random_neighbor();	// ...try a new one
		if (my_supervisor_id_ == 0) {	// this sensor has no more neighbours
			return new_events;	// return now and do not schedule another ping!
		}
		supervisor_it = near_storage_nodes_.find(my_supervisor_id_);
	}
	cout << "Sensor " << node_id_ << " pings cache " << my_supervisor_id_ << endl;	// TODO debug
	((StorageNode*)supervisor_it->second)->receive_hello(node_id_);	// send the hello ping
//	Event new_event(MyToolbox::get_current_time() + MyToolbox::ping_frequency, Event::sensor_ping);	// generate the new ping event
	Event new_event(MyToolbox::current_time_ + MyToolbox::ping_frequency_, Event::broken_sensor);	// FIXME for debug only
	new_event.set_agent(this);
	new_events.push_back(new_event);
	return new_events;
}

void SensorNode::set_supervisor() {
  // choose my supervisor as the first node in my list (does not matter how I choose it)
//  my_supervisor_id_ = near_storage_nodes_->begin()->first;
  my_supervisor_id_ = get_random_neighbor();
}


/**************************************
    Private methods
**************************************/

// this method is only for the first send!
vector<Event> SensorNode::send2(unsigned int next_node_id, Message* message) {
	vector<Event> new_events;

	// Set sender and receiver
	message->set_receiver_node_id(next_node_id);
	message->set_sender_node_id(node_id_);

	if (!event_queue_.empty()) {  // already some pending event -> it is for sure a previous measure!
//		cout << " queue not empty" << endl;

		// do not add it! I will use this event to send the actual measure!

//		Event event_to_enqueue(0, Event::storage_node_try_to_send);	// execution time does not matter now...
//		event_to_enqueue.set_agent(this);
//		event_to_enqueue.set_message(message);
//		event_queue_.push(event_to_enqueue);
	} else {  // no pending events
//		cout << " queue empty" << endl;
		map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
		MyTime current_time = MyToolbox::current_time_;  // current time of the system
		MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
		MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
		if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
			MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
			try_again_event.set_agent(this);
			try_again_event.set_message(message);
//			cout << " me not available. Try at " << new_schedule_time << endl;
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
			MyTime off = MyToolbox::get_tx_offset();
			MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
			Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
			try_again_event.set_agent(this);
			try_again_event.set_message(message);
//			cout << " other node not available. Try at " << new_schedule_time << ": " << next_node_available_time << " + " << off << endl;
//			cout << "sum = " << next_node_available_time << " + " << off << " = " << off + next_node_available_time << endl;
			event_queue_.push(try_again_event);	// goes in first position because the queue is empty
			new_events.push_back(try_again_event);
		} else {  // sender and receiver both idle, can send the message
//			cout << " everybody available" << endl;
			// Compute the message time
			MyTime processing_time = MyToolbox::get_random_processing_time();
			unsigned int num_total_bits = message->get_message_size();
			MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
			MyTime new_schedule_time = current_time + processing_time + transfer_time;
			// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
			// Just in case I want to give priority to some particular message...
			Event::EventTypes this_event_type;
			switch (message->message_type_) {
			case Message::message_type_measure: {
				this_event_type = Event::storage_node_receive_measure;
				((Measure*)message)->measure_ = old_measure_data ^ new_measure_data;	// fill the measure with the most updated value
				old_measure_data = new_measure_data;	// update the old measure value
				data_collector->add_msr(((Measure*)message)->measure_id_, node_id_);
				break;
			}
			default:
				break;
			}
			Event receive_message_event(new_schedule_time, this_event_type);
			receive_message_event.set_agent(near_storage_nodes_.find(next_node_id)->second);
			receive_message_event.set_message(message);
			new_events.push_back(receive_message_event);

//			cout << " send the message. It'll arive at " << new_schedule_time << endl;
//			cout << "  proc time " << processing_time << endl;
//			cout << "  transfer time " << transfer_time << endl;
//			cout << "  current time " << current_time << endl;

			// Update the timetable
			MyToolbox::timetable_.find(node_id_)->second = new_schedule_time; // update my available time
			for (map<unsigned int, StorageNode*>::iterator node_it = near_storage_nodes_.begin(); node_it != near_storage_nodes_.end(); node_it++) {
				MyToolbox::timetable_.find(node_it->first)->second = new_schedule_time;
			}

			// If I am here the queue was empty and it is still empty! I have to do nothing on the queue!
		}
	}
	return new_events;
}

vector<Event> SensorNode::re_send(Message* message) {
	vector<Event> new_events;

//	cout << " going to resend" << endl;

	unsigned int next_node_id = message->get_receiver_node_id();
	if (near_storage_nodes_.find(next_node_id) == near_storage_nodes_.end()) {	// my neighbor there is no longer
//		cout << " don't have my neighour" << endl;
		bool give_up = false;	// I could give up transmitting: it depends on the message type!
		switch (message->message_type_) {
		case Message::message_type_measure: {
			give_up = false;
			break;
		}
		default:
			give_up = true;
			break;
		}
		if (give_up) {	// do not try to tx this message, pass to the following one
//			cout << " I give up" << endl;
			event_queue_.pop();	// remove this event from the queue, I'm not going to execute it anymore
			if (event_queue_.empty()) {	// if the queue is now empty...
//				cout << " empty queue, do nothing" << endl;
				return new_events;	// return an empty vector, I don't have new events to schedule
			} else {
//				cout << " not empty queue, next message..." << endl;
				Message* new_message = event_queue_.front().get_message();
				return re_send(new_message);
			}
		} else {	// I cannot give up! Find another node to spread the message
//			cout << " I don't give up" << endl;
			next_node_id = get_random_neighbor();	// find another neighbour
			if (next_node_id == 0) {	// no more neighbours, I'm isolated. Postpone my delivery
//				cout << " I'm alone, finish here" << endl;
				MyTime schedule_time = MyToolbox::get_random_processing_time() + MyToolbox::get_tx_offset();
				event_queue_.front().set_time(schedule_time);	// change the schedule time of the message I am trying to send

				Event try_again_event(schedule_time, Event::sensor_try_to_send);
				try_again_event.set_agent(this);
				try_again_event.set_message(message);
				// FIXME: uncomment the following line to make the node try to send undefinitely. If the line is commented, if the node is left alone it does not generate new events ever
//				new_events.push_back(try_again_event);
				return new_events;	// return, there's no more I can do!
			}
			// if next_node_id is a valid id...
			message->set_receiver_node_id(next_node_id);	// set the new node id and go on
//			cout << " there's someone else, send to him" << endl;
		}
	}

	// If I arrive here I have a neighbour to whom I can try to send

	map<unsigned int, MyTime> timetable = MyToolbox::timetable_;  // download the timetable (I have to upload the updated version later!)
	MyTime current_time = MyToolbox::current_time_;  // current time of the system
	MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
	MyTime next_node_available_time = timetable.find(next_node_id)->second;  // time next_node gets free
	if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
//		cout << " me not available" << endl;
		MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
		try_again_event.set_agent(this);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
//		cout << " next node not available" << endl;
		MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
		Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
		try_again_event.set_agent(this);
		try_again_event.set_message(message);
		new_events.push_back(try_again_event);
		return new_events;
	} else {  // sender and receiver both idle, can send the message
//		cout << " everyone available" << endl;
		// Compute the message time
		MyTime processing_time = MyToolbox::get_random_processing_time();
		unsigned int num_total_bits = message->get_message_size();
		MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate_); // in nano-seconds
		MyTime new_schedule_time = current_time + processing_time + transfer_time;
		// Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
		Event::EventTypes this_event_type;
		switch (message->message_type_) {
		case Message::message_type_measure: {
			this_event_type = Event::storage_node_receive_measure;
			((Measure*)message)->measure_ = old_measure_data ^ new_measure_data;	// fill the measure with the most updated value
			old_measure_data = new_measure_data;	// update the old measure value
			data_collector->add_msr(((Measure*)message)->measure_id_, node_id_);
			break;
		}
		default:
			break;
		}
		Event receive_message_event(new_schedule_time, this_event_type);
		receive_message_event.set_agent(near_storage_nodes_.find(next_node_id)->second);
		receive_message_event.set_message(message);
		new_events.push_back(receive_message_event);

//		cout << " send the message. It'll arive at " << new_schedule_time << endl;
//		cout << "  proc time " << processing_time << endl;
//		cout << "  transfer time " << transfer_time << endl;
//		cout << "  current time " << current_time << endl;

		// Update the timetable
		MyToolbox::timetable_.find(node_id_)->second = new_schedule_time; // update my available time
		for (map<unsigned int, StorageNode*>::iterator node_it = near_storage_nodes_.begin(); node_it != near_storage_nodes_.end(); node_it++) {
			MyToolbox::timetable_.find(node_it->first)->second = new_schedule_time;
		}

		// Update the event_queue_
		event_queue_.pop();	// remove the current send event, now successful
		if (!event_queue_.empty()) {  // if there are other events in the queue -> for  sensor this should never occur!
//			cout << " queue not empty, schedule next message" << endl;
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

unsigned char SensorNode::get_measure_data() {
	return (unsigned char)(rand() % 256);
}
