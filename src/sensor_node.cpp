#include "sensor_node.h"

#include <stdlib.h>     /* srand, rand */

void SensorNode::set_measure(Measure measure) {
	measure_ = measure;
}

void SensorNode::generate_data() {
  unsigned char new_measure = (unsigned char)(rand() % 256);  // generate a random char
}
