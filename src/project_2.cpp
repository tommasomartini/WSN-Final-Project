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
        } else if (value_name == "max_num_users") {
          MyToolbox::max_num_users = (int)num;
        } else if (value_name == "num_users") {
          MyToolbox::num_users = (int)num;
        } else if (value_name == "num_bits_for_id") {
          MyToolbox::num_bits_for_id = (int)num;
        } else if (value_name == "num_bits_for_measure") {
          MyToolbox::num_bits_for_measure = (int)num;
        } else if (value_name == "num_bits_phy_mac_overhead") {
          MyToolbox::num_bits_phy_mac_overhead = (int)num;
        } else if (value_name == "num_bits_for_measure_id") {
          MyToolbox::num_bits_for_measure_id = (int)num;
        } else if (value_name == "bitrate") {
          MyToolbox::bitrate = (double)num;
        } else if (value_name == "bit_error_prob") {
          MyToolbox::bit_error_prob = (double)num;
        } else if (value_name == "ray_length") {
          MyToolbox::ray_length = (double)num;
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
        } else if (value_name == "mean_processing_time") {
          MyToolbox::mean_processing_time = (MyTime)num;
        } else if (value_name == "std_dev_processing_time") {
          MyToolbox::std_dev_processing_time = (MyTime)num;
        } else if (value_name == "max_tx_offset") {
          MyToolbox::max_tx_offset = (MyTime)num;
        } else if (value_name == "max_tx_offset_ping") {
          MyToolbox::max_tx_offset_ping = (MyTime)num;
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
  
int main() {

  srand(time(NULL));  // generate a random seed to generate random numbers later on
  //default_random_engine generator(time(NULL));

  import_settings();

  MyToolbox::initialize_toolbox();

// Set up the network
  // I use these vectors ONLY to set up the network
//  vector<SensorNode*> sensors;
//  vector<StorageNode*> storage_nodes;
//  vector<User*> users;

  map<unsigned int, Node*> sensors_map;
  map<unsigned int, Node*> storage_nodes_map;
  map<unsigned int, Node*> users_map;
  
  map<unsigned int, MyToolbox::MyTime> timetable;

  double y_coord;
  double x_coord;
  default_random_engine generator = MyToolbox::get_random_generator();
  uniform_real_distribution<double> distribution(0.0, MyToolbox::square_size * 1.0);
  // Create the sensors
  for (int i = 1; i <= MyToolbox::num_sensors; i++) {
    y_coord = distribution(generator);
    x_coord = distribution(generator);
    SensorNode* node = new SensorNode(MyToolbox::get_node_id(), y_coord, x_coord);
    sensors_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
  }
  // Create the storage nodes
  for (int i = 1; i <= MyToolbox::num_storage_nodes; i++) {
    y_coord = distribution(generator);
    x_coord = distribution(generator);
    StorageNode* node = new StorageNode(MyToolbox::get_node_id(), y_coord, x_coord);
    storage_nodes_map.insert(pair<unsigned int, Node*>(node->get_node_id(), node));
    timetable.insert(pair<unsigned int, MyTime>(node->get_node_id(), 0));
  }
  // Create the users
  for (int i = 1; i <= MyToolbox::num_users; i++) {
    y_coord = distribution(generator);
    x_coord = distribution(generator);
    User* user = new User(MyToolbox::get_node_id(), y_coord, x_coord);
    users_map.insert(pair<unsigned int, Node*>(user->get_node_id(), user));
    timetable.insert(pair<unsigned int, MyTime>(user->get_node_id(), 0));
  }

  // I want Toolbox to store all the maps of all the nodes
  MyToolbox::sensors_map_ptr = &sensors_map;
  MyToolbox::storage_nodes_map_ptr = &storage_nodes_map; 
  MyToolbox::users_map_ptr = &users_map;
  MyToolbox::set_timetable(timetable);

//  SensorNode *sensor1;
//  SensorNode *sensor2;
//  StorageNode *storage_node1;
//  StorageNode *storage_node2;
  double y1;
  double x1;
  double y2;
  double x2;
  double distance;
  for (auto& sensor1_pair : sensors_map) {
    SensorNode* sensor1 = (SensorNode*)sensor1_pair.second;
    y1 = sensor1->get_y_coord();
    x1 = sensor1->get_x_coord();
    for (auto& sensor2_pair : sensors_map) {
      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
      y2 = sensor2->get_y_coord();
      x2 = sensor2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (sensor1->get_node_id() != sensor2->get_node_id() && distance <= MyToolbox::ray_length) {
    	pair<map<unsigned int, Node*>::iterator, bool> res;
        res = (sensor1->near_sensors_)->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
      }
    }
    for (auto& storage_node2_pair : storage_nodes_map) {
      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (distance <= MyToolbox::ray_length) {
        sensor1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
      }
    }
  }
  for (auto& storage_node1_pair : storage_nodes_map) {
    StorageNode* storage_node1 = (StorageNode*)storage_node1_pair.second;
    y1 = storage_node1->get_y_coord();
    x1 = storage_node1->get_x_coord();
    for (auto& sensor2_pair : sensors_map) {
      SensorNode* sensor2 = (SensorNode*)sensor2_pair.second;
      y2 = sensor2->get_y_coord();
      x2 = sensor2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (distance <= MyToolbox::ray_length) {
        storage_node1->near_sensors_->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
      }
    }
    for (auto& storage_node2_pair : storage_nodes_map) {
      StorageNode* storage_node2 = (StorageNode*)storage_node2_pair.second;
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (storage_node1->get_node_id() != storage_node2->get_node_id() && distance <= MyToolbox::ray_length) {
        storage_node1->near_storage_nodes_->insert(pair<unsigned int, Node*>(storage_node2->get_node_id(), storage_node2));
      }
    }
  }

  for (auto& sensor_pair : sensors_map) {
	SensorNode* sns = (SensorNode*)sensor_pair.second;
    sns->set_supervisor();
    cout << "sensor " << sns->get_node_id() << " sup " << sns->get_my_supervisor_id() << endl;
  }

  vector<Event> event_list;
  uniform_int_distribution<MyTime> first_measure_distrib(0.0, MyToolbox::max_measure_generation_delay * 1.0);
  uniform_int_distribution<int> first_ping_distrib(MyToolbox::ping_frequency / 2, MyToolbox::ping_frequency);
  for (auto& sensor_pair : sensors_map) {
	  Event first_measure(first_measure_distrib(generator), Event::sensor_generate_measure);
	  first_measure.set_agent(sensor_pair.second);
	  vector<Event>::iterator event_iterator = event_list.begin();
	  for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
		  if (first_measure < *event_iterator) {
			  break;
		  }
	  }
	  event_list.insert(event_iterator, first_measure);

	  Event first_ping(first_ping_distrib(generator), Event::sensor_ping);
	  first_ping.set_agent(sensor_pair.second);
	  event_iterator = event_list.begin();
	  for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
		  if (first_ping < *event_iterator) {
			  break;
		  }
	  }
	  event_list.insert(event_iterator, first_ping);
  }

  // TODO attivare il check dei ping dei cache

  for (auto& ee : event_list) {
	  cout << "sensor: " << ((SensorNode*)ee.get_agent())->get_node_id() << ", event time: " << ee.get_time() << endl;
  }

  while (!event_list.empty()) {
  // for (int i = 0; i < 5; i++) {

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

    cout << "**event list: " << endl;
    for (vector<Event>::iterator ii = event_list.begin(); ii != event_list.end(); ii++) {
    	unsigned int tt_id = ((Node*)ii->get_agent())->get_node_id();
    	cout << ii->get_time() << ", event type: " << Event::int2type(ii->get_event_type()) << ", del " << MyToolbox::int2nodetype(tt_id) << ": " << tt_id << endl;
    }
    cout << "**" << endl;
  }



  

  // Event test_event(10, Event::sensor_generate_measure);
  // test_event.set_agent(sensors.at(0));
  // Measure mmeasure(18, 11, 0, Measure::measure_type_new);
  // mmeasure.set_receiver_node_id(1);
  // Event test_event(0, Event::sensor_try_to_send);
  // test_event.set_agent(sensors.at(0));
  // test_event.set_message(&mmeasure);
  // test_event.execute_action();

  // event_list.push_back(Event(7));
  // event_list.push_back(Event(8));
  // event_list.push_back(Event(78));

  
  
  // check correctness Arianna's part 
  /*
  map <int,int> mappa;
  mappa[0]=1;
  mappa[1]=4;
  
  MyToolbox::set_timetable(mappa);
  Event test_event2(30, Event::sensor_ping);
  test_event2.set_agent(sensors.at(0));
  
  
   test_event2 = test_event2.execute_action();
  
   Event test_event3(70, Event::check_sensors);
  test_event3.set_agent(storage_nodes.at(0));
     test_event3 = test_event3.execute_action();

     Event test_event4(71,Event::blacklist_sensor);
     int a=0;
     int*biiii= &a;
        BlacklistMessage list(biiii,1);
     test_event4.set_agent(storage_nodes.at(0));
     test_event4.set_blacklist(list);
     test_event4 = test_event4.execute_action();
  
  
  
  Event test_event4(71,Event::remove_measure);
     test_event4.set_agent(storage_nodes.at(0));
     vector<Event> aaa = test_event4.execute_action();
  
  */
  // for (User *user : users) {
  //   cout<<"y "<<user->get_y_coord()<<"x "<<user->get_x_coord();}

    //Event test_event2(30, Event::move_user);
   //test_event2.set_agent(users.at(0));
   //vector<Event> aaa = test_event2.execute_action();
   
