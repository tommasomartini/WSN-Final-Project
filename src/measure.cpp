#include "measure.h"
#include "my_toolbox.h"

// prova per commit eclipse 2

/**************************************
    Constructors
**************************************/
Measure::Measure() {
  message_type_ = message_type_measure;
  hop_counter_ = 0;

  measure_ = 0;
  measure_id_ = 0;
  source_sensor_id_ = 0;
  measure_type_ = Measure::measure_type_new;
}

Measure::Measure(unsigned char measure) {
  message_type_ = message_type_measure;
  hop_counter_ = 0;

  measure_ = measure;
  measure_id_ = 0;
  source_sensor_id_ = 0;
  measure_type_ = Measure::measure_type_new;
}

Measure::Measure(unsigned char measure, unsigned int measure_id, unsigned int source_sensor_id, Measure::MeasureTypes measure_type) {
  message_type_ = message_type_measure;
  hop_counter_ = 0;
  
  measure_ = measure;
  measure_id_ = measure_id;
  source_sensor_id_ = source_sensor_id;
  measure_type_ = measure_type;
}

/**************************************
    Setters
**************************************/
// void Measure::set_measure(unsigned char measure) {
//   measure_ = measure;
// }

// void Measure::set_measure_id(int measure_id){
//   measure_id_ = measure_id;
// }

// void Measure::set_source_sensor_id(int source_sensor_id) {
//   source_sensor_id_ = source_sensor_id;
// }

// void Measure::set_measure_type(Measure::MeasureTypes measure_type) {
//   measure_type_ = measure_type;
// }

/**************************************
    Functions
**************************************/
unsigned int Measure::get_message_size() {
  unsigned int message_size = 0;
  // int num_measures = measure_type_ == measure_type_new ? 1 : 2; // new or update measure? It may contain 1 or 2 measures
  message_size += MyToolbox::get_bits_for_measure(); // * num_measures; 
  message_size += MyToolbox::get_bits_for_measure_id();
  message_size += MyToolbox::get_bits_for_id();
  message_size += MyToolbox::get_bits_for_hop_counter();
  message_size += 1;  // new/update measure
  message_size += MyToolbox::get_bits_for_phy_mac_overhead();
  return message_size;
}
  
unsigned int Measure::increase_hop_counter() {
  return ++hop_counter_;
}
