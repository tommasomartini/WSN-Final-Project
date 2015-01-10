/*  Idea: un evento ha un agente, una funzione che questo agente deve eseguire e dei parametri.
    Quando chiamo la funzione, restituisco un altro evento da inserire nella lista di eventi.
*/

#ifndef __EVENT_H__   // if x.h hasn't been included yet...
#define __EVENT_H__   //   #define this so the compiler knows it has been included

#include "agent.h"

class Event {
  int time_;
  Agent agent_;

 public:
  Event(int /*time*/);

  bool operator<(Event);
  bool operator>(Event);

  int get_time() {return time_;}
  Agent get_agent() {return agent_;}
  void set_agent(Agent);
  Event execute_action();
};

#endif
