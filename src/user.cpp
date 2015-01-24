#include <iostream>
#include <vector>
#include <map>
#include <algorithm>    // std::find, to check whether an element is in a vector

#include "user.h"
#include "storage_node_message.h"
#include "my_toolbox.h"
#include "stdlib.h"
#include "event.h"
#include "storage_node.h"
#include "user_message.h"
#include "outdated_measure.h"

using namespace std;


/*
void User::switch_on_user() {
  user_on_ = true;

  // Start collecting data from the near strage nodes
  cout << "User " << user_id_ << " is near to " << near_storage_nodes.size() << endl;

  for (int i = 0; i < near_storage_nodes.size(); i++) {
    StorageNode *storage_node_ptr = (StorageNode*)near_storage_nodes.at(i);
    StorageNodeMessage storage_node_msg = storage_node_ptr->retrieve_data();
    output_symbols_.push_back(storage_node_msg);
  }

  if (output_symbols_.size() < MyToolbox::get_k()) {   // no way I can retrieve all the k messages
    cout << "User " << user_id_ << ": message cannot be decoded, needs to move" << endl;
    move();
  } else if (message_passing()) {   // all the k symbols decoded
    cout << "User " << user_id_ << ": message decoded!" << endl;
    return;
  } else {  // not able to decode
    cout << "User " << user_id_ << ": message passing failed!" << endl;
    move();
    return;
  }
} 
*/

/*
void User::switch_off_user() {
  user_on_ = false;
  // thread_.join();
}
 */

/*
void User::collect_data(Message xored_message) {
  if (CRC_check(xored_message)) {
    StorageNodeMessage message(xored_message.get_message(), xored_message.header_);
    output_symbols_.push_back(message);
  } else {
    cout << "CRC chek failed for messagr from node " << xored_message.get_source_sensor_id() << endl;
  }
}
*/

