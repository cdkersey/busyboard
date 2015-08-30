#ifndef LIBPCB_PIN
#define LIBPCB_PIN

#include <set>

#include "layer.h"
#include "net.h"

namespace libpcb {

class pin : public wire {
public:
  pin() {}

  pin(std::string name, point p, const std::set<layer> &l):
    name(name), p(p), l(l) {}

  pin(std::string name, point p): name(name), p(p) {
    for (int i = 0; i < N_CU; ++i) l.insert(layer(LAYER_CU0 + i));
  }

  point get_loc() const { return p; }
  std::string get_name() const { return name; }

  void get_points(std::set<std::pair<point, int> > &s);
  
private:
  std::set<layer> l;
  point p;
  std::string name;
};
  
}

#endif
