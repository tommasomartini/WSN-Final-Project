#include <iostream>
#include <math.h>
#include <random>
#include <algorithm>    // std::find, to check whether an element is in a vector

#include "user.h"
#include "intra_user_message.h"
#include "my_toolbox.h"
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

/**************************************
    Public methods
**************************************/

//vector<Event> User::move_user(int event_time) {
//  vector<Event> new_events;
//  // simulate the user moving
//  y_coord_=y_coord_+(-MyToolbox::ray_length * MyToolbox::space_precision + rand() % 2*MyToolbox::ray_length *MyToolbox::space_precision);
//  x_coord_=x_coord_+(-MyToolbox::ray_length * MyToolbox::space_precision+ rand() % 2*MyToolbox::ray_length*MyToolbox::space_precision);
//  y_coord_ = max(0.0, min(y_coord_, (double)(MyToolbox::square_size * MyToolbox::space_precision)));  //to avoid that coords are outside area
//  x_coord_ = max(0.0, min(x_coord_, (double)(MyToolbox::square_size * MyToolbox::space_precision)));
//  // find new near_storage and near_user
//  MyToolbox::set_near_storage_node(this);
//  MyToolbox::set_near_user(this);
//
//  // creates event user_node_query with all near nodes
//  for(int i=0; i<near_storage_nodes.size(); i++){
//      //Event new_event(event_time + MyToolbox::get_tx_offset(), Event::node_send_to_user); //event time distanziarli
//      //new_event.set_agent(near_storage_nodes.at(i));
//      //Message* new_msg_ptr = new Message();
//      //new_msg_ptr->set_sender_node_id(node_id_);
//      //new_event.set_message(new_msg_ptr);
//      //new_events.push_back(new_event);
//      UserMessage* message = new UserMessage();
//      unsigned char xor_message = ((StorageNode*)near_storage_nodes.at(i))->get_xored_measure();
//      vector <unsigned int> id_list = ((StorageNode*) near_storage_nodes.at(i))->get_ids();
//     if(id_list.size()>0){
//        vector <StorageNodeMessage> symbol = {StorageNodeMessage(xor_message,id_list)};
//        message->set_symbols(symbol);
//        message->set_user_to_reply(this);
//        message->message_type_=Message::message_type_user_to_user;    //->>sarebbe node_to_user ma in send non c'è
//        new_events = send(near_storage_nodes.at(i),message);
//     }
//  }
//  // creates event user_user_query with all near users
//  for(int i=0; i<near_users.size(); i++){
//      if((((User*)near_users.at(i))->output_symbols_).size()>0){
//        UserMessage* message = new UserMessage();
//        message->set_symbols(((User*)near_users.at(i))->output_symbols_);
//        message->set_user_to_reply(this);
//        // message->set_receiver_node_id(this->node_id_);
//        message->message_type_=Message::message_type_user_to_user;
//        new_events = send(near_users.at(i),message);
//        //Event new_event(event_time, Event::user_send_to_user); //event time distanziarli
//        //new_event.set_agent(near_users.at(i));
//        // new_event.set_agent_to_reply(this);
//        //new_events.push_back(new_event);
//      }
//  }
//  // create next move_user
//  Event new_event(event_time+(MyToolbox::user_update_time * pow (10,9)),Event::move_user);
//  new_event.set_agent(this);
//  new_events.push_back(new_event);
//
//  return new_events;
//}

vector<Event> User::move() {
  default_random_engine generator = MyToolbox::get_random_generator();
  uniform_int_distribution<int> distribution(-10, 10);  // I can have a deviation in the range -10°, +10°
  int deviation = distribution(generator);
  direction_ += deviation;

  double new_x = x_coord_ + speed_ * sin(direction_);
  double new_y = y_coord_ + speed_ * cos(direction_);

  // I could also let the user go out! Just comment the following block of code!
  bool inside_area = false;
  while (!inside_area) {
    if (new_x < 0 || new_x > MyToolbox::square_size || new_y < 0 || new_y > MyToolbox::square_size) { // I am going out from the area
      direction_ += 30; // rotate of 30 degree in clockwise sense
      new_x = x_coord_ + speed_ * sin(direction_);
      new_y = y_coord_ + speed_ * cos(direction_);
    } else {
      inside_area = true;
    }
  }

  x_coord_ = new_x; // update the user's position
  y_coord_ = new_y;
  MyToolbox::set_close_nodes(this);   // set new storage nodes and users

  vector<Event> new_events;
  Event new_event(MyToolbox::get_current_time() + MyToolbox::user_observation_time, Event::move_user);
  new_event.set_agent(this);
  new_events.push_back(new_event);
  return new_events;
}

