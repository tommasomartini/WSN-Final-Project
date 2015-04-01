#ifndef __SENSOR_NODE_H__   
#define __SENSOR_NODE_H__ 

#include <vector> 

#include "node.h"
#include "my_toolbox.h"
#include "measure.h"

class Message;
class StorageNode;
class Event;

using namespace std;

class SensorNode: public Node {
 public:
  SensorNode (unsigned int node_id) : Node (node_id) {measure_id_ = 0;}  // TODO remove
  SensorNode (unsigned int node_id, double y_coord, double x_coord);

  // setters
  // void set_measure(Measure);  // TODO remove

  // getters
  // Measure get_measure() {return measure_;}    // TODO remove. Qundo lo uso??

  // Event execution methods
  vector<Event> generate_measure(); // Tom
  vector<Event> try_retx(Message*, unsigned int /*next_node_id*/); // Tom
  vector<Event> sensor_ping(); // Tom
  vector<Event> sensor_ping(int); // Arianna
  
  void set_my_supervisor(unsigned int); // TODO: uno dei due metodi e' da rimuovere
  void set_supervisor();
  
 private:
  typedef MyToolbox::MyTime MyTime;

  Measure measure_;   // measure generated by the sensor
  unsigned int my_supervisor_id_;
  unsigned int measure_id_;
  bool first_generated_measure_;

  vector<Event> send_measure(StorageNode*); // DEPRECATED
  vector<Event> send(StorageNode*, Message*);

  int new_measure_id();
};

#endif
