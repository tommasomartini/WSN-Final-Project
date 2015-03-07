#include <iostream>
#include <vector>
#include <map>
#include <math.h>
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
    cout<<"sono denstro al message_passing "<<endl;

  bool message_passing_succeeded = true;

  map<unsigned int, unsigned char> resolved_symbols;
  map<unsigned int, unsigned char> released_symbols;
  vector<int> links_to_remove;
  // for (int counter = 1; counter <= 2; counter++) { // tutto il message passing va qui dentro!
  while (true) {
    released_symbols.clear();   // released symbols are different at each step
    links_to_remove.clear();
    // for (Message current_msg : output_symbols_) {  // scan the output symbols...
    for (int i = 0; i < output_symbols_.size(); i++) {  // scan the output symbols...
      StorageNodeMessage current_msg = output_symbols_.at(i);
      if (current_msg.sensor_ids_.size() == 1) {  // ...to find the ones with output degree 1
        // store the resolved symbol
        resolved_symbols.insert(pair<unsigned int, unsigned char>(current_msg.sensor_ids_.at(0), current_msg.xored_message_));
        released_symbols.insert(pair<unsigned int, unsigned char>(current_msg.sensor_ids_.at(0), current_msg.xored_message_));
        links_to_remove.push_back(i);
      } else if (current_msg.sensor_ids_.empty()) {  // no more links to this node
        links_to_remove.push_back(i); // if an output symbol does not have any output link anymore it is useless, let's remove it!
      }
    }

    /*  As we scan the output_symbols_ vector from the first to the last symbol, also the links we want to remove, that is
        the rows of the vector, are in the same order we find them INSIDE the vector. So, if we have to remove rows 2 and 4,
        we must first remove row 4 and then row 2. Otherwise, after removing row 2, the row which first was row 4 is now row 3
        and we remove the current row 4, which is not what we intended to remove!
    */

    unsigned int index_to_remove;
    for (int i = links_to_remove.size() - 1; i >= 0 ; i--) {  // remove the links of the resolved symbols
      index_to_remove = links_to_remove.at(i);  // index of output_symbols_ to remove
      output_symbols_.erase(output_symbols_.begin() + index_to_remove);
    }
    // Now we have to XOR the just released output symbols with the output symbols containing them
    vector<unsigned int> curr_output_header;
    vector<unsigned int>::iterator output_link_it;
    if (released_symbols.size() > 0) {  // at least one symbol has been released
      for(my_iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // iterator is a pointer to the pair in that position
        unsigned int current_id = released_iterator->first;
        unsigned char current_value = released_iterator->second;
        // for each output symbol (use an iterator to use a pointer and modify the actual values!):
        for(vector<StorageNodeMessage>::iterator output_sym_it = output_symbols_.begin(); output_sym_it != output_symbols_.end(); output_sym_it++) { 
        // for(Message current_msg : output_symbols_) {  // for each output symbol (use an iterator to use a pointer and modify the actual values!):
          // Message *curr_msg_ptr = &current_msg;
          curr_output_header = output_sym_it->sensor_ids_; // in this vector there are the source ids forming the output symbol
          output_link_it = find(output_sym_it->sensor_ids_.begin(), output_sym_it->sensor_ids_.end(), current_id);  // iterator: find the input id in output symbol's header
          if (output_link_it != output_sym_it->sensor_ids_.end()) {   // if I find a correspondence...
            unsigned char new_message = current_value ^ output_sym_it->xored_message_;  // ...XOR the released symbol with the current output one...
            output_sym_it->xored_message_ = new_message; // ...replace it...
            output_sym_it->sensor_ids_.erase(output_link_it); // ...and remove the link
          }
        }
      }
    } else {  // no symbol released at this round
      if (output_symbols_.empty()) {  // if no other symbols, everything is ok
        break;
      } else {
        cerr << "Impossible to decode! Message passing failed!" << endl;
        message_passing_succeeded = false;
        break;
      }
    }

    // for debug purpoes only: show the content of output_symbols_
    // cout << "\n- - - \nContenuto di output_symbols_:" << endl;
    // if (!output_symbols_.empty()) {
    //   for (int i = 0; i < output_symbols_.size(); i++) {  // scan the output symbols...
    //     StorageNodeMessage msg = output_symbols_.at(i);
    //     cout << "Posizione: " << i + 1 << ", messaggio: " << static_cast<int>(msg.xored_message_) << ". Source symbols: ";
    //     vector<int> v = msg.sensor_ids_;
    //     for (int j = 0; j < v.size(); ++j) {
    //       cout << v.at(j) << "  ";
    //     }
    //     cout << endl;
    //   }
    // } else {
    //   cout << "output_symbols_ e' vuoto!" << endl;
    // }
    // cout << "- - - \n\n" << endl;

  } // END while / for (int counter = 1; ...) fine del ciclo che dice quante iterazioni di msg_passing devo fare.


  if (message_passing_succeeded) {
    cout << "= = =\nMessage passing succeeded!\n= = =" << endl;
  } else {
    cout << "= = =\nMessage passing failed!\n= = =" << endl;
  }

  // // for debug purpoes only: show the content of resolved_symbols_
  // cout << "\n- - - \nContenuto di resolved_symbols:" << endl;
  // for(my_iterator resolved_it = resolved_symbols.begin(); resolved_it != resolved_symbols.end(); resolved_it++) { 
  //   int _id = resolved_it->first;
  //   unsigned char _val = resolved_it->second;
  //   cout << "s" << _id << " = " << static_cast<int>(_val) << endl;
  // }
  // cout << "- - - \n\n" << endl;
  // input_symbols_=resolved_symbols;

  return message_passing_succeeded;
}

