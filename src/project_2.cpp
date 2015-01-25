/*

g++ project_2.cpp event.cpp node.cpp measure.cpp my_toolbox.cpp sensor_node.cpp storage_node.cpp blacklist_message.cpp message.cpp node_dispatcher.cpp user.cpp -o wir -std=c++11 

-pthread -std=c++11

*/

#include <iostream>
#include <fstream>    /* read, write from or to files */
#include <vector>
#include <map>
#include <stdlib.h>     /* srand, rand */
#include <math.h>   // pow, sqrt

#include "node.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "blacklist_message.h"
#include "my_toolbox.h"
#include "event.h"
#include "user.h"
#include "node_dispatcher.h"

using namespace std;

// Parameters
const int NUM_STORAGE_NODES = 3;
const int NUM_SENSORS = 2;
const int NUM_USERS = 3;

const int NUM_BITS_FOR_MEASURE = 8; // in bits
const int NUM_BITS_FOR_ID = 32;   // in bits
const int NUM_BITS_PHY_MAC_OVERHEAD =  34 * 8 + 20 * 8; // in bits
// 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54, 125, 144, 300 Mb/s (IEEE 802.11n)
const double WIRELESS_CHANNEL_BITRATE = 1; // in Mb/s
const int PING_FREQUENCY = 10; //numero a caso
const int CHECK_SENSORS_FREQUENCY = 50; //numero a caso

const int C1 = 1;

const double RAY_LENGTH = 20.;  // in meters

const int SQUARE_SIZE = 5;  // in meters
const int SPACE_PRECISION = 1000; // how many fundamental space units in one meter

const double USER_VELOCITY = 0.8;// m/s [=3Km/h]

const int TX_RANGE = 1; // tx_range in meters
///////////////////////////////////////////////////////////////////////////////////



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
        }
      }
    }
    settings_file.close();
    cout << "succeded!" << endl;
  } else {
    cout << "...failed.\nNot able to open " << kFileName << "!" << endl; 
  }
}
  
