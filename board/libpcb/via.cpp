#include "via.h"

using namespace libpcb;
using namespace std;

void libpcb::via::draw(int pri, layer l, gerber &g) {
  if (l >= LAYER_CU0 && l < LAYER_CU0 + N_CU) {
    if (pri == -50) {
      g.set_clear();
      g.set_aperture(clearance);
      g.flash(center);
    } else if (pri == 0) {
      g.set_dark();
      g.set_aperture(outer);
      g.flash(center);
    }
  } else if (l == LAYER_PTH) {
    if (pri == 0) {
      g.set_dark();
      g.set_aperture(inner);
      g.flash(center);
    }
  }
}

void libpcb::pad::draw(int pri, layer l, gerber &g) {
  via::draw(pri, l, g);

  if (pri == 0 && (top && l == LAYER_MASK1) || (!top && l == LAYER_MASK0)) {
    g.set_dark();
    g.set_aperture(outer);
    g.flash(center);
  }
}

void libpcb::via::get_points(set<pair<point, int> > &s) {
  for (unsigned l = 0; l < N_CU; ++l)
    s.insert(make_pair(center, l));
}
