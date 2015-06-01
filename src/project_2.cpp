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
typedef MyToolbox::IdType IdType;

class EventComparator {
public:
	bool operator() (Event& lhs, Event& rhs) const {
		if (rhs < lhs) {
			return true;
		}
		return false;
	}
};

DataCollector* data_coll;
priority_queue<Event, vector<Event>, EventComparator> main_event_queue;
default_random_engine generator;
int end_time;

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
					MyTime val = (int)num * 60 * 60 * pow(10, 9);
					MyToolbox::ping_frequency_ = val;
				} else if (value_name == "check_sensors_frequency") {
					MyTime val = (int)num * 60 * 60 * pow(10, 9);
					MyToolbox::check_sensors_frequency_ = val;
				} else if (value_name == "C1") {
					MyToolbox::C1_ = (double)num;
				} else if (value_name == "square_size") {
					MyToolbox::square_size_ = (int)num;
				} else if (value_name == "processing_time") {
					MyToolbox::processing_time_ = (MyTime)num;
				} else if (value_name == "max_tx_offset") {
					MyToolbox::max_tx_offset_ = (MyTime)num;
				} else if (value_name == "user_observation_time") {
					MyTime val = (int)num * pow(10, 9);
					MyToolbox::user_observation_time_ = val;
				} else if (value_name == "measure_generation_delay") {
					MyTime val = (int)num * 60 * pow(10, 9);
					MyToolbox::measure_generation_delay_ = val;
				} else if (value_name == "sensor_failure_prob") {
					MyToolbox::sensor_failure_prob_ = (double)num;
				} else if (value_name == "num_measures_for_sensor") {
					MyToolbox::num_measures_for_sensor_ = (int)num;
				} else if (value_name == "c0_robust") {
					MyToolbox::c0_robust_ = (double)num;
				} else if (value_name == "delta_robust") {
					MyToolbox::delta_robust_ = (double)num;
				} else if (value_name == "end_time") {
					end_time = (int)num;
				} else if (value_name == "intra_user_communication") {
					int val = (int)num;
					if (val == 0) {
						MyToolbox::intra_user_communication_ = false;
					} else {
						MyToolbox::intra_user_communication_ = true;
					}
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
	double y_coord;
	double x_coord;
	uniform_real_distribution<double> distribution(0.0, MyToolbox::square_size_ * 1.0);

	// Create the sensors
	for (int i = 1; i <= MyToolbox::num_sensors_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		SensorNode node(MyToolbox::get_node_id(), y_coord, x_coord);
		node.data_collector = data_coll;
		MyToolbox::sensors_map_.insert(pair<unsigned int, SensorNode>(node.get_node_id(), node));
		MyToolbox::timetable_.insert(pair<unsigned int, MyTime>(node.get_node_id(), 0));
		MyToolbox::alive_sensors_.push_back(node.get_node_id());
	}

	// Create the storage nodes
	for (int i = 1; i <= MyToolbox::num_storage_nodes_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		StorageNode node(MyToolbox::get_node_id(), y_coord, x_coord);
		node.data_collector = data_coll;
		MyToolbox::storage_nodes_map_.insert(pair<unsigned int, StorageNode>(node.get_node_id(), node));
		MyToolbox::timetable_.insert(pair<unsigned int, MyTime>(node.get_node_id(), 0));
	}

	// Create the users
	for (int i = 1; i <= MyToolbox::num_users_; i++) {
		y_coord = distribution(generator);
		x_coord = distribution(generator);
		User user(MyToolbox::get_node_id(), y_coord, x_coord);
		user.data_collector = data_coll;
		MyToolbox::users_map_.insert(pair<unsigned int, User>(user.get_node_id(), user));
		MyToolbox::timetable_.insert(pair<unsigned int, MyTime>(user.get_node_id(), 0));
	}

	// Create the neighborhoods
	double y1;
	double x1;
	double y2;
	double x2;
	double distance;

	// ...for the sensors
	for (map<unsigned int, SensorNode>::iterator sns_it = MyToolbox::sensors_map_.begin(); sns_it != MyToolbox::sensors_map_.end(); sns_it++) {
		SensorNode* sns1 = &(sns_it->second);
		y1 = sns1->get_y_coord();
		x1 = sns1->get_x_coord();
		for (map<unsigned int, SensorNode>::iterator sns_it2 = MyToolbox::sensors_map_.begin(); sns_it2 != MyToolbox::sensors_map_.end(); sns_it2++) {
			SensorNode* sns2 = &(sns_it2->second);
			y2 = sns2->get_y_coord();
			x2 = sns2->get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (sns1->get_node_id() != sns2->get_node_id() && distance <= MyToolbox::tx_range_) {
				sns1->near_sensors_.insert(pair<unsigned int, SensorNode*>(sns2->get_node_id(), sns2));
			}
		}
		for (map<unsigned int, StorageNode>::iterator cache_it = MyToolbox::storage_nodes_map_.begin(); cache_it != MyToolbox::storage_nodes_map_.end(); cache_it++) {
			StorageNode* cache2 = &(cache_it->second);
			y2 = cache2->get_y_coord();
			x2 = cache2->get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (distance <= MyToolbox::tx_range_) {
				sns1->near_storage_nodes_.insert(pair<unsigned int, StorageNode*>(cache2->get_node_id(), cache2));
			}
		}
	}
	// ...for the storage nodes
	for (map<unsigned int, StorageNode>::iterator cache_it = MyToolbox::storage_nodes_map_.begin(); cache_it != MyToolbox::storage_nodes_map_.end(); cache_it++) {
		StorageNode* cache1 = &(cache_it->second);
		y1 = cache1->get_y_coord();
		x1 = cache1->get_x_coord();
		for (map<unsigned int, SensorNode>::iterator sns_it = MyToolbox::sensors_map_.begin(); sns_it != MyToolbox::sensors_map_.end(); sns_it++) {
			SensorNode* sns2 = &(sns_it->second);
			y2 = sns2->get_y_coord();
			x2 = sns2->get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (distance <= MyToolbox::tx_range_) {
				cache1->near_sensors_.insert(pair<unsigned int, SensorNode*>(sns2->get_node_id(), sns2));
			}
		}
		for (map<unsigned int, StorageNode>::iterator cache_it2 = MyToolbox::storage_nodes_map_.begin(); cache_it2 != MyToolbox::storage_nodes_map_.end(); cache_it2++) {
			StorageNode* cache2 = &(cache_it2->second);
			y2 = cache2->get_y_coord();
			x2 = cache2->get_x_coord();
			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));
			if (cache1->get_node_id() != cache2->get_node_id() && distance <= MyToolbox::tx_range_) {
				cache1->near_storage_nodes_.insert(pair<unsigned int, StorageNode*>(cache2->get_node_id(), cache2));
			}
		}
	}

	double density = data_coll->graph_density();
	cout << "Graph density = " << density << endl;

	int num_clouds = MyToolbox::check_clouds2();	// for debug only: to see if all the nodes can communicate
	if (num_clouds > 1) {
		cout << "> Sparse net! <" << endl;
		return false;
	}

	if (!MyToolbox::sensor_connected()) {
		cout << "> Isolated sensors! <" << endl;
		return false;
	}

	// Set sensors' supervisors
	for (map<unsigned int, SensorNode>::iterator sns_it = MyToolbox::sensors_map_.begin(); sns_it != MyToolbox::sensors_map_.end(); sns_it++) {
		(sns_it->second).set_supervisor();
	}

	return true;
}

