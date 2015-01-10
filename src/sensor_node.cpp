#include <thread>
#include <iostream>

#include "sensor_node.h"
#include "storage_node.h"
#include "message.h"

void SensorNode::set_message(Message message) {
	message_ = message;
}

void SensorNode::activate_sensor(const Interval &interval, const Timeout &timeout) {
  running_ = true;

  thread_ = thread([=]() { // lambda function. "[]() mutable -> T{}".  [=] significa che tutte le variabili visibili (cioe' dentro la funzione start?) sono accessibili per valore- 
    while (running_ == true) {
      this_thread::sleep_for(interval);   // wait for "interval" milliseconds...
      // timeout();  // ...and then execute function timeout()
      generate_data();
    }
  });
  
  // th.detach();    // Tom: lascio che il thread vada per vita propria
}

void SensorNode::activate_sensor() {
  running_ = true;
  cout << "Sensor " << node_id_ << "-> ON" << endl;

  generate_data();

  thread_ = thread([=]() { // lambda function. "[]() mutable -> T{}".  [=] significa che tutte le variabili visibili (cioe' dentro la funzione start?) sono accessibili per valore- 
    while (running_ == true) {
      this_thread::sleep_for(chrono::milliseconds(200000));   // wait for "interval" milliseconds...
      // timeout();  // ...and then execute function timeout()
      generate_data();
    }
  });
  
  // th.detach();    // Tom: lascio che il thread vada per vita propria
}

void SensorNode::stop_sensor() {
  running_ = false;
  cout << "Sensor " << node_id_ << "-> closing..." << endl;
  thread_.join(); // waits for the thread to end
  cout << "Sensor " << node_id_ << "-> OFF" << endl;
}

Message SensorNode::generate_data() {

  unsigned char new_measure = (unsigned char)(rand() % 256);  // generate a random char

  Message new_message(new_measure);
  new_message.set_source_sensor_id(node_id_);

  int next_node_index = rand() % near_storage_nodes.size();
  StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
  cout << "Sensor " << node_id_ << "-> sends " << static_cast<unsigned>(new_measure) << " to node " << next_node->get_node_id() << endl;
  next_node->manage_message(new_message);
  return new_message;

  // return Message();
}