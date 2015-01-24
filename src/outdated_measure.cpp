#include "outdated_measure.h"

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


