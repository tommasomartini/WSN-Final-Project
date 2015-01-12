#include <stdlib.h>     /* srand, rand */

#include "event.h"

Event::Event(int event_time) {
  time_ = event_time;
}

bool Event::operator<(Event event) {
  return time_ < event.get_time();
}

bool Event::operator>(Event event) {
  return time_ > event.get_time();
}

void Event::set_agent(Agent agent) {
  agent_ = agent;
    // add
}

Event Event::execute_action() {
  // guarda di che tipo sono switch
  int event_time = rand() % 100;
  return Event(event_time); 
}