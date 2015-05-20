#include "reinit_response.h"
#include "my_toolbox.h"

using namespace std;

ReinitResponse::ReinitResponse() {
}

unsigned int ReinitResponse::get_message_size() {
	return MyToolbox::num_bits_for_measure_;
}
