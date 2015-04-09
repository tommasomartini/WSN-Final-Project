#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>
#include <math.h>
#include <random>
#include <map>

#include "sensor_node.h"
#include "storage_node.h"
#include "measure.h"
#include "data_collector.h"


/**************************************
    Constructors
**************************************/
SensorNode::SensorNode(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
  measure_id_ = 0;
  first_generated_measure_ = true;
  measure_ = Measure();
  my_supervisor_id_ = -1;
}

/**************************************
    Setters
**************************************/
// void SensorNode::set_measure(Measure measure) {
// 	measure_ = measure;
// }

/**************************************
    Event execution methods
**************************************/
/*  Return two events: a new measure generation from the same node and the reception of the measure by a storage node
*/
vector<Event> SensorNode::generate_measure() {

  vector<Event> new_events; // create the new event  

  unsigned char new_measure_value = (unsigned char)(rand() % 256);  // generate a random char, i.e. a random measure
  // XOR the new measure with the old one. If this is the first measure, the old one is all-zero and the result of the XOR
  // will be the new measure itself. Otherwise I am ready to send the xored_measure in order to update the XOR measures
  // in the storage nodes cnontaining a measure from me
  unsigned char xored_measure_value = new_measure_value ^ measure_.get_measure(); 
  // choose the first storage_node randomly
  unsigned int next_node_index = rand() % near_storage_nodes_->size();
  map<unsigned int, Node*>::iterator near_nodes_iter = near_storage_nodes_->begin();
  for (int i = 0; i < next_node_index; i++) {
	  near_nodes_iter++;
  }
  StorageNode *next_node = (StorageNode*)near_nodes_iter->second;

  // Create a measure object
  measure_ = Measure(xored_measure_value, new_measure_id(), node_id_, first_generated_measure_ ? Measure::measure_type_new : Measure::measure_type_update);
  first_generated_measure_ = false;
  measure_.set_receiver_node_id(next_node->get_node_id());

  data_collector->add_msr(measure_.get_measure_id(), node_id_);
  /*
    2 events:
    - send the generated measure to another node. This may be successful or not, that is I can generate either a 
        storage_receive_measure event or a sensor_try_to_send_again_later event
    - with probability p I generate another measure (another sensor_generate_measure event), with probability 1-p the sensor
        breaks up and I genenerate a broken_sensor event.
  */
  new_events = send(next_node, &measure_);
  bool generate_another_measure = true;
  default_random_engine generator = MyToolbox::get_random_generator();
  bernoulli_distribution distribution(MyToolbox::sensor_failure_prob);
  if (distribution(generator)) {
    generate_another_measure = false;
  }
  uniform_int_distribution<int> unif_distrib(MyToolbox::max_measure_generation_delay / 2000, MyToolbox::max_measure_generation_delay / 1000);	// between 5ms and 10ms
  MyTime time_next_measure_or_failure = unif_distrib(generator) * 1000;
  if (generate_another_measure) {
	cout << "time next measure: " << time_next_measure_or_failure << endl;
    Event next_measure_event(time_next_measure_or_failure, Event::sensor_generate_measure);
    next_measure_event.set_agent(this);
    new_events.push_back(next_measure_event);
  } else {
	cout << "time next break: " << time_next_measure_or_failure << endl;
    Event failure_event(time_next_measure_or_failure, Event::broken_sensor);
    failure_event.set_agent(this);
    new_events.push_back(failure_event);
  }
  
  return new_events;
}

vector<Event> SensorNode::try_retx(Message* message, unsigned int next_node_id) {
  map<unsigned int, Node*>* nodes_map = MyToolbox::storage_nodes_map_ptr;
  StorageNode* next_node = (StorageNode*)nodes_map->find(next_node_id)->second;
  return send(next_node, message);
}

