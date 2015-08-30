#include "poly.h"

using namespace libpcb;
using namespace std;

void libpcb::poly::draw(int pri, layer lay, gerber &g) {
  if (l == lay && pri == 0) {
    g.set_dark();
    g.set_aperture(ap);
    g.move(points[0]);
    for (unsigned i = 1; i < points.size(); ++i) g.draw(points[i]);
    g.draw(points[0]);
  }
}
