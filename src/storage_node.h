#ifndef __STORAGE_NODE_H__   
#define __STORAGE_NODE_H__   

#include <vector>
#include <map>

#include "node.h"

class Event;
class Message;
class Measure;
class BlacklistMessage;
class OutdatedMeasure;

class StorageNode : public Node {

 public:
  // Constructors
  StorageNode(); // : Node () {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id); // : Node (node_id) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}
  StorageNode(unsigned int node_id, double y_coord, double x_coord); // : Node (node_id, y_coord, x_coord) {LT_degree_ = MyToolbox::get_ideal_soliton_distribution_degree(); xored_measure_ = 0;}

  int LT_degree_; // number of xored measures
  unsigned char xored_measure_ = (unsigned char)0;
  std::map<unsigned int, unsigned int> last_measures_; // pairs <sensor_id, last_measure_id>s sns_id	// TODO to deprecate
  bool keep_checking_sensors_ = true;

  // Event execution methods
//  std::vector<Event> receive_measure(Measure*);
  std::vector<Event> receive_measure2(Measure*);
  std::vector<Event> try_retx(Message*);
  std::vector<Event> receive_user_request(unsigned int /*sender user id*/);
  std::vector<Event> check_sensors();
  std::vector<Event> spread_blacklist(BlacklistMessage*);
  void refresh_xored_data2(OutdatedMeasure*);
  void refresh_xored_data(OutdatedMeasure*);
  void receive_ping(unsigned int /*sensor id*/);	// from sensor

 private:
  typedef MyToolbox::MyTime MyTime;
  typedef MyToolbox::MeasureKey MeasureKey;

  struct SensorInfo {
	  bool alive_;
	  bool following_;
	  unsigned int sensor_id_;
//	  unsigned int most_recent_id_;
	  MeasureKey most_recent_key_;
  };

//  int LT_degree_; // number of xored measures
//  unsigned char xored_measure_;
  int indeces_pointer_ = 0;
  int indeces_counter_ = 0;
  int num_ping_checks_ = 10;		// how many ping checks I should do TODO debug
  int ping_check_counter_ = 0;	// TODO debug
//  std::map<unsigned int, unsigned int> last_measures_; // pairs <sensor_id, last_measure_id>s sns_id
//  std::map<unsigned int, MeasureKey> measures_; // pairs <sensor_id, last_measure_id>s sns_id
  std::vector<unsigned int> ignore_new_list;		// when I don't accept a NEW msr from a sns I save here it
  std::map<unsigned int, MyTime> supervised_map_;         // map with  key = sensor_id and value = time of last ping
  std::map<unsigned int, SensorInfo> stored_measures_;
  std::vector<unsigned int> my_blacklist_;  // list of the sensor id's no more in the network	// TODO to deprecate
  std::vector<MeasureKey> outdated_measure_keys_;  // list of the measures I have belonging to sensors no longer in the network	// TODO to deprecate
  std::vector<int> indeces_msr_to_keep_;

  std::vector<Event> send(unsigned int /*next_node_id*/, Message*);
  std::vector<Event> re_send(Message*);
  void set_measure_indeces();
};

#endif
