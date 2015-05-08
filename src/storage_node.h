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
  vector<Event> try_retx_measure(Measure*, unsigned int /*next_node_id*/); // Tom	// FIXME remove
  vector<Event> try_retx(Message*, unsigned int /*next_node_id*/); // Tom	// FIXME remove
  vector<Event> try_retx(Message*); // Tom
  vector<Event> receive_user_request(unsigned int /*sender user id*/); // Tom
  vector<Event> receive_reinit_query(unsigned int /*sender user id*/); // Tom
  vector<Event> receive_reinit_response(); // Tom
  vector<Event> check_sensors(); // Arianna
  vector<Event> spread_blacklist(BlacklistMessage*); // Arianna
  vector<Event> remove_mesure(OutdatedMeasure*); // Arianna

 private:
  typedef MyToolbox::MyTime MyTime;

  bool reinit_mode_ = false;
  int LT_degree_; // number of xored measures
  unsigned char xored_measure_;
  map<unsigned int, unsigned int> last_measures_; // pairs <sensor_id, last_measure_id>s sns_id
  vector<unsigned int> ignore_new_list;		// when I don't accept a NEW msr from a sns I save here it
  vector<unsigned int> supervisioned_sensor_ids_;  // list of the sensor id's this node is the supervisor of TODO remove
  map<unsigned int, int> supervisioned_map_;         // map with  key = sensor_id and value = time of last ping
  vector<unsigned int> my_blacklist_;  // list of the sensor id's no more in the network

  vector<Event> send(Node* /*next_node*/, Message*);
  vector<Event> send2(unsigned int /*next_node_id*/, Message*);
  vector<Event> re_send(Message*);
  vector<Event> reinitialize();	// used to reinitialize the node when something happens (for example a received msr gap)
  unsigned int get_random_neighbor();
};

#endif
