/*

g++ project_2.cpp event.cpp node.cpp measure.cpp my_toolbox.cpp sensor_node.cpp storage_node.cpp blacklist_message.cpp message.cpp user_message.cpp outdated_measure.cpp user.cpp storage_node_message.cpp data_collector.cpp intra_user_message.cpp -o wir -std=c++11 

-pthread -std=c++11

*/

#include <iostream>
#include <fstream>    /* read, write from or to files */
#include <vector>
#include <map>
#include <random> 
#include <stdlib.h>     /* srand, rand */
#include <math.h>   // pow, sqrt

#include "node.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "my_toolbox.h"
#include "event.h"
#include "user.h"
#include "storage_node_message.h"
#include "data_collector.h"

using namespace std;

const string kFileName = "settings";
const string kDelimiter = "=";

typedef MyToolbox::MyTime MyTime;

void import_settings() {
  cout << "Importing settings from file: " << kFileName << "...";
  string line;
  ifstream settings_file(kFileName);
  if (settings_file.is_open()) {
    while (getline(settings_file, line)) {
      size_t first_space = line.find_first_of(" ");
      if (first_space != string::npos) {
        line = line.substr(0, first_space);   // trim the final blank spaces
      }
      string value_name;
      string value;
      size_t pos = line.find(kDelimiter);
      if (pos != string::npos) {    // if there is a delimiter '='
        value_name = line.substr(0, pos);
        value = line.substr(pos + 1);
        double num = stod(value);
        if (value_name == "num_storage_nodes") {
          MyToolbox::num_storage_nodes = (int)num;
        } else if (value_name == "num_sensors") {
          MyToolbox::num_sensors = (int)num;
        } else if (value_name == "num_users") {
          MyToolbox::num_users = (int)num;
        } else if (value_name == "num_bits_for_measure") {
          MyToolbox::num_bits_for_measure = (int)num;
        } else if (value_name == "bitrate") {
          MyToolbox::bitrate = (double)num;
        } else if (value_name == "bit_error_prob") {
          MyToolbox::bit_error_prob = (double)num;
        } else if (value_name == "tx_range") {
          MyToolbox::tx_range = (double)num;
        } else if (value_name == "ping_frequency") {
          MyToolbox::ping_frequency = (MyTime)num;
        } else if (value_name == "check_sensors_frequency") {
          MyToolbox::check_sensors_frequency = (MyTime)num;
        } else if (value_name == "C1") {
          MyToolbox::C1 = (double)num;
        } else if (value_name == "square_size") {
          MyToolbox::square_size = (int)num;
        } else if (value_name == "space_precision") {
          MyToolbox::space_precision = (int)num;
        } else if (value_name == "user_velocity") {
          MyToolbox::user_velocity = (double)num;
        } else if (value_name == "user_update_time") {
          MyToolbox::user_update_time = (MyTime)num;
        } else if (value_name == "processing_time") {
          MyToolbox::processing_time = (MyTime)num;
        } else if (value_name == "max_tx_offset") {
          MyToolbox::max_tx_offset = (MyTime)num;
        } else if (value_name == "user_observation_time") {
          MyToolbox::user_observation_time = (MyTime)num;
        } else if (value_name == "max_measure_generation_delay") {
          MyToolbox::max_measure_generation_delay = (MyTime)num;
        } else if (value_name == "sensor_failure_prob") {
          MyToolbox::sensor_failure_prob = (double)num;
        }
      }
    }
    settings_file.close();
    cout << "succeded!" << endl;
  } else {
    cout << "failed.\nNot able to open " << kFileName << "!" << endl; 
  }
}

DataCollector data_coll = DataCollector();
	map<unsigned int, Node*> sensors_map;
	  map<unsigned int, Node*> storage_nodes_map;
	  map<unsigned int, Node*> users_map;

	  default_random_engine generator;

