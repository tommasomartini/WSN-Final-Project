#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>

#include "event.h"
#include "sensor_node.h"
#include "my_toolbox.h"

Event::Event(int event_time) {
  time_ = event_time;
}

Event::Event(int event_time, Event::EventTypes event_type) {
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

void Event::set_message(Message message) {
  message_ = message;
}

void Event::set_blacklist(BlacklistMessage blacklist) {
  list_ = blacklist;
}

Event Event::execute_action() {
  // guarda di che tipo sono switch
  // se il tipo e' nodo manda misura a un altro nodo
  // sappiamo che agent_ e' di tipo StoraegNode
  // storagenodo nodo42 = agent;
  // evento = nodo42.gestisciLa Misura();

  MyToolbox::set_current_time(time_);

  Event new_event = Event(-1);
  vector<Event> new_events;

  int test = 0;

  switch(event_type_) {
    case sensor_generate_measure:
      // cout << "Sensor id: " << ((SensorNode*)agent_)->get_node_id() << endl;
      new_events = ((SensorNode*)agent_)->generate_measure();
      // test += agent_->do_action();
      // cout << "Risultato test: " << test << endl;
      // cout << "New event time: " << new_event.get_time() << endl; 
      // cout << "New agent id: " << ((StorageNode*)new_event.get_agent())->get_node_id() << endl; 
      break;
    case spread_measure:
      break;
    case blacklist_sensor:
        new_events = ((StorageNode*)agent_)->spread_blacklist(time_, list_);
        // cout <<"Il nuovo evento creato da blacklist è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
        break;
    case remove_measure:
      break;
    case user_node_query:
      break;
    case user_user_query:
      break;
    case new_storage_node:
      break;
    case sensor_ping:
        new_events = ((SensorNode*)agent_)->sensor_ping(time_);
        //cout <<"Il nuovo evento creato è al tempo "<<new_events.at(0).get_time()<<endl;
      break;
    case check_sensors:
        new_events = ((StorageNode*)agent_)->check_sensors(time_);
        //cout <<"Il nuovo evento creato da check è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
      break;
    case remove_sensor:
      break;
    case add_sensor:
      break;
    case remove_node:
      break;
    case add_node:
      break;
    case network_reset:
      break;
    default:
      break;  // remove this break! No break in the default option!
  }

  int event_time = rand() % 100;
  return Event(event_time); 
}