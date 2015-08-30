#ifndef LIBPCB_TRACK_H
#define LIBPCB_TRACK_H

#include <vector>

#include "net.h"
#include "drawable.h"

namespace libpcb {
  
// Linear (as in non-branching, not necessarily straight) circuit trace.
class track : public drawable, public wire {
public:
  track(int layer_idx, double thickness, double clearance):
    layer_idx(layer_idx), thickness(thickness), clearance(clearance)
  { add_priority(-50); add_priority(0); }

  track(int layer_idx, double thickness):
    layer_idx(layer_idx), thickness(thickness), clearance(thickness + 0.02)
  { add_priority(-50); add_priority(0); }

  track &add_point(point pt) { points.push_back(pt); return *this; }
  track &add_point(double x, double y) { return add_point(point(x, y)); }
  
  void draw(int pri, layer l, gerber &g);

  void get_points(std::set<std::pair<point, int> > &s);
  
private:
  int layer_idx;
  double thickness, clearance;
  std::vector<point> points;
};
 
}

#endif