int network_setup() {
//	DataCollector data_coll = DataCollector();
//	map<unsigned int, Node*> sensors_map;
//	  map<unsigned int, Node*> storage_nodes_map;
//	  map<unsigned int, Node*> users_map;

	  map<unsigned int, MyToolbox::MyTime> timetable;

	  double y_coord;
	  double x_coord;
	  /*default_random_engine*/ generator = MyToolbox::get_random_generator();
	  uniform_real_distribution<double> distribution(0.0, MyToolbox::square_size * 1.0);

	  // Create the sensors
	  for (int i = 1; i <= MyToolbox::num_sensors; i++) {
	    y_coord = distribution(generator);
	    x_coord = distribution(generator);
	    SensorNode* node = new SensorNode(MyToolbox::get_node_id(), y_coord, x_coord);
	    node->data_collector = &data_coll;
	    sensors_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
	    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
	  }

	  // Create the storage nodes
	  for (int i = 1; i <= MyToolbox::num_storage_nodes; i++) {
	    y_coord = distribution(generator);
	    x_coord = distribution(generator);
	    StorageNode* node = new StorageNode(MyToolbox::get_node_id(), y_coord, x_coord);
	    node->data_collector = &data_coll;
	    storage_nodes_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
	    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
	  }

	  // Create the users
	  for (int i = 1; i <= MyToolbox::num_users; i++) {
	    y_coord = distribution(generator);
	    x_coord = distribution(generator);
	    User* user = new User(MyToolbox::get_node_id(), y_coord, x_coord);
	    user->data_collector = &data_coll;
	    users_map.insert(pair<unsigned int, Node*>(user->get_node_id(), user));
	    timetable.insert(pair<unsigned int, MyTime>(user->get_node_id(), 0));
	  }

	  // I want Toolbox to store all the maps of all the nodes
	  MyToolbox::sensors_map_ptr = &sensors_map;
	  MyToolbox::storage_nodes_map_ptr = &storage_nodes_map;
	  MyToolbox::users_map_ptr = &users_map;
	  MyToolbox::set_timetable(timetable);

	  // Create the neighborhoods
	//  SensorNode *sensor1;
	//  SensorNode *sensor2;
	//  StorageNode *storage_node1;
	//  StorageNode *storage_node2;
	  double y1;
	  double x1;
	  double y2;
	  double x2;
	  double distance;
	  // ...for the sensors
	  for (auto& sensor1_pair : sensors_map) {
	    SensorNode* sensor1 = (SensorNode*)sensor1_pair.second;
	    y1 = sensor1->get_y_coord();
	    x1 = sensor1->get_x_coord();
	    for (auto& sensor2_pair : sensors_map) {
	      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
	      y2 = sensor2->get_y_coord();
	      x2 = sensor2->get_x_coord();
	      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
	      if (sensor1->get_node_id() != sensor2->get_node_id() && distance <= MyToolbox::tx_range) {
	//    	pair<map<unsigned int, Node*>::iterator, bool> res;
	//        res = (sensor1->near_sensors_)->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
	    	  sensor1->near_sensors_->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
	      }
	    }
	    for (auto& storage_node2_pair : storage_nodes_map) {
	      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
	      y2 = storage_node2->get_y_coord();
	      x2 = storage_node2->get_x_coord();
	      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
	      if (distance <= MyToolbox::tx_range) {
	        sensor1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
	      }
	    }
	  }
	  // ...for the storage nodes
	  for (auto& storage_node1_pair : storage_nodes_map) {
	    StorageNode* storage_node1 = (StorageNode*)storage_node1_pair.second;
	    y1 = storage_node1->get_y_coord();
	    x1 = storage_node1->get_x_coord();
	    for (auto& sensor2_pair : sensors_map) {
	      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
	      y2 = sensor2->get_y_coord();
	      x2 = sensor2->get_x_coord();
	      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
	      if (distance <= MyToolbox::tx_range) {
	        storage_node1->near_sensors_->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
	      }
	    }
	    for (auto& storage_node2_pair : storage_nodes_map) {
	      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
	      y2 = storage_node2->get_y_coord();
	      x2 = storage_node2->get_x_coord();
	      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
	      if (storage_node1->get_node_id() != storage_node2->get_node_id() && distance <= MyToolbox::tx_range) {
	        storage_node1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
	      }
	    }
	  }

	  // TODO debug
	//  for (map<unsigned int, Node*>::iterator it = storage_nodes_map.begin(); it != storage_nodes_map.end(); it++) {
	//  	  double x = it->second->get_x_coord();
	//  	  double y = it->second->get_y_coord();
	//  	  cout << "Cache " << it->second->get_node_id() << " (" << x << ", " << y << ")" << endl;
	//    }
	//
	//  for (map<unsigned int, Node*>::iterator it = sensors_map.begin(); it != sensors_map.end(); it++) {
	//	  double x = it->second->get_x_coord();
	//	  double y = it->second->get_y_coord();
	//	  cout << "Sensor " << it->second->get_node_id() << " (" << x << ", " << y << ")" << endl;
	//	  SensorNode* sns = (SensorNode*)it->second;
	//	  map<unsigned int, Node*> n_map = *(sns->near_storage_nodes_);
	//	  for (map<unsigned int, Node*>::iterator iit = n_map.begin(); iit != n_map.end(); iit++) {
	//		  double xx = iit->second->get_x_coord();
	//		  double yy = iit->second->get_y_coord();
	//		  double dd = sqrt(pow(x - xx, 2) + pow(y - yy, 2));
	//		  cout << " -" << iit->second->get_node_id() << ": (" << xx << ", " << yy << "), d=" << dd << endl;
	//	  }
	//  }
	  // end debug

	    int num_clouds = MyToolbox::show_clouds();	// for debug only: to see if all the nodes can communicate
	    return num_clouds;
}
  
