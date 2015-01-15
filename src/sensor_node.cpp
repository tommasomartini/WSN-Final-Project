#include "sensor_node.h"
#include "my_toolbox.h"

#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>
#include <math.h>
#include <map>

SensorNode::SensorNode (int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {
  measure_id_ = 0;
  first_generated_measure_ = true;
  measure_ = Measure();
}

void SensorNode::set_measure(Measure measure) {
	measure_ = measure;
}

vector<Event> SensorNode::generate_measure() {
  unsigned char new_measure_value = (unsigned char)(rand() % 256);  // generate a random char
  unsigned char xored_measure_value = new_measure_value ^ measure_.get_measure();

  // choose the first storage_node randomly
  int next_node_index = rand() % near_storage_nodes.size();
  StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
  // cout << "Storage " << node_id_ << "-> forward message: " << static_cast<unsigned>(new_measure_value) << " to node " << next_node->get_node_id() << endl;
  my_supervisor_id_ = next_node->get_node_id();

  // Create a measure object
  measure_ = Measure(xored_measure_value, measure_id_++, node_id_, first_generated_measure_ ? Measure::new_measure : Measure::update_measure);

  // Update timetable
  // cout << "x1 = " << x_coord_ << ", y1 = " << y_coord_ << endl;
  // cout << "x2 = " << next_node->get_x_coord() << ", y2 = " << next_node->get_y_coord() << endl;
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  int propagation_time = (int)((distance / MyToolbox::LIGHT_SPEED) * pow(10, 9));   // in nano-seconds
  cout << propagation_time << endl;

  int processing_time = MyToolbox::get_random_processing_time();
  cout << processing_time << endl;

  int num_total_bits =  MyToolbox::get_bits_for_phy_mac_overhead() + 
                        MyToolbox::get_bits_for_measure() +
                        MyToolbox::get_bits_for_measure() * 2 +
                        1 + // measure type
                        4 + // measure id
                        20; // other
  cout << num_total_bits << endl;
  int transfer_time = (int)(num_total_bits * 1. * pow(10, 9) / MyToolbox::get_channel_bit_rate_()); // in nano-seconds
  cout << transfer_time << endl;

  unsigned int message_time = (unsigned int)(propagation_time + processing_time + message_time);
  cout << "Message time = " << message_time << endl;

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
