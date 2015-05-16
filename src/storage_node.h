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

class StorageNode : public Node {

 public:
  // Constructors
  StorageNode() : Node () {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id) : Node (node_id) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id, double y_coord, double x_coord) : Node (node_id, y_coord, x_coord) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}

  // getters
  unsigned char get_xored_measure() {return xored_measure_;}
  std::vector<unsigned int> get_ids () {
    std::vector<unsigned int> ids;
    for (auto& i : last_measures_)
      ids.push_back(i.first);
    return ids;
  }

  // setters
  void set_supervision_map_(int, int);

  // Event execution methods
  std::vector<Event> receive_measure(Measure*); // Tom
  std::vector<Event> try_retx_measure(Measure*, unsigned int /*next_node_id*/); // Tom	// FIXME remove
  std::vector<Event> try_retx(Message*, unsigned int /*next_node_id*/); // Tom	// FIXME remove
  std::vector<Event> try_retx(Message*); // Tom
  std::vector<Event> receive_user_request(unsigned int /*sender user id*/); // Tom
  std::vector<Event> receive_reinit_query(unsigned int /*sender user id*/); // Tom
  std::vector<Event> receive_reinit_response(); // Tom
  std::vector<Event> check_sensors(); // Arianna
  std::vector<Event> spread_blacklist(BlacklistMessage*); // Arianna
  std::vector<Event> remove_mesure(OutdatedMeasure*); // Arianna

 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  bool reinit_mode_ = false;
  int LT_degree_; // number of xored measures
  unsigned char xored_measure_;
  std::map<unsigned int, unsigned int> last_measures_; // pairs <sensor_id, last_measure_id>s sns_id
  std::map<unsigned int, MeasureKey> measures_; // pairs <sensor_id, last_measure_id>s sns_id
  std::vector<unsigned int> ignore_new_list;		// when I don't accept a NEW msr from a sns I save here it
  std::vector<unsigned int> supervisioned_sensor_ids_;  // list of the sensor id's this node is the supervisor of TODO remove
  std::map<unsigned int, int> supervisioned_map_;         // map with  key = sensor_id and value = time of last ping
  std::vector<unsigned int> my_blacklist_;  // list of the sensor id's no more in the network
  std::vector<MeasureKey> outdated_measure_keys_;  // list of the measures I have belonging to sensors no longer in the network

  std::vector<Event> send(Node* /*next_node*/, Message*);
  std::vector<Event> send2(unsigned int /*next_node_id*/, Message*);
  std::vector<Event> re_send(Message*);
  std::vector<Event> reinitialize();	// used to reinitialize the node when something happens (for example a received msr gap)
  unsigned int get_random_neighbor();
};

#endif
