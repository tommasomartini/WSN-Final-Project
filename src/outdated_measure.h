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
  OutdatedMeasure(unsigned char, std::map<unsigned int, std::vector<unsigned int>>);
  OutdatedMeasure(std::map<unsigned int, unsigned char>, std::map<unsigned int, std::vector<unsigned int>>);

  unsigned char xored_data_; 	// data the node will have to xor to its measure	// TODO to deprecate
  std::map<unsigned int, unsigned char> replacements_;	// for each sensor id, the xor between the old and the new data
  std::map<unsigned int, std::vector<unsigned int>> update_infos_;	// for each sensor I have a vector with 3 numbers: <old msr id, flag erase/replace, new old msr id>
  std::vector<MeasureKey> removed_;	// keys of the measures the node will remove	// TODO deprecate
  std::vector<MeasureKey> inserted_;	// keys of the measures the node will insert	// TODO deprecate

  int increase_hop_counter();
  unsigned int get_message_size();
};

#endif	/* OUTDATED_MEASURE_H */

