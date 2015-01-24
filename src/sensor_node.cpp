#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>
#include <math.h>
#include <map>

#include "sensor_node.h"
#include "node_dispatcher.h"

SensorNode::SensorNode (int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
  measure_id_ = 0;
  first_generated_measure_ = true;
  measure_ = Measure();
}

void SensorNode::set_measure(Measure measure) {
	measure_ = measure;
}

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
  int next_node_index = rand() % near_storage_nodes.size();
  StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
  // cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(new_measure_value) << " to node " << next_node->get_node_id() << endl;
  my_supervisor_id_ = next_node->get_node_id(); // remember my supervisor

  // Create a measure object
  measure_ = Measure(xored_measure_value, measure_id_++, node_id_, first_generated_measure_ ? Measure::measure_type_new : Measure::measure_type_update);

  // // Compute the message time
  // // cout << "x1 = " << x_coord_ << ", y1 = " << y_coord_ << endl;
  // // cout << "x2 = " << next_node->get_x_coord() << ", y2 = " << next_node->get_y_coord() << endl;
  // double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  // cout << "distance " << distance << " meters" << endl;
  // MyTime propagation_time = (MyTime)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  // cout << "Propagation time: " << propagation_time << " ns" << endl;

  // MyTime processing_time = MyToolbox::get_random_processing_time();
  // cout << "Processing time: " << processing_time << " ns" << endl;

  // int num_total_bits =  MyToolbox::get_bits_for_phy_mac_overhead() + 
  //                       MyToolbox::get_bits_for_measure() +
  //                       MyToolbox::get_bits_for_measure() * 2 +
  //                       1 + // measure type
  //                       4 + // measure id
  //                       20; // other
  // cout << "Num bits: " << num_total_bits << " bits" << endl;
  // MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 9) / MyToolbox::get_channel_bit_rate_()); // in nano-seconds
  // cout << "Transfer time: " << transfer_time << " ns" << endl;

  // MyTime message_time = propagation_time + processing_time + transfer_time;
  // // cout << "Message time = " << propagation_time + processing_time << " ns" << endl;
  // // cout << "Message time = " << propagation_time + transfer_time << " ns" << endl;
  // // cout << "Message time = " << processing_time + transfer_time << " ns" << endl;
  // // cout << "Message time = " << processing_time + transfer_time + propagation_time << " ns" << endl;
  // cout << "Message time = " << message_time << " ns" << endl;

/*
    2 events:
    - generate new measure -> no problem: it can happen in any time
    - other node receives my measure / try to send again -> it depends by the channel!
      I must go through the timetable...
*/
  // new_events = send_measure(next_node);
  new_events = send(next_node, &measure_);
  unsigned long rand1 = rand();
  unsigned long rand2 = rand();
  unsigned long rand3 = rand();
  MyTime time_next_measure = rand1 * rand2 * rand3 % MAX_MEASURE_GENERATION_DELAY;
  MyTime minutes = time_next_measure / ((MyTime)1000000000 * 60);
  cout << "Next measure in " << minutes << " minutes" << endl;
  Event next_measure_event(time_next_measure, Event::sensor_generate_measure);
  next_measure_event.set_agent(this);
  new_events.push_back(next_measure_event);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // // Update timetable
  // /*  Behave like this sensor has just received a message contining the new measure.
  //     First of all: does this sensor have a pending event? A yet-to-deliver measure?
  //       I can see this through my Node::event_queue. If it is empty I have not, otherwise
  //       there is some event schedued before me! Then I just put in the bottom of the queue
  //       this new event!

  //     If there is no other event I can think to process it just right now.

  //     Am I free? The node could be busy! If it is busy schedule this event afterwards: V(this_sensor) + offset
  //     If I am free I have to sense if the next node is free. Busy? Then schedule the event "wake up and try again"
  //     at V(next_node) + offset.

  //     What if we both are idle?
  //     - I can transmit right now! Schedule the event "next_node receives my message" at curr_time + message
  //     - update time table: this_sensor, next_node and all my neighbours (my = of this sensor)
  //     - remove from my event_queue this event. There is another event afterwards?
  //       - No -> you're done!
  //       - Yes -> schedule it at t + message: right after I finish to transmit I execute the next event!
  //                Hopefully there will be no other concurrent events!
  //                All the other nodes will wait for an offset! Just this sensor will try to execute all
  //                of its events in a row (I think... :S )
  // */
  // if (!event_queue_.empty()) {  // already some pending event...
  //   cout << "At least an event in the event queue: enqueue this event too." << endl;
  //   // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
  //   // I will unfold it and I will build up a brand new event with its pieces and then I will set
  //   // a significant schedule time!
  //   Event event_to_enqueue(0, Event::sensor_try_to_send_measure);
  //   event_to_enqueue.set_agent(this);
  //   event_to_enqueue.set_message(&measure_);

  //   event_queue_.push(event_to_enqueue);

  //   // do not insert it in the new_events vector! This event is not going to be put in the main event list now!
  // } else {
  //   cout << "Empty event queue: can process this event!" << endl;
  //   map<int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
  //   MyTime current_time = MyToolbox::get_current_time();  // current time of the system
  //   MyTime my_available_time = timetable.find(node_id_)->second; // time this sensor gets free
  //   MyTime next_node_available_time = timetable.find(next_node->get_node_id())->second;  // time next_node gets free
  //   if (my_available_time > current_time) { // sensor already involved in a communication or surrounded by another communication
  //     cout << "Il sensore e' gia' occupato" << endl;
  //     MyTime new_schedule_time = my_available_time + MyToolbox::get_retransmission_offset();
  //     Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
  //     try_again_event.set_agent(this);
  //     try_again_event.set_message(&measure_);
  //     new_events.push_back(try_again_event);
  //   } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
  //     cout << "Next node e' gia' occupato" << endl;
  //     MyTime new_schedule_time = next_node_available_time + MyToolbox::get_retransmission_offset();
  //     Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
  //     try_again_event.set_agent(this);
  //     try_again_event.set_message(&measure_);
  //     new_events.push_back(try_again_event);
  //   } else {  // sender and receiver both idle, can send the message
  //     cout << "Posso trasmettere! Sensore e nodo liberi" << endl;
  //     // Schedule the new receive event
  //     MyTime new_schedule_time = current_time + message_time;
  //     Event receive_message_event(new_schedule_time, Event::storage_node_receive_measure);
  //     receive_message_event.set_agent(next_node);
  //     receive_message_event.set_message(&measure_);
  //     new_events.push_back(receive_message_event);

  //     // Update the timetable
  //     timetable.find(node_id_)->second = current_time + message_time; // update my available time
  //     for (int i = 0; i < near_storage_nodes.size(); i++) { // update the available time of all my neighbours
  //       timetable.find(near_storage_nodes.at(i)->get_node_id())->second = current_time + message_time;
  //     }
  //     MyToolbox::set_timetable(timetable);  // upload the updated timetable

  //     // Update the event_queue_
  //     if (!event_queue_.empty()) {  // if there are other events in the queue
  //       Event top_queue_event = event_queue_.front(); // the oldest event of the queue (the top one, the first)
  //       event_queue_.pop();
  //       Event popped_event(current_time + message_time, top_queue_event.get_event_type());
  //       popped_event.set_agent(this);
  //       popped_event.set_message(top_queue_event.get_message());
  //       new_events.push_back(popped_event);
  //     }
  //   }
  // }
  
  return new_events;
}