vector<Event> SensorNode::sensor_ping() {
  vector<Event> new_events;
  map<unsigned int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
  MyTime current_time = MyToolbox::get_current_time();  // current time of the system
  MyTime my_available_time = timetable.find(node_id_)->second; // time this sensor gets free
  MyTime next_node_available_time = timetable.find(my_supervisor_id_)->second;  // time next_node gets free
  if (my_available_time > current_time) { // node already involved in a communication or surrounded by another communication
    MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset_ping();
    Event try_again_event(new_schedule_time, Event::sensor_ping);
    try_again_event.set_agent(this);
    new_events.push_back(try_again_event);
  } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
    MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset_ping();
    Event try_again_event(new_schedule_time, Event::sensor_ping);
    try_again_event.set_agent(this);
    new_events.push_back(try_again_event);
  } else {  // sender and receiver both idle, can send the message
	StorageNode *supervisior_node = (StorageNode*)near_storage_nodes_->at(my_supervisor_id_);
	supervisior_node->set_supervision_map_(node_id_, MyToolbox::get_current_time());
	Event new_event(MyToolbox::get_current_time() + MyToolbox::ping_frequency, Event::broken_sensor);	// FIXME for debug only
//	Event new_event(MyToolbox::get_current_time() + MyToolbox::ping_frequency, Event::sensor_ping);
	new_event.set_agent(this);
	new_events.push_back(new_event);
  }
  return new_events;
}

vector<Event> SensorNode::sensor_ping(int event_time) {
  map<unsigned int, MyTime> timetable_ = MyToolbox::get_timetable();
  //my_supervisor_id_ = 0;  // DA TOGLIERE!!!!
  cout<<"il mio supervisior è "<<my_supervisor_id_;
  vector<Event> new_events;
  if (timetable_.find(my_supervisor_id_)->second > event_time){  //supervisor is awake
    cout<<"nodo impegnato!"<<endl;
    Event new_event(timetable_.find(my_supervisor_id_)->second + MyToolbox::get_tx_offset_ping(), Event::sensor_ping);
    new_event.set_agent(this);  
    new_events.push_back(new_event);
  }
  else {
    // TODO: questo viene molto meglio con una mappa!
    for(int i =0; i<near_storage_nodes.size(); i++ ){   //sensor try to ping just when supervisor wakes up
      if(near_storage_nodes.at(i)->get_node_id() == my_supervisor_id_){
        StorageNode *supervisior_node = (StorageNode*)near_storage_nodes.at(i);
        supervisior_node->set_supervision_map_(node_id_,event_time);     
      }
    }
    cout<<"nodo libero! prossimo ping tra "<<MyToolbox::ping_frequency;

    Event new_event(event_time + MyToolbox::ping_frequency, Event::sensor_ping);   
    new_event.set_agent(this);
    new_events.push_back(new_event);
  }
  return new_events;
}

void SensorNode::set_supervisor() {
  // choose my supervisor as the first node in my list (does not matter how I choose it)
  my_supervisor_id_ = near_storage_nodes_->begin()->first;
}




/**************************************
    Private methods
**************************************/

int SensorNode::new_measure_id() {
  return measure_id_++ % MyToolbox::num_bits_for_measure_id;
}

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

