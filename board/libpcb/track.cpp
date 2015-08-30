#include "track.h"

using namespace libpcb;
using namespace std;

void libpcb::track::get_points(set<pair<point, int> > &s) {
  for (auto &p : points)
    s.insert(make_pair(p, layer_idx));
}

void libpcb::track::draw(int pri, layer l, gerber &g) {
  if (l == LAYER_CU0 + layer_idx) {
    if (pri == -50) {
      g.set_clear();
      g.set_aperture(clearance);
      g.move(points[0]);
      for (unsigned i = 1; i < points.size(); ++i)
        g.draw(points[i]);
    } else if (pri == 0) {
      g.set_dark();
      g.set_aperture(thickness);
      g.move(points[0]);
      for (unsigned i = 1; i < points.size(); ++i)
        g.draw(points[i]);
    }
  }
}
