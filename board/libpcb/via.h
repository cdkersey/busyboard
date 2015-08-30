#ifndef LIBPCB_VIA_H
#define LIBPCB_VIA_H

#include "net.h"
#include "drawable.h"

namespace libpcb {

// Non-buried (all-layers) via.
class via : public drawable, public wire {
public:
  via(point center, double outer, double inner):
    center(center), outer(outer), inner(inner), clearance(outer*1.25)
  { add_priority(-50); add_priority(0); }

  via(point center, double outer, double inner, double clearance):
    center(center), outer(outer), inner(inner), clearance(clearance)
  { add_priority(-50); add_priority(0); }
  
  virtual void draw(int pri, layer l, gerber &g);

  void get_points(std::set<std::pair<point, int> > &s);
  
protected:
  double clearance, outer, inner;
  point center;
};

// Pads are just vias with corresponding holes in the solder mask.
class pad : public via {
public:
  pad(point center, double outer, double inner, bool top=true):
    via(center, outer, inner), top(top) {}
  pad(point center, double outer, double inner, double clear, bool top=true):
    via(center, outer, inner, clear), top(top) {}

  virtual void draw(int pri, layer l, gerber &g);
private:
  bool top;
};

}

#endif
