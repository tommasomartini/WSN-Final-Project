#include "outdated_measure.h"
#include "my_toolbox.h"

OutdatedMeasure::OutdatedMeasure() {
    symbols_to_remove_ = map<unsigned int, unsigned char>();
    hop_counter_ = 0;
}

OutdatedMeasure::OutdatedMeasure(map <unsigned int ,unsigned char> symbols_to_remove) {
    symbols_to_remove_ = symbols_to_remove;
    hop_counter_ = 0;
}

int OutdatedMeasure::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int OutdatedMeasure::get_message_size() {
  unsigned int message_size = symbols_to_remove_.size() * MyToolbox::num_bits_for_measure_;
  return message_size;
}
