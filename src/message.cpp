#include "message.h"

using namespace std;

/**************************************
    Setters
**************************************/
void Message::set_receiver_node_id(unsigned int receiver_node_id) {
  receiver_node_id_ = receiver_node_id;
}

void Message::set_sender_node_id(unsigned int sender_node_id) {
  sender_node_id_ = sender_node_id;
}

string Message::int2msgtype(int num) {
	string res = "msg_unknown";
	switch (num) {
	case 0:
		res = "message_type_measure";
		break;
	case 1:
		res = "message_type_blacklist";
		break;
	case 2:
		res = "message_type_remove_measure";
		break;
	case 3:
		res = "message_type_ping";
		break;
	case 4:
		res = "message_type_measures_for_user";
		break;
	case 5:
		res = "message_type_intra_user";
		break;
	case 6:
		res = "message_type_new_node";
		break;
	case 7:
		res = "message_type_user_to_user";
		break;

	default:
		res = "msg_unknown";
		break;
	}
	return res;
}
