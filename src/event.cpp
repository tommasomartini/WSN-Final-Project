#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <vector>

#include "event.h"
#include "sensor_node.h"
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

void Event::set_message(Message message) {
  message_ = message;
}

void Event::set_blacklist(BlacklistMessage blacklist) {
  list_ = blacklist;
}

vector<Event> Event::execute_action() {
  // guarda di che tipo sono switch
  // se il tipo e' nodo manda misura a un altro nodo
  // sappiamo che agent_ e' di tipo StoraegNode
  // storagenodo nodo42 = agent;
  // evento = nodo42.gestisciLa Misura();

  MyToolbox::set_current_time(time_); // keep track of the time flowing by. I must know what time it is in every moment

  // Event new_event = Event(-1);
  vector<Event> new_events;

  Measure mm = (Measure)message_;

  // cout << "vediamo: " << ((Measure)message_).get_measure_id() << endl;
  // int test = 0;

  switch (event_type_) {
    case sensor_generate_measure:
      // cout << "Sensor id: " << ((SensorNode*)agent_)->get_node_id() << endl;

      /*  generate_measure() should return 2 events:
          - a new measure generation of the same node
          - the reception of the measure to a storage node
      */ 
      new_events = ((SensorNode*)agent_)->generate_measure();
      // test += agent_->do_action();
      // cout << "Risultato test: " << test << endl;
      // cout << "New event time: " << new_event.get_time() << endl; 
      // cout << "New agent id: " << ((StorageNode*)new_event.get_agent())->get_node_id() << endl; 
      break;
    case storage_node_receive_measure:
      // new_events = ((StorageNode*)agent_)->manage_measure((Measure)message_));
      break;
    case spread_measure:
      break;
    case blacklist_sensor:
        new_events = ((StorageNode*)agent_)->spread_blacklist(time_, list_);
        // cout <<"Il nuovo evento creato da blacklist è al tempo "<<new_events.at(0).get_time()<<"ed è di tipo"<<new_events.at(0).event_type_<<endl;
        break;
    case remove_measure:
    {
        unsigned char message_to_remove = 0;    // da sistemare
        int id_to_remove = 1;                   // da sistemare
        Measure mess (message_to_remove);       // da sistemare
        mess.set_source_sensor_id(id_to_remove);// da sistemare
        new_events = ((StorageNode*)agent_)->remove_mesure(mess);
      break;
    }
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

  return new_events; 
}