//   users.at(1)->set_output_symbol();
 //  Event test_event3(33, Event::user_send_to_user);
  // test_event3.set_agent(users.at(1));
  // test_event3.set_agent_to_reply(users.at(0));
  // vector<Event> aaaa = test_event3.execute_action();
  // test_event.execute_action();


  // Measure mmeasure(18, 11, 0, Measure::measure_type_new);
  // mmeasure.set_receiver_node_id(1);
  // Event test_event(0, Event::sensor_try_to_send);
  // test_event.set_agent(sensors.at(0));
  // test_event.set_message(&mmeasure);
  // test_event.execute_action();

  // event_list.push_back(Event(7));
  // event_list.push_back(Event(8));
  // event_list.push_back(Event(78));

  // while (!event_list.empty()) {
  // for (int i = 0; i < 5; i++) {

  //   // TODO: verify next event has a different schedule time than this

  //   Event next_event = *(event_list.begin());
  //   event_list.erase(event_list.begin());
  //   vector<Event> new_events = next_event.execute_action();

  //   vector<Event>::iterator new_event_iterator = new_events.begin();
  //   vector<Event>::iterator old_event_iterator = event_list.begin();
  //   for (; new_event_iterator != new_events.end(); new_event_iterator++) {
  //     for (; old_event_iterator != event_list.end(); old_event_iterator++) {
  //       if (*old_event_iterator > *new_event_iterator)
  //         break;
  //     }
  //   }
  //   event_list.insert(old_event_iterator, *new_event_iterator);
  // }
  
  
  // check correctness Arianna's part 
  /*
  map <int,int> mappa;
  mappa[0]=1;
  mappa[1]=4;
  
  MyToolbox::set_timetable(mappa);
  Event test_event2(30, Event::sensor_ping);
  test_event2.set_agent(sensors.at(0));
  
  
   test_event2 = test_event2.execute_action();
  
   Event test_event3(70, Event::check_sensors);
  test_event3.set_agent(storage_nodes.at(0));
     test_event3 = test_event3.execute_action();

     Event test_event4(71,Event::blacklist_sensor);
     int a=0;
     int*biiii= &a;
        BlacklistMessage list(biiii,1);
     test_event4.set_agent(storage_nodes.at(0));
     test_event4.set_blacklist(list);
     test_event4 = test_event4.execute_action();
  
  
  
  Event test_event4(71,Event::remove_measure);
     test_event4.set_agent(storage_nodes.at(0));
     vector<Event> aaa = test_event4.execute_action();
  
  */
  // for (User *user : users) {
  //   cout<<"y "<<user->get_y_coord()<<"x "<<user->get_x_coord();}

    //Event test_event2(30, Event::move_user);
   //test_event2.set_agent(users.at(0));
   //vector<Event> aaa = test_event2.execute_action();
   
//   users.at(1)->set_output_symbol();
 //  Event test_event3(33, Event::user_send_to_user);
  // test_event3.set_agent(users.at(1));
  // test_event3.set_agent_to_reply(users.at(0));
  // vector<Event> aaaa = test_event3.execute_action();

  return 0;
}
