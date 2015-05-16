/*  Measure generated by a storage node which detects one or more expired sensor
*/
#ifndef __BLACKLIST_MESSAGE_H__  
#define __BLACKLIST_MESSAGE_H__  

#include <vector>
#include <iostream>

#include "message.h"

class BlacklistMessage : public Message {
 private:
//  unsigned int* sensor_id_; //contains id of sensors which are expired
  std::vector<unsigned int>* sensor_ids_ptr_;	// contains the ids of the expired sensors	// TODO to remove
  std::vector<unsigned int> sensor_ids_;	// contains the ids of the expired sensors
//  int length_;  // number of expired sensors
  int hop_counter_;
  
 public: 
  BlacklistMessage();
//  BlacklistMessage(unsigned int*);
//  BlacklistMessage(unsigned int*, int /*length*/);	// TODO remove
  BlacklistMessage(vector<unsigned int>*);	// TODO to remove
  BlacklistMessage(vector<unsigned int>);
//  void set_id_list(unsigned int*);
//  void set_length(int);
  
  std::vector<unsigned int>* get_id_list(){return sensor_ids_ptr_;}	// TODO to remove
  std::vector<unsigned int> get_id_list3(){return sensor_ids_;}
//  unsigned int* get_id_list2(){return sensor_id_;}	// TODO remove
//  int get_length() {return length_;}	// TODO remove
  int get_hop_counter() {return hop_counter_;}

  int increase_hop_counter();
  unsigned int get_message_size();

};

#endif

