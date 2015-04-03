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
  
using namespace std;

/**************************************
    Event execution methods
**************************************/
/*  Receive a measure either from a sensor or from another cache node
*/
vector<Event> StorageNode::receive_measure(Measure* measure) {
  cout << "Misura ricevuta da me, che sono " << node_id_ << ", del sensore " << measure->get_source_sensor_id() << endl;
  vector<Event> new_events;
  unsigned int source_id = measure->get_source_sensor_id();  // measure from sensor source_id
  if (measure->get_measure_type() == Measure::measure_type_new) { // new measure from a new sensor: accept it wp d/k
    cout << "Misura nuova " << endl;
    if (last_measures_.find(source_id) == last_measures_.end()) {  // not yet received a msg from this sensor
      int k = MyToolbox::num_sensors;
      int d = LT_degree_;
      default_random_engine gen = MyToolbox::get_random_generator();
      bernoulli_distribution bernoulli_distrib(d / k);
      // accept the new msg with probability d/k
//      if (bernoulli_distrib(gen)) { // accept it!	//TODO pee debug accetto sempre!
      if (true) {
        xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
        last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
        cout << "Mi prendo la misura. Ora salvo: " << int(xored_measure_) << endl;
        cout << "last measures contiene " << last_measures_.size() << " misure" << endl;
      }

      /*
        Sampling interval must be larger smaller than the smallest interval probability.
        I have only 2 intervals: [0, d/k] and [d/k, 1]. Call the smaller min_int = min(d/k, (1 - d/k))
        Divide the interval [0, 1] in M intervals with length 1/M.

        1/M < min_int => M > 1 / min_int = 1 / min(d/k, (1 - d/k)) = max(1 / (d/k), 1 / (1 - d/k)) = 
        = max(k/d, k/(k - d)).

        Then we can take M = 10 * max(k/d, k/(k - d))

        Choose randomly one element between 0 and M - 1 with "rand() % M".
        Divide it by (M - 1) to normaliz between 0 and 1
      
      int k = MyToolbox::num_sensors;
      int d = LT_degree_;
      double prob = 1;
      if (d != k) { // if d == k I keep all the incoming measures and prob remains 1
        int M = 10 * max(k/d, k/(k - d)); // if d == k this gives a zero denominator
        prob = (rand() % M) / (double)(M - 1);
      }
      // accept the new msg with probability d/k
      if (prob <= LT_degree_ / k) { // accept it!
        cout << "Mi prendo la misura!" << endl;
        xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
        last_measures_.insert(pair<unsigned int, unsigned int>(source_id, measure->get_measure_id()));  // save this measure
      }
      */
    }
  } else if (measure->get_measure_type() == Measure::measure_type_update) { // update measure from a sensor: always accept it, if I'm collecting this sensor's measures
    cout << "Misura update" << endl;
    if (last_measures_.find(source_id) != last_measures_.end()) {  // already received a msg from this sensor
      cout << "Gia ricevuta misura da questo nodo" << endl;
      xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
      last_measures_.find(source_id)->second = measure->get_measure_id();  // save this measure // save this message as the last received message from sensor source_id
    }
  }

  measure->increase_hop_counter();
  int hop_limit = MyToolbox::max_num_hops;
  cout << "Hop hop_limit " << hop_limit << endl;
  cout << "Hop counter " << measure->get_hop_counter() << endl;
  if (measure->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
    int next_node_index = rand() % near_storage_nodes_->size();
    map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
    for (int i = 0; i < next_node_index; i++) {
    	node_iter++;
    }
    StorageNode *next_node = (StorageNode*)node_iter->second;
    cout << "Propago a " << next_node->get_node_id() << endl;
    measure->set_receiver_node_id(next_node->get_node_id());
    new_events = send(next_node, measure);
  }

  return new_events;
} 

/*  I have already tried to send a message to someone, but I failed. Now I try again!
*/
vector<Event> StorageNode::try_retx(Message* message, int next_node_id) {
  map<unsigned int, Node*>* nodes_map = MyToolbox::storage_nodes_map_ptr;
  StorageNode* next_node = (StorageNode*)nodes_map->find(next_node_id)->second;
  return send(next_node, message);
}

