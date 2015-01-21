#ifndef __SENSOR_NODE_H__   
#define __SENSOR_NODE_H__ 

#include <vector> 

#include "node.h"
#include "storage_node.h"
#include "measure.h"
#include "event.h"
#include "my_toolbox.h"

using namespace std;

class SensorNode: public Node {
 public:
  SensorNode (int node_id) : Node (node_id) {measure_id_ = 0;}
  // SensorNode (int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {measure_id_ = 0;}
  SensorNode (int node_id, double y_coord, double x_coord);

  void set_measure(Measure);
  Measure get_measure() {return measure_;}
  vector<Event> generate_measure();
  vector<Event> sensor_ping(int);

  int do_action() {return 2;} // for debugging only

 private:
  typedef MyToolbox::MyTime MyTime;

  static const MyTime MAX_MEASURE_GENERATION_DELAY = 1 * 60 * 60 * (MyTime)1000000000;  // 1 hour, in ns

  Measure measure_;   // measure generated by the sensor
  int my_supervisor_id_;
  int measure_id_;
  bool first_generated_measure_;

  vector<Event> send_measure(StorageNode* next_node);
};

#endif
