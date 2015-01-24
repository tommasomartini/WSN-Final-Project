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
#include "blacklist_message.h"
// #include "my_toolbox.h"


Event::Event(MyTime event_time) {
  time_ = event_time;
}

Event::Event(MyTime event_time, Event::EventTypes event_type) {
  time_ = event_time;
  event_type_ = event_type;
}

bool Event::operator<(Event event) {
  return time_ < event.get_time();
}

bool Event::operator>(Event event) {
  return time_ > event.get_time();
}

void Event::set_agent(Agent *agent) {
  agent_ = agent;
}

void Event::set_message(Message *message) {
  message_ = message;
}


vector<Event> Event::execute_action() {

  MyToolbox::set_current_time(time_); // keep track of the time flowing by. I must know what time it is in every moment
  vector<Event> new_events;

  switch (event_type_) {
    case sensor_generate_measure: {
      /*  generate_measure() should return 2 events:
          - a new measure generation of the same node
          - the reception of the measure to a storage node
      */ 
      new_events = ((SensorNode*)agent_)->generate_measure(); 
      break;
    }
    case sensor_try_to_send: {
      new_events = ((SensorNode*)agent_)->try_retx(message_, message_->get_receiver_node_id()); 
      break;
    }
    case storage_node_receive_measure: {
      new_events = ((StorageNode*)agent_)->manage_measure((Measure*)message_);
      break;
    }
    case storage_node_try_to_send_measure: {
      int next_node_id = message_->get_receiver_node_id();
      new_events = ((StorageNode*)agent_)->try_retx_measure((Measure*)message_, next_node_id);
      break;
    }
    case blacklist_sensor:
        new_events = ((StorageNode*)agent_)->spread_blacklist(time_, (BlacklistMessage*)message_);
        // cout <<"Il nuovo evento creato da blacklist è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
        break;
    case remove_measure:
        new_events = ((StorageNode*)agent_)->remove_mesure((OutdatedMeasure*)message_);
      break;
    case move_user:
        new_events = ((User*)agent_)->move_user(time_);
        cout<<"evento "<<new_events.at(new_events.size()-1).get_event_type()<<endl;
      break;
    case node_send_to_user:
      break;
    case user_send_to_user:
         new_events = ((User*)agent_)->user_send_to_user((UserMessage*)message_,time_);
      break;
      case user_receive_data:
         new_events = ((User*)agent_)->user_receive_data(time_,(UserMessage*)message_);
      break;
    case new_storage_node:
      break;
    case sensor_ping: {
      new_events = ((SensorNode*)agent_)->sensor_ping(time_);
      //cout <<"Il nuovo evento creato è al tempo "<<new_events.at(0).get_time()<<endl;
      break;
    }
    case check_sensors: {
      new_events = ((StorageNode*)agent_)->check_sensors(time_);
      //cout <<"Il nuovo evento creato da check è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
      break;
    }
    default:
      break;  // remove this break! No break in the default option!
  }

  return new_events; 
}