/*  A sensor is telling me it is alive
*/
void StorageNode::set_supervision_map_(int sensor_id, int new_time){
  // If it is the first time I receive a ping from a sensor it means that that sensor wants me to be his supervisor. I save it in my supervisor map
    if (supervisioned_map_.find(sensor_id) == supervisioned_map_.end()){
        supervisioned_map_.insert(std::pair<int, int>(sensor_id, new_time));
    }
    else {
      supervisioned_map_.find(sensor_id)->second = new_time;
    }
}

/*  A user asks me to send him my data
*/
vector<Event> StorageNode::receive_user_request(unsigned int sender_user_id) {
  vector<Event> new_events;
  vector<unsigned int> my_sensor_ids;
  for (map<unsigned int, unsigned int>::iterator it = last_measures_.begin(); it != last_measures_.end(); it++) {
    my_sensor_ids.push_back(it->first);
  }
  StorageNodeMessage msg(xored_measure_, my_sensor_ids);
  Node* next_node = MyToolbox::users_map_ptr->find(sender_user_id)->second;
  msg.set_receiver_node_id(next_node->get_node_id()); // should be equal to sender_user_id
  new_events = send(next_node, &msg);
  return new_events;
}

/*  Occasionally I check if the sensors I am supervising are OK
*/
vector<Event> StorageNode::check_sensors() {
  vector<Event> new_events;
  vector<unsigned int> expired_sensors;	// list of sensor ids which didn't answer for 3 times in a row

  for (auto& x : supervisioned_map_){	// for each sensor in my supervised list...
    if(x.second + (3 * MyToolbox::ping_frequency) < MyToolbox::get_current_time()){  // ...if it didn't answer for  times...
      my_blacklist_.push_back(x.first);	// ...put it in my blacklist...
      expired_sensors.push_back(x.first);	// ...and in a list I use to update the structures
    }
  }
  // remove from the supervisioned_map the dead sensors
  for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) {
    supervisioned_map_.erase(supervisioned_map_.find(*it));
  }

  if (expired_sensors.size() > 0) {	// if there are some expired sensors I have to spread this info
//    BlacklistMessage* list = new BlacklistMessage(&expired_sensors);
    BlacklistMessage* list = new BlacklistMessage(expired_sensors);
    int next_node_index = rand() % near_storage_nodes_->size();
    map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
    for (int i = 0; i < next_node_index; i++) {
      node_iter++;
    }
    StorageNode *next_node = (StorageNode*)node_iter->second;
    list->set_receiver_node_id(next_node->get_node_id());
    list->message_type_= Message::message_type_blacklist;
    new_events = send(next_node, list);
  }

  Event new_event(MyToolbox::get_current_time() + MyToolbox::check_sensors_frequency, Event::check_sensors);
  new_event.set_agent(this);   
  new_events.push_back(new_event);

  return new_events;
}

/*  I received a blacklist message: this message contains the measure I have to remove
*/
vector<Event> StorageNode::spread_blacklist(BlacklistMessage* list) {
  vector<Event> new_events;
  vector<unsigned int> expired_sensors = list->get_id_list3();
  for (vector<unsigned int>::iterator it = expired_sensors.begin(); it != expired_sensors.end(); it++) { 	// for each id in the blacklist...
    bool msr_from_this_sns = last_measures_.find(*it) != last_measures_.end();	// ...if I have a measure from that sensor...
    bool not_yet_in_my_blacklist = find(my_blacklist_.begin(), my_blacklist_.end(), *it) == my_blacklist_.end();	// ...and this sensor is not yet in my blacklist...
	if (msr_from_this_sns && not_yet_in_my_blacklist) {
      my_blacklist_.push_back(*it);	// ...put its id in my backlist too
    }
  }
  int hop_limit = MyToolbox::max_num_hops;
  if (list->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
    list->increase_hop_counter();
    int next_node_index = rand() % near_storage_nodes_->size();
    map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
    for (int i = 0; i < next_node_index; i++) {
      node_iter++;
    }
    StorageNode *next_node = (StorageNode*)node_iter->second;
    list->set_receiver_node_id(next_node->get_node_id());
    list->message_type_ = Message::message_type_blacklist;
    new_events = send(next_node, list);
  }
  return new_events;
}

