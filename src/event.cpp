#include <stdlib.h>     /* srand, rand */

#include "event.h"

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

void Event::set_agent(Agent agent) {
  agent_ = agent;
}

Event Event::execute_action() {
  // guarda di che tipo sono switch
  // se il tipo e' nodo manda misura a un altro nodo
  // sappiamo che agent_ e' di tipo StoraegNode
  // storagenodo nodo42 = agent;
  // evento = nodo42.gestisciLa Misura();

  switch(event_type_) {
    case spread_measure:
      break;
    case blacklist_sensor:
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