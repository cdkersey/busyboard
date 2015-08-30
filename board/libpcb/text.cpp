#include "text.h"

using namespace std;
using namespace libpcb;

#ifndef DEFAULT_FONT_PATH
#define DEFAULT_FONT_PATH "FONT"
#endif

point libpcb::font::draw_char(gerber &g, char c, point p, double scale) const {
  if (c == ' ') return point(p.x + 3*scale, p.y);
  if (!f.count(c)) return p;
  
  const vector<point> &v(f.find(c)->second);

  double width = 0;
  g.set_dark();
  g.set_aperture(scale/5.0);
  for (unsigned i = 0; i < v.size(); i += 2) {
    if (v[i] != v[i+1])
      g.draw(p + v[i].scale(scale), p + v[i+1].scale(scale));
    else
      g.flash(p + v[i].scale(scale));
    if (v[i].x > width) width = v[i].x;
    if (v[i + 1].x > width) width = v[i + 1].x;
  }

  p.x += width*scale + scale * 1.3;

  return p;
}

void libpcb::font::load(istream &in) {
  while (!!in) {
    char c;
    in >> c;
    if (!in) break;
    while (!!in && in.peek() != '\n') {
      double x, y;
      in >> x >> y;
      f[c].push_back(point(x, y));
    }
  }
}

font &libpcb::get_default_font() {
  static font* p = new font(DEFAULT_FONT_PATH);
  return *p;
}

void libpcb::text::draw(int pri, layer lx, gerber &g) {
  if (pri == 100 && l == lx) {
    point pos;
    unsigned i;
    for (i = 0, pos = p; i < str.length(); i++)
      pos = f.draw_char(g, str[i], pos, scale);
  }
}