bool User::message_passing() {

  bool message_passing_succeeded = true;

  map<int, unsigned char> resolved_symbols;
  map<int, unsigned char> released_symbols;
  vector<int> links_to_remove;
  // for (int counter = 1; counter <= 2; counter++) { // tutto il message passing va qui dentro!
  while (true) {
    // cout << " ---->" << endl;
    // StorageNodeMessage current_msg;
    released_symbols.clear();   // released symbols are different at each step
    links_to_remove.clear();
    // for (Message current_msg : output_symbols_) {  // scan the output symbols...
    for (int i = 0; i < output_symbols_.size(); i++) {  // scan the output symbols...
      StorageNodeMessage current_msg = output_symbols_.at(i);
      cout << "Messaggio: " << static_cast<int>(current_msg.xored_message_) << endl;
      if (current_msg.sensor_ids_.size() == 1) {  // ...to find the ones with output degree 1
        cout << "   Ha grado 1!" << endl;
        // store the resolved symbol
        resolved_symbols.insert(pair<int, unsigned char>(current_msg.sensor_ids_.at(0), current_msg.xored_message_));
        released_symbols.insert(pair<int, unsigned char>(current_msg.sensor_ids_.at(0), current_msg.xored_message_));
        links_to_remove.push_back(i);
      } else if (current_msg.sensor_ids_.empty()) {  // no more links to this node
        links_to_remove.push_back(i); // if an output symbol does not have any output link anymore it is useless, let's remove it!
      }
    }
    cout << "Dimensione resolved: " << resolved_symbols.size() << endl;
    cout << "Dimensione released: " << released_symbols.size() << endl;

    /*  As we scan the output_symbols_ vector from the first to the last symbol, also the links we want to remove, that is
        the rows of the vector, are in the same order we find them INSIDE the vector. So, if we have to remove rows 2 and 4,
        we must first remove row 4 and then row 2. Otherwise, after removing row 2, the row which first was row 4 is now row 3
        and we remove the current row 4, which is not what we intended to remove!
    */


    int index_to_remove;
    for (int i = links_to_remove.size() - 1; i >= 0 ; i--) {  // remove the links of the resolved symbols
      index_to_remove = links_to_remove.at(i);  // index of output_symbols_ to remove
      output_symbols_.erase(output_symbols_.begin() + index_to_remove);
    }
    cout << "Dimensione output dopo rimozione link resolved: " << output_symbols_.size() << endl;
    cout << endl;
    // Now we have to XOR the just released output symbols with the output symbols containing them
    vector<int> curr_output_header;
    vector<int>::iterator output_link_it;
    if (released_symbols.size() > 0) {  // at least one symbol has been released
      cout << "At least one released symbol!\n" << endl;
      for(my_iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // iterator is a pointer to the pair in that position
        int current_id = released_iterator->first;
        unsigned char current_value = released_iterator->second;
        cout << "Cerco i link da s" << current_id << " = " << static_cast<int>(current_value) << endl;
        // for each output symbol (use an iterator to use a pointer and modify the actual values!):
        for(vector<StorageNodeMessage>::iterator output_sym_it = output_symbols_.begin(); output_sym_it != output_symbols_.end(); output_sym_it++) { 
        // for(Message current_msg : output_symbols_) {  // for each output symbol (use an iterator to use a pointer and modify the actual values!):
          // Message *curr_msg_ptr = &current_msg;
          cout << "   Output symbol attuale: " << static_cast<int>(output_sym_it->xored_message_) << endl;
          curr_output_header = output_sym_it->sensor_ids_; // in this vector there are the source ids forming the output symbol
          output_link_it = find(output_sym_it->sensor_ids_.begin(), output_sym_it->sensor_ids_.end(), current_id);  // iterator: find the input id in output symbol's header
          if (output_link_it != output_sym_it->sensor_ids_.end()) {   // if I find a correspondence...
            cout << "    Questo output symbol contiene s" << current_id << " ed e' fatto di " << output_sym_it->sensor_ids_.size() << " source pkts" << endl;
            cout << "Tom: " << *output_link_it << ", " << current_id << endl;
            unsigned char new_message = current_value ^ output_sym_it->xored_message_;  // ...XOR the released symbol with the current output one...
            // output_sym_it->set_message(new_message); // ...replace it...
            output_sym_it->xored_message_ = new_message; // ...replace it...
            cout << "    Rimpiazzo il suo valore con " << static_cast<int>(output_sym_it->xored_message_) << endl;
            output_sym_it->sensor_ids_.erase(output_link_it); // ...and remove the link
            cout << "    Ora il simbolo e' fatto da " << output_sym_it->sensor_ids_.size() << " pacchetti" << endl;
          }
        }
      }
    } else {  // no symbol released at this round
      if (output_symbols_.empty()) {  // if no other symbols, everything is ok
        break;
      } else {
        cout << "\nImpossible to decode! Message passing failed!\n" << endl;
        message_passing_succeeded = false;
        break;
      }
    }

    // for debug purpoes only: show the content of output_symbols_
    cout << "\n- - - \nContenuto di output_symbols_:" << endl;
    if (!output_symbols_.empty()) {
      for (int i = 0; i < output_symbols_.size(); i++) {  // scan the output symbols...
        StorageNodeMessage msg = output_symbols_.at(i);
        cout << "Posizione: " << i + 1 << ", messaggio: " << static_cast<int>(msg.xored_message_) << ". Source symbols: ";
        vector<int> v = msg.sensor_ids_;
        for (int j = 0; j < v.size(); ++j) {
          cout << v.at(j) << "  ";
        }
        cout << endl;
      }
    } else {
      cout << "output_symbols_ e' vuoto!" << endl;
    }
    cout << "- - - \n\n" << endl;

  } // END while / for (int counter = 1; ...) fine del ciclo che dice quante iterazioni di msg_passing devo fare.


  if (message_passing_succeeded) {
    cout << "= = =\nMessage passing succeeded!\n= = =" << endl;
  } else {
    cout << "= = =\nMessage passing failed!\n= = =" << endl;
  }

  // for debug purpoes only: show the content of resolved_symbols_
  cout << "\n- - - \nContenuto di resolved_symbols:" << endl;
  for(my_iterator resolved_it = resolved_symbols.begin(); resolved_it != resolved_symbols.end(); resolved_it++) { 
    int _id = resolved_it->first;
    unsigned char _val = resolved_it->second;
    cout << "s" << _id << " = " << static_cast<int>(_val) << endl;
  }
  cout << "- - - \n\n" << endl;
  input_symbols_=resolved_symbols;

  return message_passing_succeeded;
}

bool User::CRC_check(Message message) {
  return true;
}

