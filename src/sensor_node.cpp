#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>
#include <math.h>
#include <map>

#include "sensor_node.h"
// #include "my_toolbox.h"

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
  measure_ = Measure(xored_measure_value, measure_id_++, node_id_, first_generated_measure_ ? Measure::new_measure : Measure::update_measure);

  // Compute the message time
  // cout << "x1 = " << x_coord_ << ", y1 = " << y_coord_ << endl;
  // cout << "x2 = " << next_node->get_x_coord() << ", y2 = " << next_node->get_y_coord() << endl;
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  cout << "distance " << distance << " meters" << endl;
  MyTime propagation_time = (MyTime)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  cout << "Propagation time: " << propagation_time << " ns" << endl;

  MyTime processing_time = MyToolbox::get_random_processing_time();
  cout << "Processing time: " << processing_time << " ns" << endl;

  int num_total_bits =  MyToolbox::get_bits_for_phy_mac_overhead() + 
                        MyToolbox::get_bits_for_measure() +
                        MyToolbox::get_bits_for_measure() * 2 +
                        1 + // measure type
                        4 + // measure id
                        20; // other
  cout << "Num bits: " << num_total_bits << " bits" << endl;
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 9) / MyToolbox::get_channel_bit_rate_()); // in nano-seconds
  cout << "Transfer time: " << transfer_time << " ns" << endl;

  MyTime message_time = propagation_time + processing_time + transfer_time;
  // cout << "Message time = " << propagation_time + processing_time << " ns" << endl;
  // cout << "Message time = " << propagation_time + transfer_time << " ns" << endl;
  // cout << "Message time = " << processing_time + transfer_time << " ns" << endl;
  // cout << "Message time = " << processing_time + transfer_time + propagation_time << " ns" << endl;
  cout << "Message time = " << message_time << " ns" << endl;

/*
    2 events:
    - generate new measure -> no problem: it can happen in any time
    - other node receives my measure / try to send again -> it depends by the channel!
      I must go through the timetable...
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // Update timetable
  /*  Behave like this sensor has just received a message contining the new measure.
      First of all: does this sensor have a pending event? A yet-to-deliver measure?
        I can see this through my Node::event_queue. If it is empty I have not, otherwise
        there is some event schedued before me! Then I just put in the bottom of the queue
        this new event!

      If there is no other event I can think to process it just right now.

      Am I free? The node could be busy! If it is busy schedule this event afterwards: V(this_sensor) + offset
      If I am free I have to sense if the next node is free? Busy? Then schedule the event "wake up and try again"
      at V(next_node) + offset.

      What if we both are busy?
      - I can transmit right now! Schedule the event "next_node receives my message" at curr_time + message
      - update time table: this_sensor, next_node and all my neighbours (my = of this sensor)
      - remove from my event_queue this event. There is another event afterwards?
        - No -> you're done!
        - Yes -> schedule it at t + message: right after I finish to transmit I execute the next event!
                 Hopefully there will be no other concurrent events!
                 All the other nodes will wait for an offset! Just this sensor will try to execute all
                 of its events in a row (I think... :S )
  */
  if (!event_queue_.empty()) {  // already some pending event...
    cout << "At least an event in the event queue: enqueue this event too." << endl;
    // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
    // I will unfold it and I will build up a brand new event with its pieces and then I will set
    // a significant schedule time!
    Event event_to_enqueue(0, Event::sensor_try_to_send_measure);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(measure_);

    // do not insert it in the new_events vector! This event is not going to be put in the main event list now!
  } else {
    cout << "Empty event queue: can process this event!" << endl;

  }

  int free_time_next_node = MyToolbox::get_timetable().find(next_node->get_node_id())->second;
  if (free_time_next_node <= MyToolbox::get_current_time()) {   // next_node is free
    // no one of my neighbours can receive or transmit anything
    int near_node_id;
    for (int i = 0; i < near_storage_nodes.size(); i++) {
      near_node_id = near_storage_nodes.at(i)->get_node_id();
      MyToolbox::get_timetable().find(near_node_id)->second = MyToolbox::get_current_time() + message_time;
    }
  } else {

  }

  // Create the new event
  vector<Event> new_events;
  Event new_event(10, Event::spread_measure);
  new_event.set_agent(next_node);
  new_event.set_message(measure_);
  new_events.push_back(new_event);

  


  return new_events;
}

int SensorNode::fai_cose() {
  return 6;
}