int main() {

  import_settings();

  srand(time(NULL));  // generate a random seed to generate random numbers later on

  MyToolbox::set_k(NUM_SENSORS);
  MyToolbox::set_n(NUM_STORAGE_NODES);
  MyToolbox::set_C1(C1);
  MyToolbox::set_bits_for_measure(NUM_BITS_FOR_MEASURE);
  MyToolbox::set_bits_for_id(NUM_BITS_FOR_ID);
  MyToolbox::set_bits_for_phy_mac_overhead(NUM_BITS_PHY_MAC_OVERHEAD);
  MyToolbox::set_channel_bit_rate(WIRELESS_CHANNEL_BITRATE * 1000000);
  MyToolbox::set_ping_frequency(PING_FREQUENCY);
  MyToolbox::set_ping_frequency(CHECK_SENSORS_FREQUENCY);
  MyToolbox::set_space_precision(SPACE_PRECISION);
  MyToolbox::set_square_size(SQUARE_SIZE);
  MyToolbox::set_user_velocity(USER_VELOCITY);
  MyToolbox::set_user_update_time();
  MyToolbox::set_tx_range(TX_RANGE);

// Set up the network
  // I use these vectors to set up the network
  vector<SensorNode*> sensors;
  vector<StorageNode*> storage_nodes;
  vector<User*> users;

  map<int, Node*> sensors_map;
  map<int, Node*> storage_nodes_map;
  map<int, Node*> users_map;
  
  map<int, MyToolbox::MyTime> timetable;

  int sensor_id = 0;
  int storage_node_id = 0;
  int user_id = 0;

  double y_coord;
  double x_coord;
  // Create the sensors
  for (int i = 1; i <= MyToolbox::num_sensors; i++) {
    y_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    x_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    SensorNode *node = new SensorNode(sensor_id++, y_coord, x_coord);
    node->near_sensors_ = &sensors_map;
    node->near_storage_nodes_ = &storage_nodes_map;
    sensors.push_back(node);
    sensors_map.insert(pair<int, Node*>(node->get_node_id(), node));
  }
  // Create the storage nodes
  for (int i = 1; i <= MyToolbox::num_storage_nodes; i++) {
    y_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    x_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    StorageNode *node = new StorageNode(storage_node_id++, y_coord, x_coord);
    node->near_sensors_ = &sensors_map;
    node->near_storage_nodes_ = &storage_nodes_map;
    storage_nodes.push_back(node);
    storage_nodes_map.insert(pair<int, Node*>(node->get_node_id(), node));
    timetable.insert(pair<int, int>(node->get_node_id(), 0));
  }
  // Create the users
  for (int i = 1; i <= MyToolbox::num_users; i++) {
    y_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    x_coord = rand() % (MyToolbox::square_size * MyToolbox::space_precision);
    User *user = new User(user_id++, y_coord, x_coord);
    users.push_back(user);
    users_map.insert(pair<int, Node*>(user->get_node_id(), user));
  }

  MyToolbox::sensors_map_ptr = &sensors_map;
  MyToolbox::storage_nodes_map_ptr = &storage_nodes_map; 
  MyToolbox::users_map_ptr = &users_map;
  
  MyToolbox::set_timetable(timetable);

  SensorNode *sensor1;
  SensorNode *sensor2;
  StorageNode *storage_node1;
  StorageNode *storage_node2;
  double y1;
  double x1;
  double y2;
  double x2;
  double distance;
  for (int i = 0; i < sensors.size(); i++) {
    sensor1 = sensors.at(i);
    y1 = sensor1->get_y_coord();
    x1 = sensor1->get_x_coord();
    for (int j = 0; j < sensors.size(); j++) {
      sensor2 = sensors.at(j);
      y2 = sensor2->get_y_coord();
      x2 = sensor2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (sensor1->get_node_id() != sensor2->get_node_id() && distance <= MyToolbox::ray_length * MyToolbox::space_precision) {
        sensor1->near_sensors_->insert(pair<int, Node*>(sensor2->get_node_id(), sensor2));
      }
    }
    for (int j = 0; j < storage_nodes.size(); j++) {
      storage_node2 = storage_nodes.at(j);
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (distance <= MyToolbox::ray_length * MyToolbox::space_precision) {
        sensor1->near_storage_nodes_->insert(pair<int, Node*>(storage_node2->get_node_id(), storage_node2));
      }
    }
  }
  for (int i = 0; i < storage_nodes.size(); i++) {
    storage_node1 = storage_nodes.at(i);
    y1 = storage_node1->get_y_coord();
    x1 = storage_node1->get_x_coord();
    for (int j = 0; j < sensors.size(); j++) {
      sensor2 = sensors.at(j);
      y2 = sensor2->get_y_coord();
      x2 = sensor2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (distance <= MyToolbox::ray_length * MyToolbox::space_precision) {
        storage_node1->near_sensors_->insert(pair<int, Node*>(sensor2->get_node_id(), sensor2));
      }
    }
    for (int j = 0; j < storage_nodes.size(); j++) {
      storage_node2 = storage_nodes.at(j);
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (storage_node1->get_node_id() != storage_node2->get_node_id() && distance <= MyToolbox::ray_length * MyToolbox::space_precision) {
        storage_node1->near_storage_nodes_->insert(pair<int, Node*>(storage_node2->get_node_id(), storage_node2));
      }
    }
  }

  MyToolbox::set_sensor_nodes(sensors);
  MyToolbox::set_storage_nodes(storage_nodes);
  MyToolbox::set_users(users);

  // Event manager
  vector<Event> event_list;

  // Event test_event(10, Event::sensor_generate_measure);
  // test_event.set_agent(sensors.at(0));
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

   // Event test_event2(30, Event::move_user);
   //test_event2.set_agent(users.at(0));
   //vector<Event> aaa = test_event2.execute_action();
   
   
   // users.at(1)->set_output_symbol();
   // Event test_event2(33, Event::user_send_to_user);
   // test_event2.set_agent(users.at(1));
   // test_event2.set_agent_to_reply(users.at(0));
   // vector<Event> aaa = test_event2.execute_action();

  return 0;
}