vector<Event> SensorNode::send(StorageNode* next_node, Message* message) {
  vector<Event> new_events;

  // Compute the message time
  MyTime processing_time = MyToolbox::mean_processing_time;
  int num_total_bits = message->get_message_size();
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate); // in nano-seconds
  MyTime message_time = processing_time + transfer_time;

  if (!event_queue_.empty()) {  // already some pending event
    cout << "Coda eventi NON vuota" << endl;
    /* I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
       I will unfold it and I will build up a brand new event with its pieces and then I will set
       a significant schedule time!
    */
    Event event_to_enqueue(0, Event::sensor_try_to_send);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(message);
    event_queue_.push(event_to_enqueue);

    // do not insert it in the new_events vector! This event is not going to be put in the main event list now!

  } else {  // no pending events
    map<unsigned int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
    MyTime current_time = MyToolbox::get_current_time();  // current time of the system
    cout << "Current time " << current_time << endl;
    MyTime my_available_time = timetable.find(node_id_)->second; // time this sensor gets free
    cout << "My available time " << my_available_time * 1. / 1000000 << endl;
    MyTime next_node_available_time = timetable.find(next_node->get_node_id())->second;  // time next_node gets free
    cout << "Next node available time " << next_node_available_time * 1. / 1000000 << endl;
    if (my_available_time > current_time) { // node already involved in a communication or surrounded by another communication
      cout << "non posso inviare: io non sono libero" << endl;
      MyTime offset;
      switch (message->message_type_) {
        case Message::message_type_measure: {
          offset = MyToolbox::get_tx_offset();
          break;
        }
        case Message::message_type_ping: {
          cout << "Errore! Non dovrei mai entrare qui (sensor_node.cpp, case message_type_ping first if)" << endl;
          offset = MyToolbox::get_tx_offset_ping();
          break;
        }
        default:
          break;
      }
      MyTime new_schedule_time = my_available_time + offset;
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
      cout << "prova a inviare al tempo " << new_schedule_time << endl;
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      cout << "non posso inviare: io sono libero, ma non il mio RX" << endl;
      MyTime offset;
      switch (message->message_type_) {
        case Message::message_type_measure: {
          offset = MyToolbox::get_tx_offset();
          break;
        }
        case Message::message_type_ping: {
          cout << "Errore! Non dovrei mai entrare qui (sensor_node.cpp, case message_type_ping second if)" << endl;
          offset = MyToolbox::get_tx_offset_ping();
          break;
        }
        default:
          break;
      }
      MyTime new_schedule_time = next_node_available_time + offset;
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send);
      cout << "prova a inviare al tempo " << new_schedule_time << endl;
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      cout << "posso inviare" << endl;
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      // Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
      Event::EventTypes this_event_type;
      switch (message->message_type_) {
        case Message::message_type_measure: {
          this_event_type = Event::storage_node_receive_measure;
          break;
        }
        case Message::message_type_ping: {
          cout << "Errore! Non dovrei mai entrare qui (sensor_node.cpp, case message_type_ping else)" << endl;
          this_event_type = Event::sensor_ping;
          break;
        }
        default:
          break;
      }
      Event receive_message_event(new_schedule_time, this_event_type);
      cout << "riceve a " << new_schedule_time << endl;
      receive_message_event.set_agent(next_node);
      receive_message_event.set_message(message);
      new_events.push_back(receive_message_event);

      // Update the timetable
      timetable.find(node_id_)->second = current_time + message_time; // update my available time
      for (map<unsigned int, Node*>::iterator sns_iter = near_sensors_->begin(); sns_iter != near_sensors_->end(); sns_iter++) {
    	timetable.find(sns_iter->first)->second = current_time + message_time;
      }
      for (map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin(); node_iter != near_storage_nodes_->end(); node_iter++) {
    	timetable.find(node_iter->first)->second = current_time + message_time;
      }
      MyToolbox::set_timetable(timetable);  // upload the updated timetable

      // Update the event_queue_
      if (!event_queue_.empty()) {  // if there are other events in the queue
        Event top_queue_event = event_queue_.front(); // the oldest event of the queue (the top one, the first)
        event_queue_.pop(); // remove the oldest event form the queue
        Event popped_event(current_time + message_time, top_queue_event.get_event_type());  // create a brand new event using the popped one, seting now  valid schedule time
        popped_event.set_agent(this);
        popped_event.set_message(top_queue_event.get_message());
        new_events.push_back(popped_event); // schedule the next event
      }
    }
  }
  return new_events;
}

void SensorNode::set_my_supervisor(unsigned int supervisior){
    my_supervisor_id_=supervisior;
}