vector<Event> User::move_user(int event_time) {
    vector<Event> new_events;
    if(user_on_==true){ 
        // simulate the user moving
        y_coord_=y_coord_+(-MyToolbox::get_tx_range() + rand() % 2*MyToolbox::get_tx_range());    //controlla che abbia un senso
        x_coord_=x_coord_+(-MyToolbox::get_tx_range() + rand() % 2*MyToolbox::get_tx_range());
        // find new near_storage and near_user
        MyToolbox::set_near_storage_node(this);
        MyToolbox::set_near_user(this);
        
        cout<<"nuove coordinate y="<<y_coord_;
        cout<<"quanti nodi ho vicino? "<<near_storage_nodes.size()<<endl;
        cout<<"quanti user ho vicino?"<<near_users.size()<<endl;

                
        // creates event user_node_query with all near nodes
        for(int i=0; i<near_storage_nodes.size(); i++){
            UserMessage* message;
            message->set_user_to_reply(this);
            Event new_event(event_time, Event::node_send_to_user); //event time distanziarli
            new_event.set_agent(near_storage_nodes.at(i));
            new_event.set_message(message);
            new_events.push_back(new_event);
        }
        // creates event user_user_query with all near users
        for(int i=0; i<near_users.size(); i++){
            UserMessage* message;
            message->set_user_to_reply(this);
            Event new_event(event_time, Event::user_send_to_user); //event time distanziarli
            new_event.set_agent(near_users.at(i));
            new_event.set_message(message);
            new_events.push_back(new_event);
        }
        // create next move_user
        Event new_event(event_time+MyToolbox::get_user_update_time(),Event::move_user);
        new_event.set_agent(this);
        new_events.push_back(new_event);
        }
  return new_events;
}

vector<Event> User::user_send_to_user(UserMessage* message, int event_time){
    User* user = message->get_user_to_reply();
    vector<Event> new_events;
    cout<<"entrata";
    if(user_on_==true){
        cout<<"size old "<<user->output_symbols_.size()<<endl;
//        cout<<"ci aggiungo "<<user->output_symbols_<<.end();
        user->output_symbols_.insert(user->output_symbols_.end(), output_symbols_.begin(), output_symbols_.end()); // controlla è tardi!
        cout<<"size new "<<user->output_symbols_.size()<<endl;
        //try message passing
        if (user->message_passing()){ 
            // the user succeed message passing, now delete this user and create a new user
            user->user_on_=false;
            User *new_user = MyToolbox::new_user();
            Event new_event(event_time+10, Event::move_user); //event time da cambiare
            new_event.set_agent(new_user);
            new_events.push_back(new_event);
        }
    }   
    return new_events;
}

vector<Event> User::user_receive_data(int event_time, UserMessage* message){
    vector<Event> new_events;
    cout<<"entrata";
    if(user_on_==true){
        // add data
        StorageNodeMessage message_to_add(message->get_xored_message(),message->get_sensor_id() );
        output_symbols_.push_back(message_to_add);
        //try message passing
        if (message_passing()){ 
            // the user succeed message passing, now delete this user and create a new user
            user_on_=false;
            User *new_user = MyToolbox::new_user();
            Event new_event(event_time+10, Event::move_user); //event time da cambiare
            new_event.set_agent(new_user);
            new_events.push_back(new_event);
            
            //if there are elements in black_list spread mesaures
            //creare messagio id misura e evento spread
            map<int, unsigned char> outdated_symbols;
            for (int i=0; i<message->get_blacklist().get_length(); i++){
                if ( input_symbols_.find(message->get_blacklist().get_id_list()[i])!= input_symbols_.end()){
                    int id = message->get_blacklist().get_id_list()[i];
                    unsigned char symbol = input_symbols_.find(id)->second; 
                    outdated_symbols.insert(pair<int,unsigned char>(id,symbol));
                    }
            }
            OutdatedMeasure* symbols_to_remove = new OutdatedMeasure(outdated_symbols);
            int next_node_index = rand() % near_storage_nodes.size();
            StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
            Event event(event_time, Event::remove_measure); //event time da cambiare
            event.set_agent(next_node);
            event.set_message(symbols_to_remove);
            new_events.push_back(event);
        }
    }   
    return new_events;
}


