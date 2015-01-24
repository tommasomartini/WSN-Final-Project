/*  Idea: un evento ha un agente, una funzione che questo agente deve eseguire e dei parametri.
    Quando chiamo la funzione, restituisco un altro evento da inserire nella lista di eventi.

    Tipi di eventi:
    - generazione di una misura di un sensore e diffusione al primo nodo
    - diffusione di un messaggio:
      - misura (nuova o update)
      - blacklist nodo
      - rimozione misura nodo rimosso
      - comunicazione utente-nodo
      - comunicazione utente-utente
      - richiesta XOR da parte di un nuovo nodo
      - ping sensore -> faccio partire un timer sul nodo. allo scadere del timer il nodo si accorge che il sensore non c'e' piu'
    - rimozione sensore (tra 200ms il nodo incaaricato si rende conto)
            OPPURE
    - nodo dedicato si rende conto sensore sparito
    - nuovo sensore
    - rimozione nodo -> non succede nulla
    - nuovo nodo cache -> messaggio broadcast! genero tanti messaggi! Aspetto un certo tempo per l'inizializzazione? come so che nodi ho attorno?
        e se i nodi attorno stanno gia' trasmettendo? Meglio mettere un tempo limite, dopo il quale faccio lo XOR dei nodi ricevuti, se 
        ne ho qlcn, altrimenti ripeto! 

        E SE DUE NODI SI PARLANO SOPRA???

    - movimento utente (arrivo utente in un punto, va bene anche se e' la prima volta che l'utente compare)
    - reset rete
*/

#ifndef __EVENT_H__   
#define __EVENT_H__   

#include <vector>

#include "agent.h"
#include "message.h"
#include "my_toolbox.h"
#include "blacklist_message.h"

    // #include "node.h"

using namespace std;

class Event {

 public:
  enum EventTypes {
    // Tom
    sensor_generate_measure,
    sensor_try_to_send,
    storage_node_try_to_send_measure,
    storage_node_receive_measure,

    // Arianna
    blacklist_sensor,
    sensor_ping,
    check_sensors,
    remove_measure,
    move_user,
    user_send_to_user,

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
  BlacklistMessage list_;
  Agent *agent_to_reply_;

 public:
  Event(MyTime /*time*/);
  Event(MyTime /*time*/, Event::EventTypes);

  bool operator<(Event);
  bool operator>(Event);

  MyTime get_time() {return time_;}
  Agent* get_agent() {return agent_;}
  Message* get_message() {return message_;}
  EventTypes get_event_type() {return event_type_;}
  BlacklistMessage get_blacklist() {return list_;}
  Agent* get_user_to_reply() {return agent_to_reply_;}

  void set_agent(Agent*);
  void set_message(Message*);
  void set_blacklist(BlacklistMessage);
  void set_agent_to_reply(Agent*);
  
  vector<Event> execute_action();


  /*
      guardo il tipo di evento e in base a questo chiamo un certo metodo su Agent_1
      Agent_1 ritorna un nuovo eveto o lo so io?
        se si tratta di un passaggio di messaggio posso anche creare qui il nuovo evento!

      esempi:
      SENSORE GENERA MISURA
        Agent_ e' un sensore -> faccio il cast, chiamo il metodo generateMeasure() del sensore.
        il sensore avra' dei metodi che decidono il nodo di arrivo, la misura e l'id della misura.
        Anzi: il sensore restituisce direttam un oggetto misura!
        mi faccio restituire queste info dal sensore e costruisco un nuovo evento, che ha come agent_ il 
        nodo indirizzato, come messaggioo un passaggio di misura che contiene la misura creata.
        inserico il nuovo evento in lista. Per calcolare il delay tengo conto di
          - propagazine? ma se abbiamo distane <100m??? ha senso?
          - processing? come lo stimo? calcolo quanto ci mette il pc e assumo che un nodo ci mettera' altrettanto?
          - accesso al mezzo!
            se un nodo deve trasmettere a un altro e questa sta gia' trasmettendo o ricevendo, devo aspettare che abbia finito!!
            tengo una mappa dei nodi in tx/rx che hanno come chiave l'id del nodo e come valore +inf se non sta trasmettendo, oppure
            la somma di tempo di propagazione piu' BITRATE * #BIT DEL MESSAGGIO se sto trasmettendo. Cosi' so quando finisce.

            tempo t0. A deve trasmettere a B. B e' impegnato e finisce al tempo t1. B riceve il messggio di A e si attiva al tempo t1 + TEMPO TX A->B
            tra t0 e t1 A puo' comunicare?  per semplicita'  no... ma si spreca tempo!!
          potrei stimare ad esempio un tempo di 1ms di processing e aggiungere un rumore gaussiano per introdurre delle variazioni sul microsecondo


      CCACHE PASSA A CACHE UNA MISURA
        Agent_ e' un nodo cache. Lo so. Messaggio contiene un oggetto misura. Passo la misura al nodo, invocando
        mnage_message.
        il nodo fa quello che deve con la misur e deve tornare il prossimo nodo a cui inviare.
  */
};

#endif
