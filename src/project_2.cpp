/*

g++ project_2.cpp event.cpp node.cpp measure.cpp -o wir 

-pthread -std=c++11

*/

#include <iostream>
// #include <stdio.h>
#include <vector>
#include <stdlib.h>     /* srand, rand */
// #include <thread>
// #include <chrono>
// #include <math.h>
// #include <time.h>       /* time */

// #include "node.h"
// #include "storage_node.h"
// #include "sensor_node.h"
// #include "user.h"
// #include "my_toolbox.h"

#include "event.h"

using namespace std;

const int NUM_STORAGE_NODES = 5;
const int NUM_SENSORS = 3;
const int NUM_USERS = 1;

const int C1 = 1;

const double RAY_LENGTH = 20.;  // in meters

const int SQUARE_SIZE = 5;  // in meters
const int SPACE_PRECISION = 1000; // how many fundamental space units in one meter
  
int main() {

  srand(time(NULL));

  // MyToolbox::set_k(NUM_SENSORS);
  // MyToolbox::set_n(NUM_STORAGE_NODES);
  // MyToolbox::set_C1(C1);

  // cout << "max forward number = " << MyToolbox::get_max_msg_hops() << endl;

  vector<Event> event_list;

  event_list.push_back(Event(7));
  event_list.push_back(Event(8));
  event_list.push_back(Event(78));

  // while (!event_list.empty()) {
  for (int i = 0; i < 5; i++) {
    Event next_event = *(event_list.begin());
    event_list.erase(event_list.begin());
    Event new_event = new_event.execute_action();
    vector<Event>::iterator event_iterator = event_list.begin();
    for (; event_iterator != event_list.end(); event_iterator++) {
      if (*event_iterator > new_event)
        break;
    }
    event_list.insert(event_iterator, new_event);
  }

  return 0;
}
