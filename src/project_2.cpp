/*

g++ project_2.cpp event.cpp node.cpp measure.cpp my_toolbox.cpp sensor_node.cpp storage_node.cpp -o wir -std=c++11 

-pthread -std=c++11

*/

#include <iostream>
// #include <stdio.h>
#include <vector>
#include <map>
#include <stdlib.h>     /* srand, rand */
// #include <thread>
// #include <chrono>
#include <math.h>   // pow, sqrt
// #include <time.h>       /* time */

#include "node.h"
#include "storage_node.h"
#include "sensor_node.h"
// #include "user.h"
#include "my_toolbox.h"

#include "event.h"

using namespace std;

// Parameters
const int NUM_STORAGE_NODES = 3;
const int NUM_SENSORS = 2;
const int NUM_USERS = 1;

const int NUM_BITS_FOR_MEASURE = 8; // in bits
const int NUM_BITS_FOR_ID = 32;   // in bits
const int NUM_BITS_PHY_MAC_OVERHEAD =  34 * 8 + 20 * 8; // in bits
// 1, 2, 5.5, 6, 9, 11, 12, 18, 24, 36, 48, 54, 125, 144, 300 Mb/s (IEEE 802.11n)
const double WIRELESS_CHANNEL_BITRATE = 1; // in Mb/s

const int C1 = 1;

const double RAY_LENGTH = 20.;  // in meters

const int SQUARE_SIZE = 5;  // in meters
const int SPACE_PRECISION = 1000; // how many fundamental space units in one meter
  
int main() {

  // int a = 3;
  // int b = 10000000;
  // int c = 10000000000;
  // cout << a + b + c << endl;

  srand(time(NULL));  // generate a random seed to generate random numbers later on

  MyToolbox::set_k(NUM_SENSORS);
  MyToolbox::set_n(NUM_STORAGE_NODES);
  MyToolbox::set_C1(C1);
  MyToolbox::set_bits_for_measure(NUM_BITS_FOR_MEASURE);
  MyToolbox::set_bits_for_id(NUM_BITS_FOR_ID);
  MyToolbox::set_bits_for_phy_mac_overhead(NUM_BITS_PHY_MAC_OVERHEAD);
  MyToolbox::set_channel_bit_rate(WIRELESS_CHANNEL_BITRATE * 1000000);

  // cout << "max forward number = " << MyToolbox::get_max_msg_hops() << endl;

// Set up the network
  vector<SensorNode*> sensors;
  vector<StorageNode*> storage_nodes;
  // vector<Node*> all_nodes; // useful for the generation of the nodes and to fulfill the neighborhood tables

  map<int, MyToolbox::MyTime> timetable;

  int sensor_id = 0;
  int storage_node_id = 0;

  double y_coord;
  double x_coord;
  for (int i = 1; i <= NUM_SENSORS; i++) {
    y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
    x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
    SensorNode *node = new SensorNode(sensor_id++, y_coord, x_coord);
    sensors.push_back(node);
    // all_nodes.push_back(node);
  }

  for (int i = 1; i <= NUM_STORAGE_NODES; i++) {
    y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
    x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
    StorageNode *node = new StorageNode(storage_node_id++, y_coord, x_coord);
    storage_nodes.push_back(node);
    timetable.insert(pair<int, int>(node->get_node_id(), 0));
    // all_nodes.push_back(node);
  }

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
  // for (SensorNode *sensor1 : sensors) {  // only allowed in c++11
  for (int i = 0; i < sensors.size(); i++) {
    sensor1 = sensors.at(i);
    // cout << "Id sensore 1: " << sensor1->get_node_id() << endl;
    y1 = sensor1->get_y_coord();
    x1 = sensor1->get_x_coord();
    // cout << "Coordinate 1: " << sensor1->get_y_coord() << ", " << sensor1->get_x_coord() << endl;
    for (int j = 0; j < sensors.size(); j++) {
      sensor2 = sensors.at(j);
      // cout << "Id sensore 2: " << sensor1->get_node_id() << endl;
      y2 = sensor2->get_y_coord();
      x2 = sensor2->get_x_coord();
      // cout << "Coordinate 2: " << sensor2->get_y_coord() << ", " << sensor2->get_x_coord() << endl;
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (sensor1->get_node_id() != sensor2->get_node_id() && distance <= RAY_LENGTH * SPACE_PRECISION) {
        sensor1->add_near_sensor_node(sensor2);
      }
    }
    for (int j = 0; j < storage_nodes.size(); j++) {
      storage_node2 = storage_nodes.at(j);
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (distance <= RAY_LENGTH * SPACE_PRECISION) {
        sensor1->add_near_storage_node(storage_node2);
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
      if (distance <= RAY_LENGTH * SPACE_PRECISION) {
        storage_node1->add_near_sensor_node(sensor2);
      }
    }
    for (int j = 0; j < storage_nodes.size(); j++) {
      storage_node2 = storage_nodes.at(j);
      y2 = storage_node2->get_y_coord();
      x2 = storage_node2->get_x_coord();
      distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
      if (storage_node1->get_node_id() != storage_node2->get_node_id() && distance <= RAY_LENGTH * SPACE_PRECISION) {
        storage_node1->add_near_storage_node(storage_node2);
      }
    }
  }

  // // Create the users
  // for (int i = 1; i <= NUM_USERS; i++) {
  //   y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
  //   x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
  //   User *user = new User(node_id++, y_coord, x_coord);
  //   users.push_back(user);
  // }

  // for (User *user : users) {
  //   y1 = user->get_y_coord();
  //   x1 = user->get_x_coord();
  //   for (int j = 0; j < sensors.size(); j++) {
  //     node2 = sensors.at(j);

  //     y2 = node2->get_y_coord();
  //     x2 = node2->get_x_coord();

  //     distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

  //     if (distance <= RAY_LENGTH * SPACE_PRECISION) {
  //       user->add_near_sensor_node((SensorNode*)node2);
  //     }
  //   }

  //   for (int j = 0; j < storage_nodes.size(); j++) {
  //     node2 = storage_nodes.at(j);

  //     y2 = node2->get_y_coord();
  //     x2 = node2->get_x_coord();

  //     distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

  //     if (distance <= RAY_LENGTH * SPACE_PRECISION) {
  //       user->add_near_storage_node((StorageNode*)node2);
  //     }
  //   }
  // }

  // Event manager
  vector<Event> event_list;

  Event test_event(10, Event::sensor_generate_measure);
  test_event.set_agent(sensors.at(0));
  test_event.execute_action();

  // event_list.push_back(Event(7));
  // event_list.push_back(Event(8));
  // event_list.push_back(Event(78));

  // while (!event_list.empty()) {
  for (int i = 0; i < 5; i++) {

    // TODO: verify next event has a different schedule time than this

    Event next_event = *(event_list.begin());
    event_list.erase(event_list.begin());
    vector<Event> new_events = next_event.execute_action();

    vector<Event>::iterator new_event_iterator = new_events.begin();
    vector<Event>::iterator old_event_iterator = event_list.begin();
    for (; new_event_iterator != new_events.end(); new_event_iterator++) {
      for (; old_event_iterator != event_list.end(); old_event_iterator++) {
        if (*old_event_iterator > *new_event_iterator)
          break;
      }
    }
    event_list.insert(old_event_iterator, *new_event_iterator);
  }

  return 0;
}
