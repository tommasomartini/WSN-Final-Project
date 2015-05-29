#ifndef __SENSOR_NODE_H__   
#define __SENSOR_NODE_H__ 

#include <vector> 

#include "node.h"

class Message;
class StorageNode;
class Event;

class SensorNode: public Node {
 public:
//  SensorNode (unsigned int node_id) : Node (node_id) {measure_id_ = 0;}  // TODO remove
  SensorNode (unsigned int node_id, double y_coord, double x_coord);

  // Event execution methods
  std::vector<Event> generate_measure();
  std::vector<Event> try_retx(Message*);
  std::vector<Event> ping(); // fast and simple way
  void breakup();
  
  void set_supervisor();	// this function has to be called at the beginning of the program, after the network initialization
  
  unsigned int get_my_supervisor_id() {return my_supervisor_id_;}	// TODO just for debug

 private:
  typedef MyToolbox::MyTime MyTime;

  int how_many_measures_ = 0;
  int how_many_pings_ = 1;
  int ping_counter_ = 0;
  unsigned char old_measure_data = 17;
  unsigned char new_measure_data = 17;
  unsigned int my_supervisor_id_;
  unsigned int measure_id_ = 0;
  bool first_generated_measure_ = true;
  bool do_ping_ = true;

  std::vector<Event> send(unsigned int /*next node id*/, Message*);
  std::vector<Event> re_send(Message*);
//  unsigned int get_random_neighbor();

  unsigned char get_measure_data();
};

#endif
