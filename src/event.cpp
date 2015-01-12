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
  int event_time = rand() % 100;
  return Event(event_time); 
}