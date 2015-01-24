/*  Measure generated by a sensor node which is spread among storage node through random walk
*/
#ifndef __MEASURE_H__  
#define __MEASURE_H__  

#include <vector>

#include "message.h"

using namespace std;

class Measure : public Message {
 public:
  enum MeasureTypes {
    measure_type_new,  // single measure
    measure_type_update  // XOR of new measure and old measure
  };

 private:
  unsigned char measure_;
  int measure_id_;  // id of this measure
  int source_sensor_id_;  // id of the sensor who generated this measure
  int hop_counter_;
  Measure::MeasureTypes measure_type_;
  
 public: 
  Measure();  // to use for debug purposes only. Remove later on?
  Measure(unsigned char /*measure*/);
  Measure(unsigned char, int /*measure_id*/, int /*source_sensor_id*/, Measure::MeasureTypes);

  void set_measure(unsigned char /*measure*/);
  void set_measure_id(int measure_id);
  void set_source_sensor_id(int);
  void set_measure_type(Measure::MeasureTypes);
  
  unsigned char get_measure() {return measure_;}
  int get_measure_id() {return measure_id_;}
  int get_source_sensor_id() {return source_sensor_id_;}
  int get_hop_counter() {return hop_counter_;}
  Measure::MeasureTypes get_measure_type() {return measure_type_;}

  int increase_hop_counter();
  unsigned int get_message_size();
};

#endif