void activate_measure_generation() {
	uniform_int_distribution<MyTime> first_measure_distrib(0.0, MyToolbox::measure_generation_delay_ * 1.0);
	for (auto& sensor_pair : MyToolbox::sensors_map_) {
		Event first_measure(first_measure_distrib(generator), Event::event_type_generated_measure);
		first_measure.set_agent(&(sensor_pair.second));
		first_measure.set_agent_id(sensor_pair.first);
		main_event_queue.push(first_measure);
	}
}

void activate_ping_generation() {
	uniform_int_distribution<int> first_ping_distrib(MyToolbox::ping_frequency_ / 2, MyToolbox::ping_frequency_);
	for (auto& sensor_pair : MyToolbox::sensors_map_) {
		Event first_ping(first_ping_distrib(generator), Event::event_type_sensor_ping);
		first_ping.set_agent(&(sensor_pair.second));
		first_ping.set_agent_id(sensor_pair.first);
		main_event_queue.push(first_ping);
	}
}

void activate_ping_check() {
	uniform_int_distribution<int> first_check_distrib(MyToolbox::check_sensors_frequency_ / 2, MyToolbox::check_sensors_frequency_);
	for (auto& cache_pair : MyToolbox::storage_nodes_map_) {
		Event first_check(first_check_distrib(generator), Event::event_type_cache_checks_sensors);
		first_check.set_agent(&(cache_pair.second));
		first_check.set_agent_id(cache_pair.first);
		main_event_queue.push(first_check);
	}
}

void activate_users() {
	uniform_int_distribution<int> first_step_distrib(MyToolbox::check_sensors_frequency_ / 2, MyToolbox::check_sensors_frequency_);
	int counter = 1;	// TODO debug
	for (auto& user_pair : MyToolbox::users_map_) {
		MyTime first_step_time = 1 /* * MyToolbox::num_sensors_ */* MyToolbox::measure_generation_delay_ + 5 * MyToolbox::get_tx_offset();
		Event first_step(first_step_time, Event::event_type_user_moves);
		first_step.set_agent(&(user_pair.second));
		first_step.set_agent_id(user_pair.first);
		main_event_queue.push(first_step);

		counter++;
	}
}

int main() {

	import_settings();

	MyToolbox::initialize_toolbox();

	data_coll = new DataCollector();
	main_event_queue = priority_queue<Event, vector<Event>, EventComparator>();
	generator = MyToolbox::generator_;


//	if (std::ifstream("move_user.txt")) {
//		remove("move_user.txt");
//	}
//	ofstream outfile("move_user.txt");
//	outfile.close();

	bool setup_succeeded = network_setup();
	if (!setup_succeeded) {
		cout << "Network setup failed. Quit program!" << endl;
		return 0;
	}
	cout << "Network correctly set-up!" << endl;

	activate_measure_generation();
//	activate_ping_generation();
//	activate_ping_check();
	activate_users();

	MyTime end_time_ns = end_time * 60 * 60 * pow(10, 9);
	Event end_event(end_time_ns, Event::event_type_end);
	main_event_queue.push(end_event);

	cout << "- - - Starting the Program - - -" << endl;

	while (!main_event_queue.empty()) {
		Event next_event = main_event_queue.top();
		if (next_event.get_event_type() == Event::event_type_end) {
			cout << "End reached!" << endl;
			break;
		}
		main_event_queue.pop();
		vector<Event> new_events = next_event.execute_action();
		for (vector<Event>::iterator event_it = new_events.begin(); event_it != new_events.end(); event_it++) {
			main_event_queue.push(*event_it);
		}
	}

	cout << "- - - Ending the Program - - -" << endl;

	data_coll->report();

	cout << "Bye!" << endl;

	return 0;
}
