#ifndef LIBPCB_DRAWABLE_H
#define LIBPCB_DRAWABLE_H

#include <set>

#include "gerber.h"
#include "layer.h"

namespace libpcb {

class drawable {
public:
  drawable() { drawables.insert(this); }
  virtual ~drawable() { drawables.erase(this); }

  virtual void draw(int pri, layer l, gerber &g) = 0;

  static void draw_layer(layer l, gerber &g);
  
private:
  static std::set<drawable *> drawables;
  static std::set<int> priorities;

protected:
  static void add_priority(int pri);
};
 
}

#endif