bool User::CRC_check(Message message) {
  return true;
}

vector<Event> User::move_user(int event_time) {
  vector<Event> new_events; 
  // simulate the user moving
  y_coord_=y_coord_+(-MyToolbox::tx_range * MyToolbox::space_precision + rand() % 2*MyToolbox::tx_range *MyToolbox::space_precision); 
  x_coord_=x_coord_+(-MyToolbox::tx_range * MyToolbox::space_precision+ rand() % 2*MyToolbox::tx_range*MyToolbox::space_precision);
  y_coord_ = max(0.0, min(y_coord_, (double)(MyToolbox::square_size * MyToolbox::space_precision)));  //to avoid that coords are outside area
  x_coord_ = max(0.0, min(x_coord_, (double)(MyToolbox::square_size * MyToolbox::space_precision))); 
  // find new near_storage and near_user
  MyToolbox::set_near_storage_node(this);
  MyToolbox::set_near_user(this);
  
  // creates event user_node_query with all near nodes
  for(int i=0; i<near_storage_nodes.size(); i++){
      //Event new_event(event_time + MyToolbox::get_tx_offset(), Event::node_send_to_user); //event time distanziarli
      //new_event.set_agent(near_storage_nodes.at(i));
      //Message* new_msg_ptr = new Message();
      //new_msg_ptr->set_sender_node_id(node_id_);
      //new_event.set_message(new_msg_ptr);
      //new_events.push_back(new_event);
      UserMessage* message = new UserMessage();
      unsigned char xor_message = ((StorageNode*)near_storage_nodes.at(i))->get_xored_measure();
      vector <unsigned int> id_list = ((StorageNode*) near_storage_nodes.at(i))->get_ids();
     if(id_list.size()>0){
        vector <StorageNodeMessage> symbol = {StorageNodeMessage(xor_message,id_list)};
        message->set_symbols(symbol);
        message->set_user_to_reply(this);
        message->message_type_=Message::message_type_user_to_user;    //->>sarebbe node_to_user ma in send non c'è
        new_events = send(near_storage_nodes.at(i),message);
     }
  }
  // creates event user_user_query with all near users
  for(int i=0; i<near_users.size(); i++){
      if((((User*)near_users.at(i))->output_symbols_).size()>0){
        UserMessage* message = new UserMessage();
        message->set_symbols(((User*)near_users.at(i))->output_symbols_);
        message->set_user_to_reply(this);
        // message->set_receiver_node_id(this->node_id_);
        message->message_type_=Message::message_type_user_to_user;
        new_events = send(near_users.at(i),message);
        //Event new_event(event_time, Event::user_send_to_user); //event time distanziarli
        //new_event.set_agent(near_users.at(i));
        // new_event.set_agent_to_reply(this);
        //new_events.push_back(new_event);
      }
  }
  // create next move_user
  Event new_event(event_time+(MyToolbox::user_update_time * pow (10,9)),Event::move_user);
  new_event.set_agent(this);
  new_events.push_back(new_event);
        
  return new_events;
}

