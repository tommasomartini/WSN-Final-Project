/*

g++ project_1.cpp node.cpp sensor_node.cpp storage_node.cpp message.cpp user.cpp my_toolbox.cpp storage_node_message.cpp -o pro -pthread -std=c++11

*/

#include <iostream>
#include <stdio.h>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <thread>
#include <chrono>
#include <math.h>
#include <time.h>       /* time */

#include "node.h"
#include "storage_node.h"
#include "sensor_node.h"
#include "user.h"
#include "my_toolbox.h"

using namespace std;

const int NUM_STORAGE_NODES = 5;
const int NUM_SENSORS = 3;
const int NUM_USERS = 1;

const int C1 = 1;

const double RAY_LENGTH = 20.;	// in meters

const int SQUARE_SIZE = 5;	// in meters
const int SPACE_PRECISION = 1000;	// how many fundamental space units in one meter
	
int main() {

	MyToolbox::set_k(NUM_SENSORS);
	MyToolbox::set_n(NUM_STORAGE_NODES);
	MyToolbox::set_C1(C1);

	cout << "max forward number = " << MyToolbox::get_max_msg_hops() << endl;

	int node_id = 0;

	vector<StorageNode*> storage_nodes;
	vector<SensorNode*> sensors;
	vector<Node*> all_nodes;	// useful for the generations of the nodes and to fulfill the neighborhood tables
	vector<User*> users;

	srand(time(NULL));

	//	Create the sensors and the storage nodes
	double y_coord;
	double x_coord;
	for (int i = 1; i <= NUM_SENSORS; i++) {
		y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		SensorNode *node = new SensorNode(node_id++, y_coord, x_coord);
		sensors.push_back(node);
		all_nodes.push_back(node);
	}

	for (int i = 1; i <= NUM_STORAGE_NODES; i++) {
		y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		StorageNode *node = new StorageNode(node_id++, y_coord, x_coord);
		storage_nodes.push_back(node);
		all_nodes.push_back(node);
	}

	Node *node2;
	StorageNode *nnode2;
	double y1;
	double x1;
	double y2;
	double x2;
	double distance;
	for (Node *node1 : all_nodes) {
		y1 = node1->get_y_coord();
		x1 = node1->get_x_coord();
		for (int j = 0; j < sensors.size(); j++) {
			node2 = sensors.at(j);

			y2 = node2->get_y_coord();
			x2 = node2->get_x_coord();

			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

			if (node1->get_node_id() != node2->get_node_id() && distance <= RAY_LENGTH * SPACE_PRECISION) {
				node1->add_near_sensor_node(node2);
			}
		}

		for (int j = 0; j < storage_nodes.size(); j++) {
			node2 = storage_nodes.at(j);

			y2 = node2->get_y_coord();
			x2 = node2->get_x_coord();

			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

			if (node1->get_node_id() != node2->get_node_id() && distance <= RAY_LENGTH * SPACE_PRECISION) {
				node1->add_near_storage_node(node2);
			}
		}
	}

	// Create the users
	for (int i = 1; i <= NUM_USERS; i++) {
		y_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		x_coord = rand() % (SQUARE_SIZE * SPACE_PRECISION);
		User *user = new User(node_id++, y_coord, x_coord);
		users.push_back(user);
	}

	for (User *user : users) {
		y1 = user->get_y_coord();
		x1 = user->get_x_coord();
		for (int j = 0; j < sensors.size(); j++) {
			node2 = sensors.at(j);

			y2 = node2->get_y_coord();
			x2 = node2->get_x_coord();

			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

			if (distance <= RAY_LENGTH * SPACE_PRECISION) {
				user->add_near_sensor_node((SensorNode*)node2);
			}
		}

		for (int j = 0; j < storage_nodes.size(); j++) {
			node2 = storage_nodes.at(j);

			y2 = node2->get_y_coord();
			x2 = node2->get_x_coord();

			distance = sqrt(pow(y1 - y2, 2) + pow(x1 - x2, 2));

			if (distance <= RAY_LENGTH * SPACE_PRECISION) {
				user->add_near_storage_node((StorageNode*)node2);
			}
		}
	}

	





/*
		TEST DI CORRETTO FUNZIONAMENTO
*/
///////////////////////////////////////////////////////////////////////////////
	StorageNode *storage_1 = storage_nodes.at(0);
	storage_1->activate_node();

	StorageNode *storage_2 = storage_nodes.at(1);
	storage_2->activate_node();

	StorageNode *storage_3 = storage_nodes.at(2);
	storage_3->activate_node();

	StorageNode *storage_4 = storage_nodes.at(3);
	storage_4->activate_node();

	StorageNode *storage_5 = storage_nodes.at(4);
	storage_5->activate_node();

	SensorNode *sensor_1 = sensors.at(0);
	sensor_1->activate_sensor();

	SensorNode *sensor_2 = sensors.at(1);
	sensor_2->activate_sensor();

	SensorNode *sensor_3 = sensors.at(1);
	sensor_3->activate_sensor();

	



	

	this_thread::sleep_for(chrono::seconds(10));

	User *user = users.at(0);
	user->switch_on_user();
	user->switch_off_user();

  storage_1->stop_node();
  storage_2->stop_node();
  storage_3->stop_node();
  storage_4->stop_node();
  storage_5->stop_node();
  sensor_1->stop_sensor();
  sensor_2->stop_sensor();
  sensor_3->stop_sensor();




///////////////////////////////////////////////////////////////////////////////
		// MyToolbox::set_k(4);
// int d = MyToolbox::get_ideal_soliton_distribution_degree();
// cout << d << endl;


		
///////////////////////////////////////////////////////////////////////////////
		// MESSAGE PASSING TEST

		// User user(0);

		// /*
		// s1 = 100 = 4		s4 = 101 = 5
		// s2 = 010 = 2    s5 = 110 = 6
		// s3 = 001 = 1
		// */

		// // t1 = s2 + s3 = 010 + 001 = 011 = 3
		// Message t1(3);
		// vector<int> header1;
		// header1.push_back(2);
		// header1.push_back(3);
		// t1.header_ = header1;

		// // t2 = s1 + s4 + s5 = 100 + 101 + 110 = 111 = 7
		// Message t2(7);
		// vector<int> header2;
		// header2.push_back(1);
		// header2.push_back(4);
		// header2.push_back(5);
		// t2.header_ = header2;

		// // t3 = s2 = 010 = 2
		// Message t3(2);
		// vector<int> header3;
		// header3.push_back(2);
		// t3.header_ = header3;

		// // t4 = s1 + s2 = 100 + 010 = 110 = 6
		// Message t4(6);
		// vector<int> header4;
		// header4.push_back(1);
		// header4.push_back(2);
		// t4.header_ = header4;

		// // t5 = s4 + s5 = 101 + 110 = 011 = 3
		// Message t5(3);
		// vector<int> header5;
		// header5.push_back(4);
		// header5.push_back(5);
		// t5.header_ = header5;

		// // t6 = s1 + s5 = 100 + 110 = 010 = 2
		// Message t6(2);
		// vector<int> header6;
		// header6.push_back(1);
		// header6.push_back(5);
		// t6.header_ = header6;

		// // t7 = s3 + s4 + s5 = 001 + 101 + 110 = 010 = 2
		// Message t7(2);
		// vector<int> header7;
		// header7.push_back(3);
		// header7.push_back(4);
		// header7.push_back(5);
		// t7.header_ = header7;

		// // t8 = s5 = 110 = 6
		// Message t8(6);
		// vector<int> header8;
		// header7.push_back(5);
		// t8.header_ = header8;

		// user.collect_data(t1);
		// user.collect_data(t2);
		// user.collect_data(t3);
		// user.collect_data(t4);
		// user.collect_data(t5);
		// user.collect_data(t6);
		// user.collect_data(t7);
		// user.collect_data(t8);
		// user.message_passing();

///////////////////////////////////////////////////////////////////////////////
		// /*
		// s1 = 100 = 4		s4 = 101 = 5
		// s2 = 010 = 2    s5 = 000 = 0
		// s3 = 001 = 1
		// */

		// User user(0);

		// // Create 7 output symbols:
		// // t1 = s1 + s2 = 100 + 010 = 110 = 6
		// Message t1(6);
		// vector<int> header1;
		// header1.push_back(1);
		// header1.push_back(2);
		// t1.header_ = header1;

		// // t2 = s2 = 010 = 2
		// Message t2(2);
		// vector<int> header2;
		// header2.push_back(2);
		// t2.header_ = header2;

		// // t3 = s5 = 000 = 0
		// Message t3(0);
		// vector<int> header3;
		// header3.push_back(5);
		// t3.header_ = header3;

		// // t4 = s3 + s5 = 001 + 000 = 001 = 1
		// Message t4(1);
		// vector<int> header4;
		// header4.push_back(3);
		// header4.push_back(5);
		// t4.header_ = header4;

		// // t5 = s1 + s3 = 100 + 001 = 101 = 5
		// Message t5(5);
		// vector<int> header5;
		// header5.push_back(1);
		// header5.push_back(3);
		// t5.header_ = header5;

		// // t6 = s1 + s4 = 100 + 101 = 001 = 1
		// Message t6(1);
		// vector<int> header6;
		// header6.push_back(1);
		// header6.push_back(4);
		// t6.header_ = header6;

		// // t7 = s1 + s2 = 100 + 010 = 110 = 6
		// Message t7(6);
		// vector<int> header7;
		// header7.push_back(1);
		// header7.push_back(2);
		// t7.header_ = header7;

		// user.collect_data(t1);
		// user.collect_data(t2);
		// user.collect_data(t3);
		// user.collect_data(t4);
		// user.collect_data(t5);
		// user.collect_data(t6);
		// user.collect_data(t7);
		// user.message_passing();

///////////////////////////////////////////////////////////////////////////////
	// SensorNode *sensor = sensors.at(0);
	// // sensor->activate_sensor(chrono::milliseconds(1000), []() {});
	// StorageNode *sto = (StorageNode*)(sensor->near_storage_nodes.at(0));		
	// // cout << "(Richiamo) Message node " << sto->get_node_id() << ": " << static_cast<unsigned>(sto->get_xored_message()) << endl;
	// // cout << sto->get_node_id() << endl;
	// sto->transfer_message(6);
	// sto->transfer_message(3);

	// this_thread::sleep_for(chrono::seconds(5));
	// sensor->stop_sensor();
///////////////////////////////////////////////////////////////////////////////

	// for (Node *node : all_nodes) {
	// 	cout << "Nodo numero: " << node->get_node_id() << endl;
	// 	if (node->near_sensor_nodes.size() > 0) {
	// 		cout << "Sensori vicini: " << endl;
	// 		for (Node near_sensor : node->near_sensor_nodes) {
	// 			cout << near_sensor.get_node_id() << endl;
	// 		}
	// 	} else {
	// 		cout << "No sensori vicini!" << endl;
	// 	}
	// 	if (node->near_storage_nodes.size() > 0) {
	// 		cout << "Nodi storage vicini: " << endl;
	// 		for (Node near_storage : node->near_storage_nodes) {
	// 			cout << near_storage.get_node_id() << endl;
	// 		}
	// 	} else {
	// 		cout << "No nodi storage vicini!" << endl;
	// 	}
	// }
///////////////////////////////////////////////////////////////////////////////

	// for (int i = 0; i < NUM_SENSORS; i++) {
	// for (std::vector<SensorNode>::iterator it = sensors.begin(); it != sensors.end(); ++it) {
	// 	SensorNode node = *it;
	// 	double y_coord = node.get_y_coord();
	// 	double x_coord = node.get_x_coord();
	// 	// cout << y_coord << ", " << x_coord << endl;
	// 	printf("y coord: %4.2f, x coord: %4.2f \n", y_coord, x_coord);
	// }

	// // for (int i = 0; i < NUM_SENSORS; i++) {
	// for (std::vector<StorageNode>::iterator it = storage_nodes.begin(); it != storage_nodes.end(); ++it) {
	// 	StorageNode node = *it;
	// 	double y_coord = node.get_y_coord();
	// 	double x_coord = node.get_x_coord();
	// 	// cout << y_coord << ", " << x_coord << endl;
	// 	printf("y coord: %4.2f, x coord: %4.2f \n", y_coord, x_coord);
	// }

///////////////////////////////////////////////////////////////////////////////

/*
Test for threads of sensors
*/
	// SensorNode sensor_1(++node_id);
	// sensor_1.activate_sensor(chrono::milliseconds(1000), [&sensor_1]() { 	// mi serve [&] perche' sto chiamando sensor_1
	// 	cout << "Measure generated by node: " << sensor_1.get_node_id() << endl;
	// });

	// SensorNode sensor_2(++node_id);
	// sensor_2.activate_sensor(chrono::milliseconds(500), [&sensor_2]() {
	// 	cout << "Measure generated by node: " << sensor_2.get_node_id() << endl;
	// });

	// this_thread::sleep_for(chrono::seconds(5));
 //  sensor_1.stop_sensor();
 //  sensor_2.stop_sensor();
///////////////////////////////////////////////////////////////////////////////



 	return 0;
}
