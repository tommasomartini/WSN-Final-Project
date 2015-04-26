#include "reinit_query.h"

using namespace std;

//ReinitQuery::ReinitQuery() {
//	neighbours_to_query_ = nullptr;
//}

ReinitQuery::ReinitQuery(vector<unsigned int> neighbours) {
	neighbours_to_query_ = neighbours;
}
