#ifndef LIBPCB_POLY_H
#define LIBPCB_POLY_H

#include <vector>

#include "drawable.h"

namespace libpcb {

// Freeform line-art closed polygon
class poly : public drawable {
public:
  poly(layer l, double ap = 0.010): l(l), ap(ap) { add_priority(0); }

  poly &add_point(point p) { points.push_back(p); return *this; }
  poly &add_point(double x, double y) { return add_point(point(x, y)); }

  virtual void draw(int pri, layer lay, gerber &g);

private:
  double ap;
  layer l;
  std::vector<point> points;
};

// Filled rectangle, goes under traces and pads
class plane : public drawable {
public:
  plane(layer l, point p0, point p1): l(l), p0(p0), p1(p1) {
    add_priority(-1000);
  }
  
  void draw(int pri, layer lyr, gerber &g);

private:
  layer l;
  point p0, p1;
};
 
}
 
#endif
