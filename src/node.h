#ifndef __NODE_H__   
#define __NODE_H__  

#include <vector>
#include <map>
#include <queue>

#include "event.h"
#include "data_collector.h"

class SensorNode;
class StorageNode;
class User;

class Node : public Agent {
 public:
  DataCollector* data_collector;

  std::map<unsigned int, Node*> near_sensors_;
  std::map<unsigned int, Node*> near_storage_nodes_;
  std::map<unsigned int, User> near_users_;  // creata domenica 8 marzo. Davvero necessario?

  Node();
  Node(unsigned int /*node_id*/);
  Node(unsigned int /*node_id*/, double /*y_coord*/, double /*x_coord*/);
    
  // getters
  unsigned int get_node_id() {return node_id_;}
  double get_y_coord() {return y_coord_;}
  double get_x_coord() {return x_coord_;}

  // setters
  void set_position(double /*y_coord*/, double /*x_coord*/);
  
  // TODO remove?
//  void add_near_storage_node(Node*);
//  void add_near_sensor_node(Node*);
//  void add_near_user(Node*);
//  static bool are_nodes_near(Node*, Node*); // TODO: non ha senso mettere questa funzione in node!

 protected:
  unsigned int node_id_;
  double y_coord_, x_coord_;
  /*  queue of the events generated by this node which are still to be executed.
        The first event must be always present in the event queue, even if it is
      not going to be effectively executed.

      The first event is always the one in execution in this moment. Empty queue => no events being executed
  */ 
  queue<Event> event_queue_; 
  
  unsigned int get_random_neighbor();
};

#endif
