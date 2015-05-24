#ifndef __EVENT_H__   
#define __EVENT_H__   

#include <vector>

#include "my_toolbox.h"

class Message;

class Event {

 public:
  enum EventTypes {
    // Tom
    sensor_generate_measure,
    sensor_try_to_send,
    storage_node_try_to_send,
    storage_node_receive_measure,
    node_send_to_user,
    broken_sensor,
    user_try_to_send,
    user_try_to_send_to_user,
	storage_get_reinit_query,
	storage_get_reinit_response,
	storage_get_user_hello,
	user_get_user_hello,

    // Arianna
    blacklist_sensor,
    sensor_ping,
    check_sensors,
    remove_measure,
    move_user,
    user_send_to_user,
    user_receive_data,

    // To do
    new_storage_node,
    add_sensor,
    remove_node,

    // Not to do
    network_reset
  };

  static std::string int2type(int);

 private:
  typedef MyToolbox::MyTime MyTime;

  MyTime time_;
  Agent* agent_;
  Message* message_;
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
  void set_time(MyTime);
  void set_agent(Agent*);
  void set_message(Message*);
  
  // functions
  std::vector<Event> execute_action();
};

#endif