int main() {

  srand(time(NULL));  // generate a random seed to generate random numbers later on
  //default_random_engine generator(time(NULL));

  import_settings();

  MyToolbox::initialize_toolbox();

  int howmany = 100;
  int w = 0;
  for (int i = 0; i < howmany; i++) {
	  cout << i << endl;
	  int aa = network_setup();
	  if (aa > 1) {
		  w++;
	  }
  }
  cout << "Probability sparse network: " << w * 1. / howmany << endl;
  return 0;

//  DataCollector data_coll = DataCollector();
//  MyToolbox::dc = &data_coll;

  /************************************************************************************************
  *
  * SET UP OF THE NETWORK
  *
  ************************************************************************************************/

//  map<unsigned int, Node*> sensors_map;
//  map<unsigned int, Node*> storage_nodes_map;
//  map<unsigned int, Node*> users_map;
//
//  map<unsigned int, MyToolbox::MyTime> timetable;
//
//  double y_coord;
//  double x_coord;
//  default_random_engine generator = MyToolbox::get_random_generator();
//  uniform_real_distribution<double> distribution(0.0, MyToolbox::square_size * 1.0);
//
//  // Create the sensors
//  for (int i = 1; i <= MyToolbox::num_sensors; i++) {
//    y_coord = distribution(generator);
//    x_coord = distribution(generator);
//    SensorNode* node = new SensorNode(MyToolbox::get_node_id(), y_coord, x_coord);
//    node->data_collector = &data_coll;
//    sensors_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
//    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
//  }
//
//  // Create the storage nodes
//  for (int i = 1; i <= MyToolbox::num_storage_nodes; i++) {
//    y_coord = distribution(generator);
//    x_coord = distribution(generator);
//    StorageNode* node = new StorageNode(MyToolbox::get_node_id(), y_coord, x_coord);
//    node->data_collector = &data_coll;
//    storage_nodes_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
//    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
//  }
//
//  // Create the users
//  for (int i = 1; i <= MyToolbox::num_users; i++) {
//    y_coord = distribution(generator);
//    x_coord = distribution(generator);
//    User* user = new User(MyToolbox::get_node_id(), y_coord, x_coord);
//    user->data_collector = &data_coll;
//    users_map.insert(pair<unsigned int, Node*>(user->get_node_id(), user));
//    timetable.insert(pair<unsigned int, MyTime>(user->get_node_id(), 0));
//  }
//
//  // I want Toolbox to store all the maps of all the nodes
//  MyToolbox::sensors_map_ptr = &sensors_map;
//  MyToolbox::storage_nodes_map_ptr = &storage_nodes_map;
//  MyToolbox::users_map_ptr = &users_map;
//  MyToolbox::set_timetable(timetable);
//
//  // Create the neighborhoods
////  SensorNode *sensor1;
////  SensorNode *sensor2;
////  StorageNode *storage_node1;
////  StorageNode *storage_node2;
//  double y1;
//  double x1;
//  double y2;
//  double x2;
//  double distance;
//  // ...for the sensors
//  for (auto& sensor1_pair : sensors_map) {
//    SensorNode* sensor1 = (SensorNode*)sensor1_pair.second;
//    y1 = sensor1->get_y_coord();
//    x1 = sensor1->get_x_coord();
//    for (auto& sensor2_pair : sensors_map) {
//      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
//      y2 = sensor2->get_y_coord();
//      x2 = sensor2->get_x_coord();
//      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//      if (sensor1->get_node_id() != sensor2->get_node_id() && distance <= MyToolbox::tx_range) {
////    	pair<map<unsigned int, Node*>::iterator, bool> res;
////        res = (sensor1->near_sensors_)->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
//    	  sensor1->near_sensors_->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
//      }
//    }
//    for (auto& storage_node2_pair : storage_nodes_map) {
//      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
//      y2 = storage_node2->get_y_coord();
//      x2 = storage_node2->get_x_coord();
//      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//      if (distance <= MyToolbox::tx_range) {
//        sensor1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
//      }
//    }
//  }
//  // ...for the storage nodes
//  for (auto& storage_node1_pair : storage_nodes_map) {
//    StorageNode* storage_node1 = (StorageNode*)storage_node1_pair.second;
//    y1 = storage_node1->get_y_coord();
//    x1 = storage_node1->get_x_coord();
//    for (auto& sensor2_pair : sensors_map) {
//      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
//      y2 = sensor2->get_y_coord();
//      x2 = sensor2->get_x_coord();
//      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//      if (distance <= MyToolbox::tx_range) {
//        storage_node1->near_sensors_->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
//      }
//    }
//    for (auto& storage_node2_pair : storage_nodes_map) {
//      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
//      y2 = storage_node2->get_y_coord();
//      x2 = storage_node2->get_x_coord();
//      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
//      if (storage_node1->get_node_id() != storage_node2->get_node_id() && distance <= MyToolbox::tx_range) {
//        storage_node1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
//      }
//    }
//  }
//
//  // TODO debug
////  for (map<unsigned int, Node*>::iterator it = storage_nodes_map.begin(); it != storage_nodes_map.end(); it++) {
////  	  double x = it->second->get_x_coord();
////  	  double y = it->second->get_y_coord();
////  	  cout << "Cache " << it->second->get_node_id() << " (" << x << ", " << y << ")" << endl;
////    }
////
////  for (map<unsigned int, Node*>::iterator it = sensors_map.begin(); it != sensors_map.end(); it++) {
////	  double x = it->second->get_x_coord();
////	  double y = it->second->get_y_coord();
////	  cout << "Sensor " << it->second->get_node_id() << " (" << x << ", " << y << ")" << endl;
////	  SensorNode* sns = (SensorNode*)it->second;
////	  map<unsigned int, Node*> n_map = *(sns->near_storage_nodes_);
////	  for (map<unsigned int, Node*>::iterator iit = n_map.begin(); iit != n_map.end(); iit++) {
////		  double xx = iit->second->get_x_coord();
////		  double yy = iit->second->get_y_coord();
////		  double dd = sqrt(pow(x - xx, 2) + pow(y - yy, 2));
////		  cout << " -" << iit->second->get_node_id() << ": (" << xx << ", " << yy << "), d=" << dd << endl;
////	  }
////  }
//  // end debug
//
//    int num_clouds = MyToolbox::show_clouds();	// for debug only: to see if all the nodes can communicate
//    if (num_clouds > 1) {
//    	cout << "Sparse net. Exit program." << endl;
//    	return 0;
//    }
//    return 0;

//  // Set sensors' supervisors
//  for (auto& sensor_pair : sensors_map) {
//	((SensorNode*)sensor_pair.second)->set_supervisor();
//    cout << "Sensor " << sensor_pair.second->get_node_id() << " sup " << ((SensorNode*)sensor_pair.second)->get_my_supervisor_id()
//    		<< " #neighbours: " << sensor_pair.second->near_storage_nodes_->size() << endl;
//  }


  /************************************************************************************************
   *
   * ACTIVATION OF THE NETWORK
   *
   ************************************************************************************************/

  // Sensors: activate measure generation and ping generation
  vector<Event> event_list;
  uniform_int_distribution<MyTime> first_measure_distrib(0.0, MyToolbox::max_measure_generation_delay * 1.0);
  uniform_int_distribution<int> first_ping_distrib(MyToolbox::ping_frequency / 2, MyToolbox::ping_frequency);
  for (auto& sensor_pair : sensors_map) {
	  // Activate measure generation
	  Event first_measure(first_measure_distrib(generator), Event::sensor_generate_measure);
//	  Event first_measure(first_measure_distrib(MyToolbox::get_random_generator()), Event::sensor_generate_measure);
	  cout << "first measure time " << first_measure.get_time() << endl;
	  first_measure.set_agent(sensor_pair.second);
	  vector<Event>::iterator event_iterator = event_list.begin();
	  for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
		  if (first_measure < *event_iterator) {
			  break;
		  }
	  }
	  event_list.insert(event_iterator, first_measure);

	  // Activate ping
//	  Event first_ping(first_ping_distrib(generator), Event::sensor_ping);
//	  first_ping.set_agent(sensor_pair.second);
//	  event_iterator = event_list.begin();
//	  for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
//		  if (first_ping < *event_iterator) {
//			  break;
//		  }
//	  }
//	  event_list.insert(event_iterator, first_ping);
  }

  // Storage nodes: activate ping check
