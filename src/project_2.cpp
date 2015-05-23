/*

g++ project_2.cpp event.cpp node.cpp measure.cpp my_toolbox.cpp sensor_node.cpp storage_node.cpp blacklist_message.cpp message.cpp user_message.cpp outdated_measure.cpp user.cpp storage_node_message.cpp data_collector.cpp intra_user_message.cpp -o wir -std=c++11 

-pthread -std=c++11

 */

#include <iostream>
#include <fstream>    /* read, write from or to files */
#include <vector>
#include <queue>
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
#include "data_collector.h"

using namespace std;

const string kFileName = "settings";
const string kDelimiter = "=";

typedef MyToolbox::MyTime MyTime;

class EventComparator {
public:
	bool operator() (Event& lhs, Event& rhs) const {
		if (rhs < lhs) {
			return true;
		}
		return false;
	}
};

//int main ()
//{
//  int myints[]= {10,60,50,20};
//
//  std::priority_queue<int> first;
//  std::priority_queue<int> second (myints,myints+4);
//  std::priority_queue<int, std::vector<int>, std::greater<int> >
//                            third (myints,myints+4);
//  // using mycomparison:
//  typedef std::priority_queue<int,std::vector<int>,mycomparison> mypq_type;
//
//  mypq_type fourth;                       // less-than comparison
//  mypq_type fifth (mycomparison(true));   // greater-than comparison
//
//  return 0;
//}

DataCollector* data_coll;
vector<Event> event_list;
priority_queue<Event, vector<Event>, EventComparator> main_event_queue;
map<unsigned int, SensorNode> sensors_map;
map<unsigned int, StorageNode> storage_nodes_map;
map<unsigned int, User> users_map;
map<unsigned int, MyToolbox::MyTime> timetable;
default_random_engine generator;

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
					MyToolbox::num_storage_nodes_ = (int)num;
				} else if (value_name == "num_sensors") {
					MyToolbox::num_sensors_ = (int)num;
				} else if (value_name == "num_users") {
					MyToolbox::num_users_ = (int)num;
				} else if (value_name == "num_bits_for_measure") {
					MyToolbox::num_bits_for_measure_ = (int)num;
				} else if (value_name == "bitrate") {
					MyToolbox::bitrate_ = (double)num;
				} else if (value_name == "bit_error_prob") {
					MyToolbox::bit_error_prob_ = (double)num;
				} else if (value_name == "tx_range") {
					MyToolbox::tx_range_ = (double)num;
				} else if (value_name == "ping_frequency") {
					MyToolbox::ping_frequency_ = (MyTime)num;
				} else if (value_name == "check_sensors_frequency") {
					MyToolbox::check_sensors_frequency_ = (MyTime)num;
				} else if (value_name == "C1") {
					MyToolbox::C1_ = (double)num;
				} else if (value_name == "square_size") {
					MyToolbox::square_size_ = (int)num;
				} else if (value_name == "space_precision") {
					MyToolbox::space_precision_ = (int)num;
				} else if (value_name == "user_velocity") {
					MyToolbox::user_velocity_ = (double)num;
				} else if (value_name == "user_update_time") {
					MyToolbox::user_update_time_ = (MyTime)num;
				} else if (value_name == "processing_time") {
					MyToolbox::processing_time_ = (MyTime)num;
				} else if (value_name == "max_tx_offset") {
					MyToolbox::max_tx_offset_ = (MyTime)num;
				} else if (value_name == "user_observation_time") {
					MyToolbox::user_observation_time_ = (MyTime)num;
				} else if (value_name == "max_measure_generation_delay") {
					MyToolbox::max_measure_generation_delay_ = (MyTime)num;
				} else if (value_name == "sensor_failure_prob") {
					MyToolbox::sensor_failure_prob_ = (double)num;
				}
			}
		}
		settings_file.close();
		cout << "succeded!" << endl;
	} else {
		cout << "failed.\nNot able to open " << kFileName << "!" << endl;
	}
}

