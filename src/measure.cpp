#include "measure.h"

Measure::Measure() {
  message_type_ = message_type_measure;

  measure_ = 0;
  measure_id_ = 0;
  source_sensor_id_ = 0;
  measure_type_ = Measure::new_measure;
}

Measure::Measure(unsigned char measure) {
  message_type_ = message_type_measure;

  measure_ = measure;
  measure_id_ = 0;
  source_sensor_id_ = 0;
  measure_type_ = Measure::new_measure;
}

Measure::Measure(unsigned char measure, int measure_id, int source_sensor_id, Measure::MeasureTypes measure_type) {
  message_type_ = message_type_measure;
  
  measure_ = measure;
  measure_id_ = measure_id;
  source_sensor_id_ = source_sensor_id;
  measure_type_ = measure_type;
}

void Measure::set_measure(unsigned char measure) {
  measure_ = measure;
}

void Measure::set_measure_id(int measure_id){
  measure_id_ = measure_id;
}

void Measure::set_source_sensor_id(int source_sensor_id) {
  source_sensor_id_ = source_sensor_id;
}

void Measure::set_message_type(Measure::MeasureTypes message_type) {
  measure_type_ = message_type;
}
  
int Measure::increase_hop_counter() {
  return ++hop_counter_;
}