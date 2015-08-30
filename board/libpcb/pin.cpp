#include "pin.h"

using namespace libpcb;
using namespace std;

void libpcb::pin::get_points(set<pair<point, int> > &s) {
  for (auto x : l)
    s.insert(make_pair(p, (int)x - LAYER_CU0));
}
