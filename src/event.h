#ifndef __EVENT_H__   
#define __EVENT_H__   

#include <vector>

#include "agent.h"
#include "message.h"
#include "my_toolbox.h"
#include "blacklist_message.h"

using namespace std;

class Event {

 public:
  enum EventTypes {
    // Tom
    sensor_generate_measure,
    sensor_try_to_send,
    storage_node_try_to_send,
    storage_node_try_to_send_measure, // TO DEPRECATE
    storage_node_receive_measure,

    // Arianna
    blacklist_sensor,
    sensor_ping,
    check_sensors,
    remove_measure,
    move_user,
    user_send_to_user,
    user_receive_data,

    // To do
    node_send_to_user,
    new_storage_node,
    remove_sensor,
    add_sensor,
    remove_node,
    add_node,

    // Not to do
    network_reset
  };

 private:
  typedef MyToolbox::MyTime MyTime;

  int time_;
  Agent *agent_;
  Message *message_;
  Event::EventTypes event_type_;

 public:
  Event(MyTime /*time*/);
  Event(MyTime /*time*/, Event::EventTypes);

  bool operator<(Event);
  bool operator>(Event);

  // getters
  MyTime get_time() {return time_;}
  Agent* get_agent() {return agent_;}
  Message* get_message() {return message_;}
  EventTypes get_event_type() {return event_type_;}

  // setters
  void set_agent(Agent*);
  void set_message(Message*);
  
  // functions
  vector<Event> execute_action();
};

#endif