/*  A user informs me about what measures are obsolete
*/
vector<Event> StorageNode::remove_mesure(OutdatedMeasure* message_to_remove){
  vector<Event> new_events;
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
  int hop_limit = MyToolbox::max_num_hops;
  if (message_to_remove->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
    message_to_remove->increase_hop_counter();
    unsigned int next_node_index = rand() % near_storage_nodes_->size();
    map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
    for (int i = 0; i < next_node_index; i++) {
      node_iter++;
    }
    StorageNode *next_node = (StorageNode*)node_iter->second;
//    new_events = send(next_node, message_to_remove);	// my choice is not to flood the network with heavy messages
  }
  return new_events;
}


/**************************************
    Private methods
**************************************/

/*  First of all: does this sensor have a pending event? A yet-to-deliver measure?
        I can see this through my Node::event_queue. If it is empty I have not, otherwise
        there is some event schedued before me! Then I just put in the bottom of the queue
        this new event!

      If there is no other event I can think to process it just right now.

      Am I free? The node could be busy! If it is busy schedule this event afterwards: V(this_sensor) + offset
      If I am free I have to sense if the next node is free. Busy? Then schedule the event "wake up and try again"
      at V(next_node) + offset.

      What if we both are idle?
      - I can transmit right now! Schedule the event "next_node receives my message" at curr_time + message
      - update time table: this_sensor, next_node and all my neighbours (my = of this sensor)
      - remove from my event_queue this event. There is another event afterwards?
        - No -> you're done!
        - Yes -> schedule it at t + message: right after I finish to transmit I execute the next event!
                 Hopefully there will be no other concurrent events!
                 All the other nodes will wait for an offset! Just this sensor will try to execute all
                 of its events in a row (I think... :S )
*/
vector<Event> StorageNode::send(Node* next_node, Message* message) {
  vector<Event> new_events;

  // Compute the message time
  MyTime processing_time = MyToolbox::get_random_processing_time();
  unsigned int num_total_bits = message->get_message_size();
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate); // in nano-seconds
  MyTime message_time = /*processing_time + transfer_time;*/ 5;	// FIXME debug
  
  // Update the timetable
  if (!event_queue_.empty()) {  // already some pending event
    // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
    // I will unfold it and I will build up a brand new event with its pieces and then I will set
    // a significant schedule time!
    Event event_to_enqueue(0, Event::storage_node_try_to_send);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(message);
    event_queue_.push(event_to_enqueue);

    // do not insert it in the new_events vector! This event is not going to be put in the main event list now!
  } else {  // no pending events
    map<unsigned int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
    MyTime current_time = MyToolbox::get_current_time();  // current time of the system
    MyTime my_available_time = timetable.find(node_id_)->second; // time this node gets free (ME)
    MyTime next_node_available_time = timetable.find(next_node->get_node_id())->second;  // time next_node gets free
    if (my_available_time > current_time) { // this node is already involved in a communication or surrounded by another communication
      MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
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
        default:
          break;
      }
      Event receive_message_event(new_schedule_time, this_event_type);
      receive_message_event.set_agent(next_node);
      receive_message_event.set_message(message);
      new_events.push_back(receive_message_event);

      // Update the timetable
      timetable.find(node_id_)->second = current_time + message_time; // update my available time
      for (int i = 0; i < near_storage_nodes.size(); i++) { // update the available time of all my neighbours
        timetable.find(near_storage_nodes.at(i)->get_node_id())->second = current_time + message_time;
      }
      MyToolbox::set_timetable(timetable);  // upload the updated timetable

      // Update the event_queue_
      if (!event_queue_.empty()) {  // if there are other events in the queue
        cout << "Coda eventi NON vuota" << endl;
        Event top_queue_event = event_queue_.front(); // the oldest event of the queue (the top one, the first)
        event_queue_.pop(); // remove the oldest event frrm the queue
        Event popped_event(current_time + message_time + MyToolbox::get_tx_offset(), top_queue_event.get_event_type());  // create a brand new event using the popped one, seting now  valid schedule time
        popped_event.set_agent(this);
        popped_event.set_message(top_queue_event.get_message());
        new_events.push_back(popped_event); // schedule the next event
      }
    }
  }

  return new_events;
}
