#ifndef __STORAGE_NODE_H__   
#define __STORAGE_NODE_H__   

#include <vector>
#include <map>

#include "node.h"
#include "my_toolbox.h"

class Event;
class Message;
class Measure;
class BlacklistMessage;
class OutdatedMeasure;

using namespace std;

class StorageNode : public Node {

 public:
  // Constructors
  StorageNode() : Node () {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id) : Node (node_id) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}

  // getters
  unsigned char get_xored_measure() {return xored_measure_;}
  vector<unsigned int> get_ids () {
      vector<unsigned int> ids;
      for (auto& i : last_measures_) 
            ids.push_back(i.first);
    return ids;
  }

  // setters
  void set_supervision_map_(int, int);

  // Event execution methods
  vector<Event> receive_measure(Measure*); // Tom
  vector<Event> try_retx_measure(Measure*, int /*next_node_id*/); // Tom
  vector<Event> try_retx(Message*, int /*next_node_id*/); // Tom
  vector<Event> receive_user_request(unsigned int /*sender user id*/); // Tom
  vector<Event> check_sensors(); // Arianna
  vector<Event> spread_blacklist(BlacklistMessage*); // Arianna
  vector<Event> remove_mesure(OutdatedMeasure*); // Arianna

 private:
  typedef MyToolbox::MyTime MyTime;

  int LT_degree_; // number of xored measures
  unsigned char xored_measure_;
  map<unsigned int, unsigned int> last_measures_; // pairs <sensor_id, last_measure_id>
  vector<unsigned int> supervisioned_sensor_ids_;  // list of the sensor id's this node is the supervisior of
  map<unsigned int, int> supervisioned_map_;         // map with  key = sensor_id and value = time of last ping
  vector<unsigned int> my_blacklist_;  // list of the sensor id's no more in the network

  vector<Event> send(Node* /*next_node*/, Message*);
 
};

#endif