// vector<Event> User::user_send_to_user(User* user, int event_time){
//     vector<Event> new_events; 
//     cout<<"size old "<<user->output_symbols_.size()<<endl;
//         cout<<"nuove coordinate y="<<y_coord_;
//         cout<<"quanti nodi ho vicino? "<<near_storage_nodes.size()<<endl;
//         cout<<"quanti user ho vicino?"<<near_users.size()<<endl;

                
//         // creates event user_node_query with all near nodes
//         for(int i=0; i<near_storage_nodes.size(); i++){
//             UserMessage* message;
//             message->set_user_to_reply(this);
//             Event new_event(event_time, Event::node_send_to_user); //event time distanziarli
//             new_event.set_agent(near_storage_nodes.at(i));
//             new_event.set_message(message);
//             new_events.push_back(new_event);
//         }
//         // creates event user_user_query with all near users
//         for(int i=0; i<near_users.size(); i++){
//             UserMessage* message;
//             message->set_user_to_reply(this);
//             Event new_event(event_time, Event::user_send_to_user); //event time distanziarli
//             new_event.set_agent(near_users.at(i));
//             new_event.set_message(message);
//             new_events.push_back(new_event);
//         }
//         // create next move_user
//         Event new_event(event_time+MyToolbox::get_user_update_time(),Event::move_user);
//         new_event.set_agent(this);
//         new_events.push_back(new_event);
//         }
//   return new_events;
// }
/*
vector<Event> User::user_send_to_user(UserMessage* message, int event_time){
    User* user = message->get_user_to_reply();
    vector<Event> new_events;
    cout<<"entrata";
        cout<<"size old "<<user->output_symbols_.size()<<endl;
//        cout<<"ci aggiungo "<<user->output_symbols_<<.end();
    user->output_symbols_.insert(user->output_symbols_.end(), output_symbols_.begin(), output_symbols_.end()); // controlla è tardi!
    cout<<"size new "<<user->output_symbols_.size()<<endl;
    //try message passing
    if (user->message_passing()){ 
        // the user succeed message passing, now delete this user and create a new user
        User *new_user = MyToolbox::new_user();
        Event new_event(event_time+10, Event::move_user); //event time da cambiare
        new_event.set_agent(new_user);
        new_events.push_back(new_event);
    }
    return new_events;
}       
*/      //metodo user_send_to_user diventato obsoleto!

void User::add_symbols(vector<StorageNodeMessage> symbols, User* user){
  user->output_symbols_.reserve(user->output_symbols_.size() + symbols.size());
  (user->output_symbols_).insert((user->output_symbols_).end(), symbols.begin(), symbols.end());
}

vector<Event> User::user_receive_data(int event_time, UserMessage* message){
    vector<Event> new_events;
    User* user_to_update = message->get_user_to_reply();
    User::add_symbols(message->get_symbols(),message->get_user_to_reply());
    //try message passing
        if (user_to_update->message_passing()){ 
            //cout<<"message passing ok"<<"black list = "<<message->get_blacklist().get_length()<<endl;
            // the user succeed message passing, now delete this user and create a new user
            User *new_user = MyToolbox::new_user();
            Event new_event(event_time+10, Event::move_user); //event time da cambiare
            new_event.set_agent(new_user);
            new_events.push_back(new_event);
            
            //if there are elements in black_list spread mesaures
            //creare messagio id misura e evento spread
            map<int, unsigned char> outdated_symbols;
            for (int i=0; i<message->get_blacklist().get_length(); i++){
                if ( user_to_update->input_symbols_.find(message->get_blacklist().get_id_list()[i])!= user_to_update->input_symbols_.end()){
                    int id = message->get_blacklist().get_id_list()[i];
                    unsigned char symbol = user_to_update->input_symbols_.find(id)->second; 
                    outdated_symbols.insert(pair<int,unsigned char>(id,symbol));
                    }
            }
            if(outdated_symbols.size()>0){ 
                OutdatedMeasure* symbols_to_remove = new OutdatedMeasure(outdated_symbols);
                int next_node_index = rand() % near_storage_nodes.size();
                StorageNode *next_node = (StorageNode*)near_storage_nodes.at(next_node_index);
                //Event event(event_time, Event::remove_measure); //event time da cambiare
                //event.set_agent(next_node);
                //event.set_message(symbols_to_remove);
                //new_events.push_back(event);
                symbols_to_remove->set_receiver_node_id(next_node->get_node_id());
                symbols_to_remove->message_type_=Message::message_type_remove_measure;
                new_events = send(next_node, symbols_to_remove);
            }
    }   
    return new_events;
}


