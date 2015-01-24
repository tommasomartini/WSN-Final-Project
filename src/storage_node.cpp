#include <iostream>
#include <map>
#include <algorithm>    // std::max
#include <iostream>
#include <math.h>

#include "storage_node.h"
#include "node_dispatcher.h"
  
using namespace std;


vector<Event> StorageNode:: manage_measure(Measure* measure) {
  vector<Event> new_events;

  int source_id = measure->get_source_sensor_id();  // measure from sensor source_id
  cout << "Storage " << node_id_ << "-> new measure from sensor " << source_id << endl;
  if (measure->get_measure_type() == Measure::measure_type_new) { // new measure from a new sensor: accept it wp d/k
    cout << "New measure from a sensor" << endl;
    /*
      Sampling interval must be larger smaller than the smallest interval probability.
      I have only 2 intervals: [0, d/k] and [d/k, 1]. Call the smaller min_int = min(d/k, (1 - d/k))
      Divide the interval [0, 1] in M intervals with length 1/M.

      1/M < min_int => M > 1 / min_int = 1 / min(d/k, (1 - d/k)) = max(1 / (d/k), 1 / (1 - d/k)) = 
      = max(k/d, k/(k - d)).

      Then we can take M = 10 * max(k/d, k/(k - d))

      Choose randomly one element between 0 and M - 1 with "rand() % M".
      Divide it by (M - 1) to normaliz between 0 and 1
    */
    int k = MyToolbox::get_k();
    int d = LT_degree_;
    double prob = 1;
    printf("Storage %i-> k=%i, d=%i, prob=%3.2f\n", node_id_, k, d, prob);
    if (d != k) { // if d == k I keep all the incoming measures and prob remains 1
      int M = 10 * max(k/d, k/(k - d)); // if d == k this gives a zero denominator
      prob = (rand() % M) / (double)(M - 1);
    }
    printf("Storage %i-> k=%i, d=%i, prob=%3.2f\n", node_id_, k, d, prob);
    // accept the new msg with probability d/k
    if (prob <= LT_degree_ / k) { // accept it!
      xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
      cout << "Storage " << node_id_ << "-> accept new msg from sensor " << source_id << ". Now stores " << static_cast<unsigned>(xored_measure_) << endl;
      last_measures_.insert(pair<int, int>(source_id, measure->get_measure_id()));  // save this measure
    }
  } else if (measure->get_measure_type() == Measure::measure_type_update) { // update measure from a sensor: always accept it, if I'm collecting this sensor's measures
    cout << "Update measure from a sensor" << endl;
    if (last_measures_.find(source_id) != last_measures_.end()) {  // already received a msg from this sensor
      xored_measure_ = xored_measure_ ^ measure->get_measure();  // save the new xored message
      cout << "Storage " << node_id_ << "-> accept update from sensor " << source_id << ". Now stores " << static_cast<unsigned>(xored_measure_) << endl;
      last_measures_.find(source_id)->second = measure->get_measure_id();  // save this measure // save this message as the last received message from sensor source_id
    }
  } else {
    cout << "Storage " << node_id_ << "-> Error! Neither new_msg nor update " << endl;
  }

  measure->increase_hop_counter();
  int hop_limit = MyToolbox::get_max_msg_hops();
  cout << "Hop counter from my toolbox: " << hop_limit << endl;
  if (measure->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
    int next_node_index = rand() % near_storage_nodes.size();
    StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
    cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(measure->get_measure()) << " to node " << next_node->get_node_id() << endl;
    new_events = send_measure(next_node, measure);
  } else {
    cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(measure->get_measure()) << " is not to be forwarded" << endl;
  }

  return new_events;
} 

vector<Event> StorageNode::try_retx_measure(Measure* measure, int next_node_id) {
  map<int, Node*>* nodes_map = NodeDispatcher::storage_nodes_map_ptr;
  StorageNode* next_node = (StorageNode*)nodes_map->find(next_node_id)->second;
  return send_measure(next_node, measure);
}

void StorageNode::set_supervision_map_(int sensor_id, int new_time){
    if (supervisioned_map_.find(sensor_id) == supervisioned_map_.end() ){
        supervisioned_map_.insert(std::pair<int, int>(sensor_id,new_time));
    }
    else
      supervisioned_map_.find(sensor_id)->second = new_time;
}

vector<Event> StorageNode::check_sensors(int event_time){       //assumption: sensors always wake up
    vector<Event> new_events;
    int expired_sensors[supervisioned_map_.size()];
    int i = 0;
    bool new_blacklist_element = false;

    for (auto& x: supervisioned_map_){
        if(x.second +(3*MyToolbox::get_ping_frequency())< event_time){  // don't ping in last 3*ping_frequency
            my_blacklist_.push_back( x.first);
            new_blacklist_element = true;
            expired_sensors[i] =  x.first;
            i++;    
        }
    }
    for(int j=0; j<i; j++)
        supervisioned_map_.erase(supervisioned_map_.find(expired_sensors[j])); 
   
    if (new_blacklist_element == true){     //make event for spread blacklist
        int* ex_sensors=expired_sensors;
        BlacklistMessage* list = new BlacklistMessage(ex_sensors,i);
        int next_node_index = rand() % near_storage_nodes.size();
        StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
        Event new_event(event_time, Event::blacklist_sensor); //to add propagation time?
        new_event.set_agent(next_node);   
        new_event.set_message(list);
        new_events.push_back(new_event);
    }
    if(supervisioned_map_.size()!=0){
        Event new_event(event_time + MyToolbox::get_check_sensors_frequency_(), Event::check_sensors); 
        new_event.set_agent(this);   
        new_events.push_back(new_event);
    }
    
    return new_events;
}

