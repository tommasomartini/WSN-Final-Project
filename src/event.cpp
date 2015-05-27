#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>

#include "event.h"
#include "sensor_node.h"
#include "measure.h"
#include "storage_node.h"
#include "measure.h"
#include "user.h"
#include "outdated_measure.h"
#include "node_info_message.h"
#include "user_info_message.h"

using namespace std;


Event::Event(MyTime event_time) {
	event_type_ = Event::event_type_null;
	time_ = event_time;
	agent_ = nullptr;
	message_ = nullptr;
}

Event::Event(MyTime event_time, Event::EventTypes event_type) {
	time_ = event_time;
	event_type_ = event_type;
	agent_ = nullptr;
	message_ = nullptr;
}

bool Event::operator<(Event event) {
	return time_ < event.get_time();
}

bool Event::operator>(Event event) {
	return time_ > event.get_time();
}

void Event::set_time(MyTime time) {
	time_ = time;
}

void Event::set_agent(Agent *agent) {
	agent_ = agent;
}

void Event::set_message(Message* message) {
	message_ = message;
}

string Event::int2type(int num) {
	string res = "unknonw";
	switch (num) {
	case 0:
		res = "sensor_generate_measure";
		break;
	case 1:
		res = "sensor_try_to_send";
		break;
	case 2:
		res = "storage_node_try_to_send";
		break;
	case 3:
		res = "storage_node_receive_measure";
		break;
	case 4:
		res = "node_send_to_user";
		break;
	case 5:
		res = "broken_sensor";
		break;
	case 6:
		res = "user_try_to_send";
		break;
	case 7:
		res = "user_try_to_send_to_user";
		break;
	case 8:
		res = "blacklist_sensor";
		break;
	case 9:
		res = "sensor_ping";
		break;
	case 10:
		res = "check_sensors";
		break;
	case 11:
		res = "remove_measure";
		break;
	case 12:
		res = "move_user";
		break;
	case 13:
		res = "user_send_to_user";
		break;
	case 14:
		res = "user_receive_data";
		break;
	case 15:
		res = "new_storage_node";
		break;
	case 16:
		res = "add_sensor";
		break;
	case 17:
		res = "remove_node";
		break;
	default:
		res = "unknown";
		break;
	}
	return res;
}


vector<Event> Event::execute_action() {

	//  unsigned int gh_id = ((Node*)agent_)->get_node_id();
	//  cout << "EXECUTION: " << MyToolbox::int2nodetype(gh_id) << " " << gh_id << ", event type: " << int2type(event_type_) << ", current time: " << time_ << endl;

	MyToolbox::current_time_ = time_; // keep track of the time flowing by. I must know what time it is in every moment
	vector<Event> new_events;

	// check whether the agent supposed to execute this action is still existing
	Node* current_agent = (Node*)agent_;
	unsigned int current_node_id = current_agent->get_node_id();
	if (!MyToolbox::is_node_active(current_node_id)) {
		cout << "Skip this event: dead agent!" << endl;
		return new_events;
	}

	switch (event_type_) {
	case event_type_generated_measure: {
		new_events = ((SensorNode*)agent_)->generate_measure();
		break;
	}
	case event_type_sensor_re_send: {
		new_events = ((SensorNode*)agent_)->try_retx(message_);
		break;
	}
	case event_type_cache_receives_measure: {
		new_events = ((StorageNode*)agent_)->receive_measure((Measure*)message_);
		break;
	}
	case event_type_cache_re_send: {
		new_events = ((StorageNode*)agent_)->try_retx(message_);
		break;
	}
	case event_type_cache_gets_blacklist: {
		new_events = ((StorageNode*)agent_)->spread_blacklist((BlacklistMessage*)message_);
		break;
	}
	case event_type_cache_receives_user_info: {
		((StorageNode*)agent_)->refresh_xored_data((OutdatedMeasure*)message_);
		break;
	}
	case event_type_user_moves: {
		new_events = ((User*)agent_)->move();
		break;
	}
	case event_type_cache_receives_user_request: {
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		delete message_;
		break;
	}
	case event_type_user_receives_user_request: {
		new_events = ((User*)agent_)->receive_user_request(message_->get_sender_node_id());
		delete message_;
		break;
	}
	case event_type_user_receives_user_data: {
		new_events = ((User*)agent_)->receive_user_data((UserInfoMessage*)message_);
		break;
	}
	case event_type_user_re_send: {
		new_events = ((User*)agent_)->try_retx(message_);
		break;
	}
	case event_type_user_receives_node_data: {
		new_events = ((User*)agent_)->receive_node_data((NodeInfoMessage*)message_);
		break;
	}
	case event_type_sensor_ping: {
		new_events = ((SensorNode*)agent_)->ping();
		break;
	}
	case event_type_cache_checks_sensors: {
		new_events = ((StorageNode*)agent_)->check_sensors();
		break;
	}
	case event_type_sensor_breaks: {
		((SensorNode*)agent_)->breakup();
		break;
	}
	case event_type_cache_gets_user_hello: {
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	case event_type_user_gets_user_hello: {
		new_events = ((User*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	default:
		break;  // remove this break! No break in the default option!
	}

	return new_events;
}
