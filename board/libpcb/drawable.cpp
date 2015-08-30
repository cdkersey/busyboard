#include "drawable.h"

using namespace std;
using namespace libpcb;

void libpcb::drawable::add_priority(int pri) { priorities.insert(pri); }

void libpcb::drawable::draw_layer(layer l, gerber &g) {
  for (auto pri : priorities)
    for (auto &p : drawables)
      p->draw(pri, l, g);
}

set<drawable*> libpcb::drawable::drawables;
set<int> libpcb::drawable::priorities;
