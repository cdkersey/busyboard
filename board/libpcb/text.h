#ifndef LIBPCB_TEXT_H
#define LIBPCB_TEXT_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "drawable.h"

namespace libpcb {

class font {
public:
  font(std::string filename) { std::ifstream in(filename); load(in); }
  point draw_char(gerber &g, char c, point p, double scale) const;
  
private:
  void load(std::istream &in);

  std::map<char, std::vector<point> > f;
};

class text : public drawable {
public:
 text(const font &f, int layer, point pos, std::string str, double scale):
    f(f), l(layer), p(pos), str(str), scale(scale) { add_priority(100); }

  void draw(int pri, layer l, gerber &g);
  
private:
  int l;
  std::string str;
  const font &f;
  double scale;
  point p;
};

font &get_default_font();
 
}

#endif
