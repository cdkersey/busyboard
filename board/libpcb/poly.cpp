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

void libpcb::plane::draw(int pri, layer lyr, gerber &g) {
  if (lyr == l && pri == -1000) {
    g.start_region();
    g.draw(p0, point(p0.x, p1.y));
    g.draw(p1);
    g.draw(point(p1.x, p0.y));
    g.end_region();
  }
}