vector<Event> User::send(Node* next_node, Message* message) {
  vector<Event> new_events;

  MyTime processing_time = MyToolbox::mean_processing_time;  
  unsigned int num_total_bits = message->get_message_size();
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate); // in nano-seconds
  MyTime message_time = processing_time + transfer_time;
  

  /*cout<<"coordinate mie: x= "<<x_coord_<<" y= "<<y_coord_;
  //cout<<"coordinate sue: x= "<<next_node->get_x_coord()<<" y = "<<next_node->get_y_coord();
  // Compute the message time
  double distance = (sqrt(pow(y_coord_ - next_node->get_y_coord(), 2) + pow(x_coord_ - next_node->get_x_coord(), 2))) / 1000;  // in meters
  MyTime propagation_time = (MyTime)((distance / MyToolbox::kLightSpeed) * pow(10, 9));   // in nano-seconds
  MyTime processing_time =  MyToolbox::get_random_processing_time();  
  unsigned int num_total_bits = message->get_message_size();
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate); // in nano-seconds
  MyTime message_time = propagation_time + processing_time + transfer_time;
  */
  
  //cout<<"num bit = "<<num_total_bits<<endl;
  //cout<<"propagation_time "<<propagation_time<<endl; 
  //cout<<"transfer_time "<<transfer_time<<endl;
  //cout<<"processing_time "<<processing_time<<endl;
  //cout<<"message time "<<message_time<<endl;
  
  // Update the timetable
  if (!event_queue_.empty()) {  // already some pending event
    // I set a schedule time for this event, but it has no meaning! Once I will extract it from the queue
    // I will unfold it and I will build up a brand new event with its pieces and then I will set
    // a significant schedule time!
    Event event_to_enqueue(0, Event::storage_node_try_to_send);
    event_to_enqueue.set_agent(this);
    event_to_enqueue.set_message(message);
    event_queue_.push(event_to_enqueue);

    // do not insert it in the new_events vector! This event is not going to be put in the main event list now!
  } else {  // no pending events
    map<unsigned int, MyTime> timetable = MyToolbox::get_timetable();  // download the timetable (I have to upload the updated version later!)
    MyTime current_time = MyToolbox::get_current_time();  // current time of the system
    MyTime my_available_time = timetable.find(node_id_)->second; // time this sensor gets free
    MyTime next_node_available_time = timetable.find(next_node->get_node_id())->second;  // time next_node gets free
    if (my_available_time > current_time) { // this node already involved in a communication or surrounded by another communication
      MyTime new_schedule_time = my_available_time + MyToolbox::get_tx_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
      try_again_event.set_agent(this); 
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
      Event try_again_event(new_schedule_time, Event::storage_node_try_to_send);
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      // Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
      Event::EventTypes this_event_type;
      switch (message->message_type_) {
        case Message::message_type_user_to_user: {
          this_event_type = Event::user_receive_data;
          break;
        }
        case Message::message_type_remove_measure: {
          this_event_type = Event::remove_measure;
          break;
        }
        default:
          break;
      }
      Event receive_message_event(new_schedule_time, this_event_type);
      receive_message_event.set_agent(next_node);
      receive_message_event.set_message(message);
      new_events.push_back(receive_message_event);

      // Update the timetable
      timetable.find(node_id_)->second = current_time + message_time; // update my available time
      for (int i = 0; i < near_storage_nodes.size(); i++) { // update the available time of all my neighbours
        timetable.find(near_storage_nodes.at(i)->get_node_id())->second = current_time + message_time;
      }
      MyToolbox::set_timetable(timetable);  // upload the updated timetable

      // Update the event_queue_
      if (!event_queue_.empty()) {  // if there are other events in the queue
        Event top_queue_event = event_queue_.front(); // the oldest event of the queue (the top one, the first)
        event_queue_.pop(); // remove the oldest event frrm the queue
        Event popped_event(current_time + message_time + MyToolbox::get_tx_offset(), top_queue_event.get_event_type());  // create a brand new event using the popped one, seting now  valid schedule time
        popped_event.set_agent(this);
        popped_event.set_message(top_queue_event.get_message());
        new_events.push_back(popped_event); // schedule the next event
      }
    }
  }
  
  return new_events;
}