bool network_setup() {
	sensors_map = map<unsigned int, SensorNode>();
	storage_nodes_map = map<unsigned int, StorageNode>();
	users_map = map<unsigned int, User>();
	timetable = map<unsigned int, MyToolbox::MyTime>();

	double y_coord;
	double x_coord;
	uniform_real_distribution<double> distribution(0.0, MyToolbox::square_size_ * 1.0);

	// Create the sensors
	for (int i = 1; i <= MyToolbox::num_sensors_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		SensorNode node(MyToolbox::get_node_id(), y_coord, x_coord);
		node.data_collector = data_coll;
		sensors_map.insert(pair<unsigned int, SensorNode>(node.get_node_id(), node));
		timetable.insert(pair<unsigned int, MyTime>(node.get_node_id(), 0));
	}

	// Create the storage nodes
	for (int i = 1; i <= MyToolbox::num_storage_nodes_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		StorageNode node(MyToolbox::get_node_id(), y_coord, x_coord);
		node.data_collector = data_coll;
		storage_nodes_map.insert(pair<unsigned int, StorageNode>(node.get_node_id(), node));
		timetable.insert(pair<unsigned int, MyTime>(node.get_node_id(), 0));
	}

	// Create the users
	for (int i = 1; i <= MyToolbox::num_users_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		User user(MyToolbox::get_node_id(), y_coord, x_coord);
		user.data_collector = data_coll;
		users_map.insert(pair<unsigned int, User>(user.get_node_id(), user));
		timetable.insert(pair<unsigned int, MyTime>(user.get_node_id(), 0));
	}

	// I want Toolbox to store all the maps of all the nodes
	MyToolbox::sensors_map_ptr_ = sensors_map;
	MyToolbox::storage_nodes_map_ptr_ = storage_nodes_map;
	MyToolbox::users_map_ptr_ = users_map;
	MyToolbox::timetable_= timetable;

	// Create the neighborhoods
	double y1;
	double x1;
	double y2;
	double x2;
	double distance;

	// ...for the sensors
//	for (auto& sensor1_pair : *sensors_map) {
	for (map<unsigned int, SensorNode>::iterator sns_it = sensors_map.begin(); sns_it != sensors_map.end(); sns_it++) {
		SensorNode sensor1 = sns_it->second;
		y1 = sensor1.get_y_coord();
		x1 = sensor1.get_x_coord();
		for (map<unsigned int, SensorNode>::iterator sns_it2 = sensors_map.begin(); sns_it2 != sensors_map.end(); sns_it2++) {
			SensorNode sensor2 = sns_it2->second;
			y2 = sensor2.get_y_coord();
			x2 = sensor2.get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (sensor1.get_node_id() != sensor2.get_node_id() && distance <= MyToolbox::tx_range_) {
				//    	pair<map<unsigned int, Node*>::iterator, bool> res;
				//        res = (sensor1->near_sensors_)->insert(pair<unsigned int, Node*>(sensor2->get_node_id(), sensor2));
				sensor1.near_sensors_.insert(pair<unsigned int, SensorNode>(sensor2.get_node_id(), sensor2));
			}
		}
		for (map<unsigned int, StorageNode>::iterator cache_it = storage_nodes_map.begin(); cache_it != storage_nodes_map.end(); cache_it++) {
			StorageNode storage_node2 = cache_it->second;
			y2 = storage_node2.get_y_coord();
			x2 = storage_node2.get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (distance <= MyToolbox::tx_range_) {
				sensor1.near_storage_nodes_.insert(pair<unsigned int, StorageNode>(storage_node2.get_node_id(), storage_node2));
			}
		}
	}
	// ...for the storage nodes
	for (map<unsigned int, StorageNode>::iterator cache_it = storage_nodes_map.begin(); cache_it != storage_nodes_map.end(); cache_it++) {
		StorageNode storage_node1 = cache_it->second;
		y1 = storage_node1.get_y_coord();
		x1 = storage_node1.get_x_coord();
		for (map<unsigned int, SensorNode>::iterator sns_it = sensors_map.begin(); sns_it != sensors_map.end(); sns_it++) {
			SensorNode sensor2 = sns_it->second;
			y2 = sensor2.get_y_coord();
			x2 = sensor2.get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (distance <= MyToolbox::tx_range_) {
				storage_node1.near_sensors_.insert(pair<unsigned int, SensorNode>(sensor2.get_node_id(), sensor2));
			}
		}
		for (map<unsigned int, StorageNode>::iterator cache_it2 = storage_nodes_map.begin(); cache_it2 != storage_nodes_map.end(); cache_it2++) {
			StorageNode storage_node2 = cache_it2->second;
			y2 = storage_node2.get_y_coord();
			x2 = storage_node2.get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (storage_node1.get_node_id() != storage_node2.get_node_id() && distance <= MyToolbox::tx_range_) {
				storage_node1.near_storage_nodes_.insert(pair<unsigned int, StorageNode>(storage_node2.get_node_id(), storage_node2));
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

	int num_clouds = MyToolbox::check_clouds();	// for debug only: to see if all the nodes can communicate
	if (num_clouds > 1) {
		cout << "Sparse net. Exit program." << endl;
		return false;
	}

	if (!MyToolbox::sensor_connected()) {
		return false;
	}

	// Set sensors' supervisors
	for (map<unsigned int, SensorNode>::iterator sns_it = sensors_map.begin(); sns_it != sensors_map.end(); sns_it++) {
		(sns_it->second).set_supervisor();
		cout << "Sensor " << sns_it->second.get_node_id() << " sup " << sns_it->second.get_my_supervisor_id()
		    										<< " #neighbours: " << sns_it->second.near_storage_nodes_.size() << endl;
	}

	return true;
}

void activate_measure_generation() {
	uniform_int_distribution<MyTime> first_measure_distrib(0.0, MyToolbox::max_measure_generation_delay_ * 1.0);
	uniform_int_distribution<int> first_ping_distrib(MyToolbox::ping_frequency_ / 2, MyToolbox::ping_frequency_);
	for (auto& sensor_pair : sensors_map) {
		Event first_measure(first_measure_distrib(generator), Event::sensor_generate_measure);
		cout << "first measure time " << first_measure.get_time() << endl;
		first_measure.set_agent(&(sensor_pair.second));
		vector<Event>::iterator event_iterator = event_list.begin();
		for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
			if (first_measure < *event_iterator) {
				break;
			}
		}
		event_list.insert(event_iterator, first_measure);
	}
}

void activate_ping_generation() {
	uniform_int_distribution<MyTime> first_measure_distrib(0.0, MyToolbox::max_measure_generation_delay_ * 1.0);
	uniform_int_distribution<int> first_ping_distrib(MyToolbox::ping_frequency_ / 2, MyToolbox::ping_frequency_);
	for (auto& sensor_pair : sensors_map) {
		vector<Event>::iterator event_iterator = event_list.begin();
		Event first_ping(first_ping_distrib(generator), Event::sensor_ping);
		first_ping.set_agent(&(sensor_pair.second));
		event_iterator = event_list.begin();
		for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
			if (first_ping < *event_iterator) {
				break;
			}
		}
		event_list.insert(event_iterator, first_ping);
	}
}

void activate_ping_check() {
	uniform_int_distribution<int> first_check_distrib(MyToolbox::check_sensors_frequency_ / 2, MyToolbox::check_sensors_frequency_);
	for (auto& cache_pair : storage_nodes_map) {
		Event first_check(first_check_distrib(generator), Event::check_sensors);
		first_check.set_agent(&(cache_pair.second));
		vector<Event>::iterator event_iterator = event_list.begin();
		for (; event_iterator != event_list.end(); event_iterator++) {	// scan the event list and insert the new event in the right place
			if (first_check < *event_iterator) {
				break;
			}
		}
		event_list.insert(event_iterator, first_check);
	}
}

int main() {

	//	srand(time(NULL));  // generate a random seed to generate random numbers later on

	import_settings();

	MyToolbox::initialize_toolbox();

	data_coll = new DataCollector();
	event_list = vector<Event>();
	main_event_queue = priority_queue<Event, vector<Event>, EventComparator>();
	generator = MyToolbox::generator_;

	bool setup_succeeded = network_setup();
//	if (!setup_succeeded) {
//		cout << "Network setup failed. Quit program!" << endl;
//		return 0;
//	}


//	return 0;
//
//	activate_measure_generation();
//	activate_ping_generation();
//	activate_ping_check();

	while (!main_event_queue.empty()) {

		// TODO: verify next event has a different schedule time than this

		Event next_event = main_event_queue.top();
		main_event_queue.pop();
		vector<Event> new_events = next_event.execute_action();
		for (vector<Event>::iterator event_it = new_events.begin(); event_it != new_events.end(); event_it++) {
			main_event_queue.push(*event_it);
		}
	}

	return 0;
}