/*  Receive data from a storage node or from another user
*/
vector<Event> User::user_receive_data(UserMessage* message){
  vector<Event> new_events;
  vector<StorageNodeMessage> symbols_from_msg = message->get_symbols();
  for (vector<StorageNodeMessage>::iterator sym_it = symbols_from_msg.begin(); sym_it != symbols_from_msg.end(); sym_it++) {	// for each new symbol...
    output_symbols_.push_back(*sym_it);	// ...add it to the list of the encoded symbols
  }
//  User* user_to_update = message->get_user_to_reply();	// TODO why??
//  User::add_symbols(message->get_symbols(), message->get_user_to_reply());	// FIXME why???????

  if (message_passing()){	// if message passing works
    // the user succeeds message passing, now delete this user and create a new user
    User *new_user = MyToolbox::new_user();
    Event new_event(MyToolbox::get_current_time() + MyToolbox::user_update_time, Event::move_user);
    new_event.set_agent(new_user);
    new_events.push_back(new_event);
    
    vector<unsigned int>* bl = message->get_blacklist().get_id_list();
    map<unsigned int, unsigned char> outdated_symbols;
    for (vector<unsigned int>::iterator id_iter = bl->begin(); id_iter != bl->end(); id_iter++) {	// for each id in the blacklist...
      if (input_symbols_.find(*id_iter) != input_symbols_.end()) {	// ...if the corresponding sensor has a decoded measure in my input symbol list...
        unsigned int sns_id = *id_iter;	// ...save the id...
        unsigned char sns_msg = input_symbols_.find(*id_iter)->second;	// ...and save the measure...
        outdated_symbols.insert(pair<unsigned int, unsigned char>(sns_id, sns_msg));	// ...and insert the new pair id-msr in the list of the outdated symbols
      }
    }
    if(outdated_symbols.size() > 0){	// if there is some outdated measure...
      OutdatedMeasure* symbols_to_remove = new OutdatedMeasure(outdated_symbols);
      int next_node_index = rand() % near_storage_nodes_->size();
      map<unsigned int, Node*>::iterator node_iter = near_storage_nodes_->begin();
      for (int i = 0; i < next_node_index; i++) {
        node_iter++;
      }
      StorageNode *next_node = (StorageNode*)node_iter->second;
      symbols_to_remove->set_receiver_node_id(next_node->get_node_id());
      symbols_to_remove->message_type_=Message::message_type_remove_measure;
      new_events = send(next_node, symbols_to_remove);
    }
  }   
  return new_events;
}

vector<Event> User::receive_data(NodeInfoMessage node_info_msg) {
	vector<Event> new_events;
	nodes_info.erase(node_info_msg.node_id_);	// remove the old info. There could be none!
	/* Just for debug
	int num_removed = nodes_info.erase(node_info_msg.node_id_);	// remove the old info. There could be no one, then num_removed == 0!
	if (num_removed == 0) {
		cout << "User::receive_data no previous info from node " << node_info_msg.node_id_ << endl;
	} else {
		cout << "User::receive_data removed " << num_removed << " previous info from node " << node_info_msg.node_id_ << endl;
	}
	*/
	pair<unsigned int, NodeInfoMessage> new_info_pair(node_info_msg.node_id_, node_info_msg);
	nodes_info.insert(new_info_pair);

	// Update the measure id
	for (map<unsigned int, unsigned int>::iterator it_msrs_info = node_info_msg.msrs_info_.begin(); it_msrs_info != node_info_msg.msrs_info_.end(); it_msrs_info++) {
		unsigned int current_sns_id = it_msrs_info->first;
		unsigned int current_msr_id = it_msrs_info->second;
		if (updated_sensors_measures_.find(current_sns_id) == updated_sensors_measures_.end()) {	// never received a measure from this sensor
			updated_sensors_measures_.insert(pair<unsigned int, unsigned int>(current_sns_id, current_msr_id));	// add the new sensor and the relative measure
		} else {	// already received a measure from this sensor -> update it!
			unsigned int current_updated_msr_id = updated_sensors_measures_.find(current_sns_id)->second;	// the most updated measure I have from this sensor
			if (current_msr_id > current_updated_msr_id) {	// Update the measure id
				updated_sensors_measures_.find(current_sns_id)->second = current_msr_id;
			}
		}
	}

	// Check to have only updated messages
	vector<unsigned int> node_info_to_remove = vector<unsigned int>();
	for (map<unsigned int, unsigned int>::iterator it_msr = updated_sensors_measures_.begin(); it_msr != updated_sensors_measures_.end(); it_msr++) {
		unsigned int current_sns_id = it_msr->first;
		unsigned int updated_msr_id = it_msr->second;
		for (map<unsigned int, NodeInfoMessage>::iterator it_info = nodes_info.begin(); it_info != nodes_info.end(); it_info++) {
			unsigned int curr_msr_id = it_info->second.msrs_info_.find(updated_msr_id)->second;
			if (curr_msr_id < updated_msr_id) {		// this measure is no more valid
				node_info_to_remove.push_back(it_info->first);	// add this node to the list of the measures to remove
			}
		}
	}

	// Remove the node info with outdated measures
	for (vector<unsigned int>::iterator to_remove_it = node_info_to_remove.begin(); to_remove_it != node_info_to_remove.end(); to_remove_it++) {
		nodes_info.erase(*to_remove_it);
	}

	// Now I use the data!
	// Message passing...
	// Detect blacklist measures
	// Communicate measures of dead sensors to the nodes

	return new_events;
}

