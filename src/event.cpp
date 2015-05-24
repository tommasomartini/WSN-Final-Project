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

using namespace std;


Event::Event(MyTime event_time) {
	time_ = event_time;
}

Event::Event(MyTime event_time, Event::EventTypes event_type) {
	time_ = event_time;
	event_type_ = event_type;
	agent_ = NULL;
	message_ = NULL;
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
		cout << "esecuzione bloccata:il nodo è morto!" << endl;
		return new_events;
	}

	switch (event_type_) {
	case sensor_generate_measure: {
		/*  generate_measure() should return 2 events:
          - a new measure generation of the same node OR the sensor's failure
          - the reception of the measure to a storage node
		 */
		new_events = ((SensorNode*)agent_)->generate_measure();
		break;
	}
	case sensor_try_to_send: {
		new_events = ((SensorNode*)agent_)->try_retx(message_);
		break;
	}
	case storage_node_receive_measure: {
		new_events = ((StorageNode*)agent_)->receive_measure((Measure*)message_);
		break;
	}
	case storage_node_try_to_send: {
		new_events = ((StorageNode*)agent_)->try_retx(message_);
		break;
	}
	case blacklist_sensor: {
		new_events = ((StorageNode*)agent_)->spread_blacklist((BlacklistMessage*)message_);
		// cout <<"Il nuovo evento creato da blacklist è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
		break;
	}
	case remove_measure: {
		new_events = ((StorageNode*)agent_)->remove_mesure((OutdatedMeasure*)message_);
		break;
	}
	case move_user: {
		new_events = ((User*)agent_)->move();
		//new_events = ((User*)agent_)->move_user(time_);
		cout<<"evento "<<new_events.at(new_events.size()-1).get_event_type()<<endl;
		break;
	}
	case node_send_to_user: {
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	case user_send_to_user: {
		new_events = ((User*)agent_)->user_send_to_user(message_->get_sender_node_id());
		break;
	}
	case user_try_to_send: {
		int next_node_id = message_->get_receiver_node_id();
		new_events = ((User*)agent_)->try_retx(message_, next_node_id);
		break;
	}
	case user_try_to_send_to_user: {
		int next_node_id = message_->get_receiver_node_id();
		new_events = ((User*)agent_)->try_retx_to_user(message_, next_node_id);
		break;
	}
	case user_receive_data: {
		//         new_events = ((User*)agent_)->receive_data((NodeInfoMessage)*(message_));
		break;
	}
	case sensor_ping: {
		new_events = ((SensorNode*)agent_)->sensor_ping2();
		//cout <<"Il nuovo evento creato è al tempo "<<new_events.at(0).get_time()<<endl;
		break;
	}
	case check_sensors: {
		new_events = ((StorageNode*)agent_)->check_sensors();
		//      for (int tt = 0; tt < new_events.size(); tt++) {
		//    	  cout << "--" << Event::int2type(new_events.at(tt).get_event_type()) << endl;
		//      }
		//      if (new_events.size() > 1 && new_events.at(0).event_type_ == EventTypes::blacklist_sensor) {
		//    	  BlacklistMessage* bbl = (BlacklistMessage*)(new_events.at(0).get_message());
		//    	  cout << "-----------------------dentro forevent.cpp:" << bbl->get_id_list()->size() << endl;
		//    	  for (int jj = 0; jj < 10; jj++) {
		//    		  cout << bbl->get_id_list()->at(jj) << endl;
		//    	  }
		//      }
		//cout <<"Il nuovo evento creato da check è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
		break;
	}
	case broken_sensor: {
		cout << "Sensor " << ((Node*)agent_)->get_node_id() << " broken, time " << MyToolbox::current_time_ << endl;
		//      MyToolbox::remove_sensor(((Node*)agent_)->get_node_id());
		break;
	}
	case storage_get_user_hello: {
		new_events = ((StorageNode*)agent_)->receive_user_request(message_->get_sender_node_id());
		break;
	}
	case user_get_user_hello: {
		new_events = ((User*)agent_)->user_send_to_user(message_->get_sender_node_id());
		break;
	}
	default:
		break;  // remove this break! No break in the default option!
	}


	//  // DEBUGGING
	//  cout << "=== Nuovi eventi da inserire in lista:" << endl;
	//  for (unsigned int i = 0; i < new_events.size(); i++) {
	//    Event event = new_events.at(i);
	//    unsigned int df_id = ((Node*)(event.get_agent()))->get_node_id();
	//    cout << MyToolbox::int2nodetype(df_id) << " " << df_id << ", event type: " << int2type(event.get_event_type()) << ", time: " << event.get_time() << endl;
	//  }

	//  for (unsigned int i = 0; i < new_events.size(); i++) {
	//	  Event ev = new_events.at(i);
	//	  if (ev.event_type_ == EventTypes::blacklist_sensor) {
	//		  cout << "(add event) list size " << ((BlacklistMessage*)ev.message_)->get_id_list()->size() << endl;
	//	  }
	//  }

	return new_events;
}
