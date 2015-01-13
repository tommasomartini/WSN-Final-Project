/*

g++ project_2.cpp event.cpp node.cpp measure.cpp my_toolbox.cpp sensor_node.cpp storage_node.cpp -o wir 

-pthread -std=c++11

*/

#include <iostream>
// #include <stdio.h>
#include <vector>
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

const int NUM_STORAGE_NODES = 3;
const int NUM_SENSORS = 2;
const int NUM_USERS = 1;

const int C1 = 1;

const double RAY_LENGTH = 20.;  // in meters

const int SQUARE_SIZE = 5;  // in meters
const int SPACE_PRECISION = 1000; // how many fundamental space units in one meter
  
int main() {

  srand(time(NULL));

  MyToolbox::set_k(NUM_SENSORS);
  MyToolbox::set_n(NUM_STORAGE_NODES);
  MyToolbox::set_C1(C1);

  // cout << "max forward number = " << MyToolbox::get_max_msg_hops() << endl;

// Set up the network
  vector<SensorNode*> sensors;
  vector<StorageNode*> storage_nodes;
  // vector<Node*> all_nodes; // useful for the generation of the nodes and to fulfill the neighborhood tables

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
    // all_nodes.push_back(node);
  }

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
  // for (int i = 0; i < 5; i++) {
  //   Event next_event = *(event_list.begin());
  //   event_list.erase(event_list.begin());
  //   Event new_event = next_event.execute_action();
  //   vector<Event>::iterator event_iterator = event_list.begin();
  //   for (; event_iterator != event_list.end(); event_iterator++) {
  //     if (*event_iterator > new_event)
  //       break;
  //   }
  //   event_list.insert(event_iterator, new_event);
  // }

  return 0;
}
