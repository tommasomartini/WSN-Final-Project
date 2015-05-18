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
  vector<Event> new_events;

  default_random_engine generator = MyToolbox::get_random_generator();
  uniform_int_distribution<int> distribution(-10, 10);  // I can have a deviation in the range -10°, +10°
  int deviation = distribution(generator);
  direction_ += deviation;	// change a bit my direction

  double new_x = x_coord_ + speed_ * sin(direction_);	// compute my new position
  double new_y = y_coord_ + speed_ * cos(direction_);

  // I could also let the user go out! Just comment the following block of code!
  bool inside_area = false;
  while (!inside_area) {	// if I am going outside the area...
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

  // TODO I have to interrogate my neighbours! If the message passing fails I schedule another move_event, otherwise I schedule a new_user event

//  Event new_event(MyToolbox::get_current_time() + MyToolbox::user_observation_time, Event::move_user);	// set the new move_user event
//  new_event.set_agent(this);
//  new_events.push_back(new_event);
  return new_events;
}

/*  Receive data from a storage node or from another user
*/
// TO BE DEPRECATED
vector<Event> User::user_receive_data(UserMessage* message){
  vector<Event> new_events;
  vector<StorageNodeMessage> symbols_from_msg = message->get_symbols();
  for (vector<StorageNodeMessage>::iterator sym_it = symbols_from_msg.begin(); sym_it != symbols_from_msg.end(); sym_it++) {	// for each new symbol...
    output_symbols2_.push_back(*sym_it);	// ...add it to the list of the encoded symbols
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

	// No matter if I will keep these info or not because they are not updated: info about the blacklist cannot be "outdated", I'll keep it anyway
	for (vector<MeasureKey>::iterator old_msr_it = node_info_msg.outdated_measures_.begin(); old_msr_it != node_info_msg.outdated_measures_.end(); old_msr_it++) {	// for each outdated in the new info
		if (outdated_measures_.find(*old_msr_it) != outdated_measures_.end()) {	// this measure is not yet in my outdated list
			outdated_measures_.insert(pair<MeasureKey, unsigned char>(*old_msr_it, 0));	// add it, with a void measure...
		}
	}

	nodes_info_.erase(node_info_msg.node_id_);	// remove the old info. There could be none!
	/* Just for debug
	int num_removed = nodes_info.erase(node_info_msg.node_id_);	// remove the old info. There could be no one, then num_removed == 0!
	if (num_removed == 0) {
		cout << "User::receive_data no previous info from node " << node_info_msg.node_id_ << endl;
	} else {
		cout << "User::receive_data removed " << num_removed << " previous info from node " << node_info_msg.node_id_ << endl;
	}
	*/
	OutputSymbol curr_output_symbol_(node_info_msg.output_message_, node_info_msg.sources_);
	pair<unsigned int, OutputSymbol> new_output_symbol(node_info_msg.node_id_, curr_output_symbol_);
	nodes_info2_.insert(new_output_symbol);

	// Update the measure id
	for (vector<MeasureKey>::iterator msr_key_it_ = node_info_msg.sources_.begin(); msr_key_it_ != node_info_msg.sources_.end(); msr_key_it_++) {	// for each of the measure just received...
		unsigned int current_sns_id = (*msr_key_it_).sensor_id_;	// id of the sensor which generated this measure
		unsigned int current_msr_id = (*msr_key_it_).measure_id_;	// id of the measure
		if (updated_sensors_measures_.find(current_sns_id) == updated_sensors_measures_.end()) {	// never received a measure from this sensor
			updated_sensors_measures_.insert(pair<unsigned int, unsigned int>(current_sns_id, current_msr_id));	// add the new sensor and the relative measure
		} else {	// already received a measure from this sensor -> update it if the new measure is newer
			unsigned int current_updated_msr_id = updated_sensors_measures_.find(current_sns_id)->second;	// the most updated measure I have from this sensor
			if (current_msr_id > current_updated_msr_id) {	// the just received measure is more recent than the one I had
				updated_sensors_measures_.find(current_sns_id)->second = current_msr_id;	// replace the old measure with the new one, just received
			}
		}
	}

//	// Check to have only updated messages
//	vector<unsigned int> node_info_to_remove = vector<unsigned int>();	// here will go the node info which contains outdated measures
//	for (map<unsigned int, unsigned int>::iterator it_msr = updated_sensors_measures_.begin(); it_msr != updated_sensors_measures_.end(); it_msr++) {	// for each measure I consider "updated"...
//		unsigned int current_sns_id = it_msr->first;	// id of the sensor which generated it
//		unsigned int updated_msr_id = it_msr->second;	// id of the measure
//		for (map<unsigned int, OutputSymbol>::iterator it_info = nodes_info2_.begin(); it_info != nodes_info2_.end(); it_info++) {	// for each node info I have...
//			vector<MeasureKey> key_vector = it_info->second.sources_;	// ...take its key vector
//			vector<MeasureKey>::iterator key_vec_it = key_vector.begin();	// iterator to browse the key vector
//			for (; key_vec_it != key_vector.end(); key_vec_it++) {	// span the whole vector
//				if ((*key_vec_it).sensor_id_ == current_sns_id) {	// if there is a measure from this sensor...
//					if ((*key_vec_it).measure_id_ < updated_msr_id) {	// ...but the measure id is older than the most updated I have...
//						node_info_to_remove.push_back(it_info->first);	// ...add this node to the list of the measures to remove
//					}
//				}
//			}
//		}
//	}
//	// Remove the node info with outdated measures
//	for (vector<unsigned int>::iterator to_remove_it = node_info_to_remove.begin(); to_remove_it != node_info_to_remove.end(); to_remove_it++) {	// for each node info having outdated measure...
//		nodes_info2_.erase(*to_remove_it);	// ...discard it!
//	}

	// Message passing...
	if (message_passing3()) {	// message passing succeeded! I have decoded all the symbols
		// Communicate measures of dead sensors to the nodes

		// TODO when I will have communicated the new info to all the nodes I'll create an event: new_user, in which this user exits and a new one is created
	} else {	// message passing failed: symbols not decoded...
		// do nothing and wait to try message passing again...
	}
	return new_events;
}

/*  This user receives a "beep" from another user, asking him to send him his measures
*/
vector<Event> User::user_send_to_user(unsigned int sender_user_id) {
  vector<Event> new_events;

  IntraUserMessage intra_user_msg;  // msg to send to another user
  intra_user_msg.messages_ = output_symbols2_;
  Node* next_node = MyToolbox::users_map_ptr->find(sender_user_id)->second;
  intra_user_msg.set_receiver_node_id(next_node->get_node_id()); // should be equal to sender_user_id
  new_events = send(next_node, &intra_user_msg);

  return new_events;
}

/**************************************
    Private methods
**************************************/

bool User::message_passing3() {
	bool message_passing_succeeded = true;

	// Create a copy of the node info to work with
	map<unsigned int, OutputSymbol> info = nodes_info2_;

	map<MeasureKey, unsigned char> resolved_symbols;
	map<MeasureKey, unsigned char> released_symbols;
	vector<unsigned int> links_to_remove;
	while (true) {
		released_symbols.clear();   // released symbols are different at each step
		links_to_remove.clear();	// and so are the links I have to remove when I release an output symbol
		for (map<unsigned int, OutputSymbol>::iterator info_it = info.begin(); info_it != info.end(); info_it++) {	// scan the output symbols...
			OutputSymbol curr_output_symbol = info_it->second;
			if (curr_output_symbol.sources_.size() == 1) {  // ...to find the ones with output degree 1
				// store the resolved symbol
				resolved_symbols.insert(pair<MeasureKey, unsigned char>(*(curr_output_symbol.sources_.begin()), curr_output_symbol.xored_msg_));
				released_symbols.insert(pair<MeasureKey, unsigned char>(*(curr_output_symbol.sources_.begin()), curr_output_symbol.xored_msg_));
				links_to_remove.push_back(info_it->first);
				// Note that using a map if I try to insert two pairs with the same key, the second insertion does not succeed. In this way
				// I am guaranteed to insert only one entry for each sns id in the lists and not to have duplicated symbols.
			} else if (curr_output_symbol.sources_.empty()) {  // no more links to this node
				links_to_remove.push_back(info_it->first); // if an output symbol does not have any output link anymore it is useless, let's remove it!
			}
		}

		// Erase from the (copy of the) info the row relative to the removed links. I don't need that symbol anymore
		for (vector<unsigned int>::iterator links_it = links_to_remove.begin(); links_it != links_to_remove.end(); links_it++) {
			info.erase(*links_it);
		}

		// Now we have to XOR the just released output symbols with the output symbols containing them
		if (released_symbols.size() > 0) {  // at least one symbol has been released
			for(map<MeasureKey, unsigned char>::iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // for each released symbol...
				MeasureKey current_msr_key = released_iterator->first;	// id of the sensor who produced the released symbol and id of the measure
				unsigned char current_value = released_iterator->second;	// xored measure of this sensor
				// for each output symbol (use an iterator to use a pointer and modify the actual values!):
				for (map<unsigned int, OutputSymbol>::iterator out_sym_it = info.begin(); out_sym_it != info.end(); out_sym_it++) {	// ...for each output symbol not yet released (still in the copied buffer)...
					vector<MeasureKey>::iterator key_vec_it = find(out_sym_it->second.sources_.begin(), out_sym_it->second.sources_.end(), current_msr_key);	// ...look for this key in its key vector
					if (key_vec_it != out_sym_it->second.sources_.end()) {	// if its xored message is also formed by this measure...
						unsigned char new_xor = current_value ^ out_sym_it->second.xored_msg_;  // ...XOR the released symbol with the current output one...
						out_sym_it->second.xored_msg_ = new_xor; // ...replace it...
						out_sym_it->second.sources_.erase(key_vec_it);	// ...and remove the link
					}
				}
			}
		} else {  // no symbol released at this round
			if (info.empty()) {  // if no other symbols, everything is ok
				decoded_symbols2_ = resolved_symbols;	// store the decoded measures...
				// ...and update the backlist
				for (map<MeasureKey, unsigned char>::iterator bl_it = outdated_measures_.begin(); bl_it != outdated_measures_.end(); bl_it++) {	// for each blacklisted id...
					bl_it->second = decoded_symbols2_.find(bl_it->first)->second;	// ...store the just decoded measure
				}
				break;
			} else {
				cerr << "Impossible to decode! Message passing failed!" << endl;
				message_passing_succeeded = false;
				break;
			}
		}
	}
	return message_passing_succeeded;
}

bool User::message_passing2() {
	bool message_passing_succeeded = true;

	// Create a copy of the node info to work with
	map<unsigned int, NodeInfoMessage> info = nodes_info_;

	map<unsigned int, unsigned char> resolved_symbols;
	map<unsigned int, unsigned char> released_symbols;
	vector<unsigned int> links_to_remove;
	while (true) {
		released_symbols.clear();   // released symbols are different at each step
		links_to_remove.clear();
		//    for (int i = 0; i < nodes_info.size(); i++) {
		for (map<unsigned int, NodeInfoMessage>::iterator info_it = info.begin(); info_it != info.end(); info_it++) {	// scan the output symbols...
			NodeInfoMessage curr_info_msg = info_it->second;
			if (curr_info_msg.msrs_info_.size() == 1) {  // ...to find the ones with output degree 1
				// store the resolved symbol
				resolved_symbols.insert(pair<unsigned int, unsigned char>(curr_info_msg.msrs_info_.begin()->first, curr_info_msg.output_message_));
				released_symbols.insert(pair<unsigned int, unsigned char>(curr_info_msg.msrs_info_.begin()->first, curr_info_msg.output_message_));
				links_to_remove.push_back(info_it->first);
				// Note that using a map if I try to insert two pairs with the same key, the second insertion does not succeed. In this way
				// I am guaranteed to insert only one entry for each sns id in the lists and not to have duplicated symbols.
			} else if (curr_info_msg.msrs_info_.empty()) {  // no more links to this node
				links_to_remove.push_back(info_it->first); // if an output symbol does not have any output link anymore it is useless, let's remove it!
			}
		}

		// Erase from the (copy of the) info the row relative to the removed links. I don't need that symbol anymore
		for (vector<unsigned int>::iterator links_it = links_to_remove.begin(); links_it != links_to_remove.end(); links_it++) {
			info.erase(*links_it);
		}

		// Now we have to XOR the just released output symbols with the output symbols containing them
		//vector<unsigned int> curr_output_header;
		//vector<unsigned int>::iterator output_link_it;
		if (released_symbols.size() > 0) {  // at least one symbol has been released
			for(my_iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // for each released symbol...
				unsigned int current_sns_id = released_iterator->first;	// id of the sensor who produced the released symbol
				unsigned char current_value = released_iterator->second;	// xored measure of this sensor
				// for each output symbol (use an iterator to use a pointer and modify the actual values!):
				for (map<unsigned int, NodeInfoMessage>::iterator out_sym_it = info.begin(); out_sym_it != info.end(); out_sym_it++) {	// ...for each output symbol not yet released (still in the copied buffer)...
					if (out_sym_it->second.msrs_info_.find(current_sns_id) != out_sym_it->second.msrs_info_.end()) {	// ...if its xored message is also formed by this released sensor id...
						unsigned char new_xor = current_value ^ out_sym_it->second.output_message_;  // ...XOR the released symbol with the current output one...
						out_sym_it->second.output_message_ = new_xor; // ...replace it...
						out_sym_it->second.msrs_info_.erase(current_sns_id);	// ...and remove the link
					}
				}
			}
		} else {  // no symbol released at this round
			if (info.empty()) {  // if no other symbols, everything is ok
				decoded_symbols_ = resolved_symbols;	// store the decoded measures...
				// ...and update the backlist
				for (map<unsigned int, unsigned char>::iterator bl_it = blacklist_.begin(); bl_it != blacklist_.end(); bl_it++) {	// for each blacklisted id...
					bl_it->second = decoded_symbols_.find(bl_it->first)->second;	// ...store the just decoded measure
				}
				break;
			} else {
				cerr << "Impossible to decode! Message passing failed!" << endl;
				message_passing_succeeded = false;
				break;
			}
		}
	}
	return message_passing_succeeded;
}

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
    for (int i = 0; i < output_symbols2_.size(); i++) {  // scan the output symbols...
      StorageNodeMessage current_msg = output_symbols2_.at(i);	// (...current symbol...)
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
      output_symbols2_.erase(output_symbols2_.begin() + index_to_remove);
    }
    // Now we have to XOR the just released output symbols with the output symbols containing them
    vector<unsigned int> curr_output_header;
    vector<unsigned int>::iterator output_link_it;
    if (released_symbols.size() > 0) {  // at least one symbol has been released
      for(my_iterator released_iterator = released_symbols.begin(); released_iterator != released_symbols.end(); released_iterator++) {  // iterator is a pointer to the pair in that position
        unsigned int current_id = released_iterator->first;
        unsigned char current_value = released_iterator->second;
        // for each output symbol (use an iterator to use a pointer and modify the actual values!):
        for(vector<StorageNodeMessage>::iterator output_sym_it = output_symbols2_.begin(); output_sym_it != output_symbols2_.end(); output_sym_it++) { 
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
      if (output_symbols2_.empty()) {  // if no other symbols, everything is ok
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
  user->output_symbols2_.reserve(user->output_symbols2_.size() + symbols.size()); // reallocate the memory of the vector in order to contain all the new symbols
  (user->output_symbols2_).insert((user->output_symbols2_).end(), symbols.begin(), symbols.end());  // append the new symbols
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

  MyTime processing_time = MyToolbox::processing_time;  
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
      for (map<unsigned int, Node*>::iterator node_it = near_storage_nodes_->begin(); node_it != near_storage_nodes_->end(); node_it++) {
      				timetable.find(node_it->first)->second = new_schedule_time;
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
