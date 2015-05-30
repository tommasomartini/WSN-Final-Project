#include "outdated_measure.h"

using namespace std;

OutdatedMeasure::OutdatedMeasure() {
	message_type_ = Message::message_type_user_info_for_cache;
    hop_counter_ = 0;
    xored_data_ = 0;
}

OutdatedMeasure::OutdatedMeasure(unsigned char xored_data, map<unsigned int, vector<unsigned int>> update_infos) {
	message_type_ = Message::message_type_user_info_for_cache;
	hop_counter_ = 0;

	xored_data_ = xored_data;
	update_infos_ = update_infos;
}

OutdatedMeasure::OutdatedMeasure(unsigned char xored_data, vector<MeasureKey> removed, vector<MeasureKey> inserted) {
	message_type_ = Message::message_type_user_info_for_cache;
	hop_counter_ = 0;

	xored_data_ = xored_data;
	removed_ = removed;
	inserted_ = inserted;
}

int OutdatedMeasure::increase_hop_counter() {
  return ++hop_counter_;
}

unsigned int OutdatedMeasure::get_message_size() {
  return MyToolbox::num_bits_for_measure_;
}