vector<Event> SensorNode::try_retx(Message* message, int next_node_id) {
  map<int, Node*>* nodes_map = NodeDispatcher::storage_nodes_map_ptr;
  StorageNode* next_node = (StorageNode*)nodes_map->find(next_node_id)->second;
  return send(next_node, message);
}

vector<Event> SensorNode::sensor_ping(int event_time){
    map<int, MyTime> timetable_ = MyToolbox::get_timetable();
     //my_supervisor_id_ = 0;  // DA TOGLIERE!!!!
    vector<Event> new_events;
    if (timetable_.find(my_supervisor_id_)->second > event_time){  //supervisor is awake
        Event new_event(timetable_.find(my_supervisor_id_)->second, Event::sensor_ping);
        new_event.set_agent(this);  
        new_events.push_back(new_event);
    }
    else {
        for(int i =0; i<near_storage_nodes.size(); i++ ){   //sensor try to ping just when supervisor wakes up
            if(near_storage_nodes.at(i)->get_node_id() == my_supervisor_id_){
                StorageNode *supervisior_node = (StorageNode*)near_storage_nodes.at(i);
                supervisior_node->set_supervision_map_(node_id_,event_time);     
            }
        }
        Event new_event(event_time+MyToolbox::get_ping_frequency(), Event::sensor_ping);   
        new_event.set_agent(this);
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
vector<Event> SensorNode::send_measure(StorageNode* next_node) {
  vector<Event> new_events;

  // Compute the message time
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  cout << "distance " << distance << " meters" << endl;
  MyTime propagation_time = (MyTime)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  cout << "Propagation time: " << propagation_time << " ns" << endl;

  MyTime processing_time = MyToolbox::get_random_processing_time();
  cout << "Processing time: " << processing_time << " ns" << endl;

  int number_of_measures = measure_.get_measure_type() == Measure::measure_type_new ? 1 : 2;
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
    Event event_to_enqueue(0, Event::sensor_try_to_send_measure);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(&measure_);

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
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(&measure_);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      cout << "Next node e' gia' occupato" << endl;
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_retransmission_offset();
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(&measure_);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      cout << "Posso trasmettere! I nodi sono liberi" << endl;
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      Event receive_message_event(new_schedule_time, Event::storage_node_receive_measure);
      receive_message_event.set_agent(next_node);
      receive_message_event.set_message(&measure_);
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

vector<Event> SensorNode::send(StorageNode* next_node, Message* message) {
  vector<Event> new_events;

  // Compute the message time
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  cout << "distance " << distance << " meters" << endl;
  MyTime propagation_time = (MyTime)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  cout << "Propagation time: " << propagation_time << " ns" << endl;

  MyTime processing_time = MyToolbox::get_random_processing_time();
  cout << "Processing time: " << processing_time << " ns" << endl;

  int number_of_measures = measure_.get_measure_type() == Measure::measure_type_new ? 1 : 2;
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

  if (!event_queue_.empty()) {  // already some pending event
    cout << "At least an event in the event queue: enqueue this event too." << endl;
    // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
    // I will unfold it and I will build up a brand new event with its pieces and then I will set
    // a significant schedule time!
    Event event_to_enqueue(0, Event::sensor_try_to_send_measure);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(message);

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
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      cout << "Next node e' gia' occupato" << endl;
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_retransmission_offset();
      Event try_again_event(new_schedule_time, Event::sensor_try_to_send_measure);
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      cout << "Posso trasmettere! I nodi sono liberi" << endl;
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      Event receive_message_event(new_schedule_time, Event::storage_node_receive_measure);
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
        cout << "Altri eventi in coda" << endl;
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
