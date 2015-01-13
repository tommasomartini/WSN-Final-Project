#include "sensor_node.h"
#include "my_toolbox.h"

#include <stdlib.h>     /* srand, rand */
#include <vector>

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

  // Create the new event
  vector<Event> new_events;
  Event new_event(10, Event::spread_measure);
  new_event.set_agent(next_node);
  new_event.set_message(measure_);
  new_events.push_back(new_event);

  // Update timetable


  return new_events;
}