//  uniform_int_distribution<int> first_check_distrib(MyToolbox::check_sensors_frequency / 2, MyToolbox::check_sensors_frequency);
//  for (auto& cache_pair : storage_nodes_map) {
//    Event first_check(first_check_distrib(generator), Event::check_sensors);
//    first_check.set_agent(cache_pair.second);
//    vector<Event>::iterator event_iterator = event_list.begin();
//    for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
//      if (first_check < *event_iterator) {
//        break;
//      }
//    }
//    event_list.insert(event_iterator, first_check);
//  }

  /************************************************************************************************
   *
   * STARTING OF THE SIMULATION
   *
   ************************************************************************************************/

  while (!event_list.empty()) {

    // TODO: verify next event has a different schedule time than this
      
    Event next_event = *(event_list.begin());
    event_list.erase(event_list.begin());
    vector<Event> new_events = next_event.execute_action();

    vector<Event>::iterator new_event_iterator = new_events.begin();
    for (; new_event_iterator != new_events.end(); new_event_iterator++) {
      vector<Event>::iterator old_event_iterator = event_list.begin();
      for (; old_event_iterator != event_list.end(); old_event_iterator++) {
        if (*old_event_iterator > *new_event_iterator) {
          break;
        }
      }
      event_list.insert(old_event_iterator, *new_event_iterator);
    }

//    show the list of the events
//    cout << "**event list: " << endl;
//    for (vector<Event>::iterator ii = event_list.begin(); ii != event_list.end(); ii++) {
//    	unsigned int tt_id = ((Node*)ii->get_agent())->get_node_id();
//    	cout << ii->get_time() << ", event type: " << Event::int2type(ii->get_event_type()) << ", del " << MyToolbox::int2nodetype(tt_id) << ": " << tt_id << endl;
//    }
//    cout << "**" << endl;
  }

  return 0;
}
