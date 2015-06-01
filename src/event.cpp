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
	agent_id_ = 0;
	message_ = nullptr;
}

Event::Event(MyTime event_time, Event::EventTypes event_type) {
	time_ = event_time;
	event_type_ = event_type;
	agent_ = nullptr;
	agent_id_ = 0;
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

void Event::set_agent_id(unsigned int agent_id) {
	agent_id_ = agent_id;
}

void Event::set_message(Message* message) {
	message_ = message;
}

vector<Event> Event::execute_action() {
	MyToolbox::current_time_ = time_; // keep track of the time flowing by. I must know what time it is in every moment
	vector<Event> new_events;

	// check whether the agent supposed to execute this action is still existing
	if (!MyToolbox::is_node_active(agent_id_)) {
//		cout << "Skip this event (type: " << event_type_ << "); unknown agent: " << agent_id_ << "!" << endl;
		return new_events;
	}

	bool debug = false;

	switch (event_type_) {
	case event_type_generated_measure: {
		if(debug)cout << "=== generate measure" << endl;
		new_events = ((SensorNode*)agent_)->generate_measure();
		break;
	}
	case event_type_sensor_re_send: {
		if(debug) cout << "=== event_type_sensor_re_send" << endl;
		new_events = ((SensorNode*)agent_)->try_retx(message_);
		break;
	}
	case event_type_cache_receives_measure: {
		if(debug)cout << "=== event_type_cache receives measure " << ((StorageNode*)agent_)->get_node_id() << endl;
		new_events = ((StorageNode*)agent_)->receive_measure2((Measure*)message_);
		break;
	}
	case event_type_cache_re_send: {
		if (debug) cout << "=== event_type_cache resend " << ((StorageNode*)agent_)->get_node_id() << endl;
		new_events = ((StorageNode*)agent_)->try_retx(message_);
		break;
	}
	case event_type_cache_gets_blacklist: {
		if(debug)cout << "=== event_type_gets blacklist" << endl;
		new_events = ((StorageNode*)agent_)->spread_blacklist((BlacklistMessage*)message_);
		break;
	}
	case event_type_cache_receives_user_info: {
		if(debug)cout << "=== event_type_cache_receives_user_info" << endl;
		((StorageNode*)agent_)->refresh_xored_data2((OutdatedMeasure*)message_);
		break;
	}
	case event_type_user_moves: {
		if(debug)cout << "=== event_type_user_moves " << ((User*)agent_)->get_node_id() << endl;
		new_events = ((User*)agent_)->move();
		break;
	}
	case event_type_cache_receives_user_request: {
		if(debug)cout << "=== event_type_cache_receives_user_request" << endl;
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		delete message_;
		break;
	}
	case event_type_user_receives_user_request: {
		if(debug)cout << "=== event_type_user_receives_user_request" << endl;
		new_events = ((User*)agent_)->receive_user_request(message_->get_sender_node_id());
		delete message_;
		break;
	}
	case event_type_user_receives_user_data: {
		if(debug)cout << "=== event_type_user_receives_user_data" << endl;
		new_events = ((User*)agent_)->receive_user_data((UserInfoMessage*)message_);
		break;
	}
	case event_type_user_re_send: {
		if(debug)cout << "=== event_type_user_resend " << ((User*)agent_)->get_node_id() << endl;
		new_events = ((User*)agent_)->try_retx(message_);
		break;
	}
	case event_type_user_receives_node_data: {
		if(debug)cout << "=== event_type_user_receives_node_data" << endl;
		new_events = ((User*)agent_)->receive_node_data((NodeInfoMessage*)message_);
		break;
	}
	case event_type_sensor_ping: {
		if(debug)cout << "=== event_type_ping" << endl;
		new_events = ((SensorNode*)agent_)->ping();
		break;
	}
	case event_type_cache_checks_sensors: {
		if(debug)cout << "=== event_type_check sensors" << endl;
		new_events = ((StorageNode*)agent_)->check_sensors();
		break;
	}
	case event_type_sensor_breaks: {
		if(debug)cout << "=== event_type_sensor_breaks" << endl;
		((SensorNode*)agent_)->breakup();
		break;
	}
	case event_type_cache_gets_user_hello: {
		if(debug)cout << "=== event_type_cache_gets_user_hello" << endl;
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	case event_type_user_gets_user_hello: {
		if(debug)cout << "=== event_type_user_gets_user_hello" << endl;
		new_events = ((User*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	default:
		break;  // remove this break! No break in the default option!
	}

	return new_events;
}
