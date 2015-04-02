
#ifndef OUTDATED_MEASURE_H
#define	OUTDATED_MEASURE_H

#include <map>
#include "message.h"

using namespace std;
  
class OutdatedMeasure : public Message {
 private:
  map<unsigned int, unsigned char> symbols_to_remove_; // id of the sensors to remove and their measures
  int hop_counter_;

 public:
  // Constructors
  OutdatedMeasure();
  OutdatedMeasure(map<unsigned int ,unsigned char>);

  map<unsigned int, unsigned char> get_outdaded_measure() {return symbols_to_remove_;}
  int get_size() {return symbols_to_remove_.size();}
  int get_hop_counter() {return hop_counter_;}
  int increase_hop_counter();
  unsigned int get_message_size();
};

#endif	/* OUTDATED_MEASURE_H */

