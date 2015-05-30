#ifndef __EVENT_H__   
#define __EVENT_H__   

#include <vector>

#include "my_toolbox.h"

class Message;

class Event {

 public:
  enum EventTypes {
    event_type_generated_measure,
    event_type_sensor_re_send,
    event_type_cache_re_send,
    event_type_cache_receives_measure,
    event_type_cache_receives_user_request,
    event_type_sensor_breaks,
    event_type_user_re_send,
	event_type_cache_gets_user_hello,
	event_type_user_gets_user_hello,
    event_type_cache_gets_blacklist,
    event_type_sensor_ping,
    event_type_cache_checks_sensors,
    event_type_cache_receives_user_info,
    event_type_user_moves,
    event_type_user_receives_node_data,
	event_type_user_receives_user_data,
	event_type_user_receives_user_request,
	event_type_null
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
