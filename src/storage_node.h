#ifndef __STORAGE_NODE_H__   
#define __STORAGE_NODE_H__   

#include <vector>
#include <map>

#include "node.h"
#include "measure.h"
#include "event.h"
#include "my_toolbox.h"

using namespace std;

class StorageNode : public Node {

 public:
  StorageNode() : Node () {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();}
  StorageNode(int node_id) : Node (node_id) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();}
  StorageNode(int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree();}

  unsigned char get_xored_measure() {return xored_measure_;}
  void manage_message();
  vector<Event> manage_measure(Measure*); 
  void set_supervision_map_(int, int);
  vector<Event> check_sensors(int);
  vector<Event> spread_blacklist(int,BlacklistMessage);
  vector<Event> remove_mesure(Measure);

  int do_action() {return 5;} // for debugging only

 private:
  typedef MyToolbox::MyTime MyTime;

  int LT_degree_; // number of xored measures
  unsigned char xored_measure_;
  map<int, int> last_measures_; // pairs <sensor_id, last_measure_id>
  vector<int> supervisioned_sensor_ids_;  // list of the sensor id's this node is the superisor of
  map<int, int> supervisioned_map_;         // map with  key = sensor_id and value = time of last ping
  vector<int> my_blacklist_;  // list of the sensor id's no more in the network

  vector<Event> send_measure(StorageNode* next_node, Measure* measure);
};

#endif