vector<Event> StorageNode::spread_blacklist(int event_time, BlacklistMessage* list){
    vector<Event> new_events;
    for(int i=0; i<list->get_length(); i++){
        if ( last_measures_.find(list->get_id_list()[i]) != last_measures_.end() )
            my_blacklist_.push_back(list->get_id_list()[i]);
    }  
    int hop_limit = MyToolbox::get_max_msg_hops();
    if (list->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
        list->increase_hop_counter();
        int next_node_index = rand() % near_storage_nodes.size();
        StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
        Event new_event(event_time, Event::blacklist_sensor); // event_time has to consider timetable + backoff time (+ propagation time??)
        new_event.set_agent(next_node);   
        new_event.set_message(list);
        new_events.push_back(new_event);
    }
    
    return new_events;
}

vector<Event> StorageNode::remove_mesure(OutdatedMeasure* message_to_remove){
    vector<Event> new_events;
    map<int,unsigned char> outdated_measure = message_to_remove->get_outdaded_measure();
    for (map<int, unsigned char>::iterator it=outdated_measure.begin(); it!=outdated_measure.end(); ++it){
        if (last_measures_.find(it->first) != last_measures_.end()){
            xored_measure_ = xored_measure_ ^ it->second;
            last_measures_.erase(last_measures_.find(it->first)); 
           }
    }
    int hop_limit = MyToolbox::get_max_msg_hops();
    if (message_to_remove->get_hop_counter() < hop_limit) {  // the message has to be forwarded again
      message_to_remove->increase_hop_counter();
      int next_node_index = rand() % near_storage_nodes.size();
      StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
      Event new_event(10, Event::remove_measure); // to set event time!!!!!!!
      new_event.set_agent(next_node);   
      new_event.set_message(message_to_remove);
      new_events.push_back(new_event);
  }
  return new_events;
}




///////////////////////////////////////////////////////////////////////////////
// Private methods

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
vector<Event> StorageNode::send_measure(StorageNode* next_node, Measure* measure) {
  vector<Event> new_events;

  // Compute the message time
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  cout << "distance " << distance << " meters" << endl;
  MyTime propagation_time = (MyTime)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  cout << "Propagation time: " << propagation_time << " ns" << endl;

  MyTime processing_time = MyToolbox::get_random_processing_time();
  cout << "Processing time: " << processing_time << " ns" << endl;

  int number_of_measures = measure->get_measure_type() == Measure::measure_type_new ? 1 : 2;
  int num_total_bits =  MyToolbox::get_bits_for_phy_mac_overhead() + 
                        MyToolbox::get_bits_for_measure() +
                        MyToolbox::get_bits_for_measure() * number_of_measures +
                        1 + // measure type
                        4 + // measure id
                        20; // other
  cout << "Num bits: " << num_total_bits << " bits" << endl;
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 9) / MyToolbox::get_channel_bit_rate_()); // in nano-seconds
  cout << "Transfer time: " << transfer_time << " ns" << endl;

  MyTime message_time = propagation_time + processing_time + transfer_time;
  cout << "Message time = " << message_time << " ns" << endl;

  // Update the timetable
  if (!event_queue_.empty()) {  // already some pending event
    cout << "At least an event in the event queue: enqueue this event too." << endl;
    // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
    // I will unfold it and I will build up a brand new event with its pieces and then I will set
    // a significant schedule time!
    Event event_to_enqueue(0, Event::storage_node_try_to_send_measure);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(measure);

    event_queue_.push(event_to_enqueue);

    // do not insert it in the new_events vector! This event is not going to be put in the main event list now!
  } else {  // no pending events
    cout << "Empty event queue: can process this event!" << endl;
    map<int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
    MyTime current_time = MyToolbox::get_current_time();  // current time of the system
    MyTime my_available_time = timetable.find(node_id_)->second; // time this sensor gets free
    MyTime next_node_available_time = timetable.find(next_node->get_node_id())->second;  // time next_node gets free
    if (my_available_time > current_time) { // node already involved in a communication or surrounded by another communication
      cout << "Il sensore e' gia' occupato" << endl;
      MyTime new_schedule_time = my_available_time + MyToolbox::get_retransmission_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(measure);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      cout << "Next node e' gia' occupato" << endl;
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_retransmission_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(measure);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      cout << "Posso trasmettere! I nodi sono liberi" << endl;
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      Event receive_message_event(new_schedule_time, Event::storage_node_receive_measure);
      receive_message_event.set_agent(next_node);
      receive_message_event.set_message(measure);
      new_events.push_back(receive_message_event);

      // Update the timetable
      timetable.find(node_id_)->second = current_time + message_time; // update my available time
      for (int i = 0; i < near_storage_nodes.size(); i++) { // update the available time of all my neighbours
        timetable.find(near_storage_nodes.at(i)->get_node_id())->second = current_time + message_time;
      }
      MyToolbox::set_timetable(timetable);  // upload the updated timetable

      // Update the event_queue_
      if (!event_queue_.empty()) {  // if there are other events in the queue
        cout << "Altri eventi in coda" << endl;
        Event top_queue_event = event_queue_.front(); // the oldest event of the queue (the top one, the first)
        event_queue_.pop(); // remove the oldest event frrm the queue
        Event popped_event(current_time + message_time, top_queue_event.get_event_type());  // create a brand new event using the popped one, seting now  valid schedule time
        popped_event.set_agent(this);
        popped_event.set_message(top_queue_event.get_message());
        new_events.push_back(popped_event); // schedule the next event
      }
    }

     
    }
  
  return new_events;
}
  
