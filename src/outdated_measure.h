#ifndef OUTDATED_MEASURE_H
#define	OUTDATED_MEASURE_H

#include <map>
#include "message.h"
#include "my_toolbox.h"
  
class OutdatedMeasure : public Message {
 private:
  typedef MyToolbox::MeasureKey MeasureKey;

  int hop_counter_;

 public:
  OutdatedMeasure();
  OutdatedMeasure(unsigned char, std::vector<MeasureKey> /*removed*/, std::vector<MeasureKey> /*inserted*/);

  unsigned char xored_data_; 	// data the node will have to xor to its measure
  std::vector<MeasureKey> removed_;	// keys of the measures the node will remove
  std::vector<MeasureKey> inserted_;	// keys of the measures the node will insert

  int increase_hop_counter();
  unsigned int get_message_size();
};

#endif	/* OUTDATED_MEASURE_H */

