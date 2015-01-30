#include "outdated_measure.h"
#include "my_toolbox.h"

OutdatedMeasure::OutdatedMeasure() {
    symbols_to_remove_;
    hop_counter_=0;
}

OutdatedMeasure::OutdatedMeasure(map <int ,unsigned char> symbols_to_remove) {
    symbols_to_remove_=symbols_to_remove;
    hop_counter_=0;
}

int OutdatedMeasure::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int OutdatedMeasure::get_message_size() {
  unsigned int message_size = 0;
  message_size += MyToolbox::get_bits_for_measure(); // * num_measures; 
  message_size += MyToolbox::get_bits_for_id();
  message_size += MyToolbox::get_bits_for_hop_counter();
  message_size += MyToolbox::get_bits_for_phy_mac_overhead();
  message_size = message_size*(symbols_to_remove_.size());
  return message_size;
}