/*  This user receives a "beep" from another user, asking him to send him his measures
*/
vector<Event> User::user_send_to_user(unsigned int sender_user_id) {
  vector<Event> new_events;

  IntraUserMessage intra_user_msg;  // msg to send to another user
  intra_user_msg.messages_ = output_symbols_;
  Node* next_node = MyToolbox::users_map_ptr->find(sender_user_id)->second;
  intra_user_msg.set_receiver_node_id(next_node->get_node_id()); // should be equal to sender_user_id
  new_events = send(next_node, &intra_user_msg);

  return new_events;
}

/**************************************
    Private methods
**************************************/

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

void User::add_symbols(vector<StorageNodeMessage> symbols, User* user){	// FIXME che cacchio e' questo metodo??
  user->output_symbols_.reserve(user->output_symbols_.size() + symbols.size()); // reallocate the memory of the vector in order to contain all the new symbols
  (user->output_symbols_).insert((user->output_symbols_).end(), symbols.begin(), symbols.end());  // append the new symbols
}

vector<Event> User::try_retx(Message* message, int next_node_id) {
  map<unsigned int, Node*>* nodes_map = MyToolbox::storage_nodes_map_ptr;
  StorageNode* next_node = (StorageNode*)nodes_map->find(next_node_id)->second;
  return send(next_node, message);
}

vector<Event> User::try_retx_to_user(Message* message, int next_node_id) {
  map<unsigned int, Node*>* users_map = MyToolbox::users_map_ptr;
  User* next_user = (User*)users_map->find(next_node_id)->second;
  double rx_user_x = next_user->get_x_coord();
  double rx_user_y = next_user->get_y_coord();
  double dist = sqrt(pow(x_coord_ - rx_user_x, 2) + pow(y_coord_ - rx_user_y, 2));  // compute the distance between the two users
  if (dist < MyToolbox::tx_range) { // the users are still able to communicate
    return send(next_user, message);
  } else {
    return vector<Event>();
  }
}

vector<Event> User::send(Node* next_node, Message* message) {
  vector<Event> new_events;

  MyTime processing_time = MyToolbox::mean_processing_time;  
  unsigned int num_total_bits = message->get_message_size();
  MyTime transfer_time = (MyTime)(num_total_bits * 1. * pow(10, 3) / MyToolbox::bitrate); // in nano-seconds
  MyTime message_time = processing_time + transfer_time;
  
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
      Event try_again_event(0); // create an event with a fake schedule time
      if (message->message_type_ == Message::message_type_intra_user) {
        try_again_event = Event(new_schedule_time, Event::user_try_to_send);
      } else {
        try_again_event = Event(new_schedule_time, Event::user_try_to_send_to_user);
      }
      try_again_event.set_agent(this); 
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else if (next_node_available_time > current_time) { // next_node already involved in a communication or surrounded by another communication
      MyTime new_schedule_time = next_node_available_time + MyToolbox::get_tx_offset();
      Event try_again_event(0); // create an event with a fake schedule time
      if (message->message_type_ == Message::message_type_intra_user) {
        try_again_event = Event(new_schedule_time, Event::user_try_to_send);
      } else {
        try_again_event = Event(new_schedule_time, Event::user_try_to_send_to_user);
      }
      try_again_event.set_agent(this);
      try_again_event.set_message(message);
      new_events.push_back(try_again_event);
    } else {  // sender and receiver both idle, can send the message
      // Schedule the new receive event
      MyTime new_schedule_time = current_time + message_time;
      // Now I have to schedule a new event in the main event queue. Accordingly to the type of the message I can schedule a different event
      Event::EventTypes this_event_type;
      switch (message->message_type_) {
        case Message::message_type_intra_user: {
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
      // TODO devo aggiornre la mappa, non il vettore!!!
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
