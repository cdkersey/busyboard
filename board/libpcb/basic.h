#ifndef LIBPCB_BASIC_H
#define LIBPCB_BASIC_H

#include <sstream>

#include "component.h"
#include "poly.h"
#include "via.h"
#include "text.h"

namespace libpcb {
  
// Variable length/width DIP
template <unsigned L, unsigned W, bool V = false>
  class dip : public component
{
public:
  dip(std::string name, point pos);
  std::string get_type_name() { return "U"; }
};

// Variable-width 2-prong component
template <char X, unsigned L, bool V = false>
  class twoprong : public component
{
public:
  twoprong(std::string name, point pos);
  std::string get_type_name() { return std::string(X, 1); }
};

// Some nice, basic typedefs.

template <unsigned L, bool V = false> using Ll = twoprong<'L', L, V>;
template <unsigned L, bool V = false> using Cl = twoprong<'C', L, V>;
template <unsigned L, bool V = false> using Rl = twoprong<'R', L, V>;
template <unsigned L, bool V = false> using Dl = twoprong<'D', L, V>;

typedef Rl<6> R6;
typedef Rl<3> R3;

typedef Rl<6, true> R6V;
typedef Rl<3, true> R3V;

typedef dip<4, 3, false> DIP8;
typedef dip<4, 3, true> DIP8V;
typedef dip<7, 3, false> DIP14;
typedef dip<7, 3, true> DIP14V;
typedef dip<8, 3, false> DIP16;
typedef dip<8, 3, true> DIP16V;

typedef dip<14, 6, false> DIP28;
typedef dip<14, 6, true> DIP28V;
typedef dip<20, 6, false> DIP40;
typedef dip<20, 6, true> DIP40V;

template <unsigned L, unsigned W, bool V>
  libpcb::dip<L,W,V>::dip(std::string name, point p0): component(name)
{
  using namespace std;
  
  point p = p0;
  for (unsigned i = 0; i < L*2; ++i) {
    new pad(p, 0.08, 0.035);

    ostringstream oss;
    oss << i + 1;
    add_pin(oss.str(), p);
    
    if (i == L-1)
      p += point((V ? -0.1*W : 0), (V ? 0 : 0.1*W));
    else
      if (i < L)
        p += point((V ? 0 : 0.1), (V ? 0.1 : 0));
      else
        p -= point((V ? 0 : 0.1), (V ? 0.1 : 0));
  }
  
  new text(get_default_font(),
	   LAYER_SILKSCREEN,
	   p0 + (V ? point(-0.1, -0.3): point(0.15, 0.1)),
	   name,
	   1/40.0);

  (new poly(LAYER_SILKSCREEN))->
    add_point(p0 + point(0, 0.05)).
    add_point(p0 + point(0, 0.1*W - 0.05)).
    add_point(p0 + point(0.1*(L-1), 0.1*W - 0.05)).
    add_point(p0 + point(0.1*(L-1), 0.05));

  (new poly(LAYER_SILKSCREEN))->
    add_point(p0 + point(0, W*0.05 + 0.05)).
    add_point(p0 + point(0.05, W*0.05 + 0.05)).
    add_point(p0 + point(0.1, W*0.05)).
    add_point(p0 + point(0.05, W*0.05 - 0.05)).
    add_point(p0 + point(0, W*0.05 - 0.05));
  
}

template <char X, unsigned L, bool V>
  libpcb::twoprong<X,L,V>::twoprong(std::string name, point p0): component(name)
{
  using namespace std;
  
  point p1 = point(p0.x + (V ? 0 : L*0.1), p0.y + (V ? L*0.1 : 0));

  new pad(p0, 0.08, 0.035);
  new pad(p1, 0.08, 0.035);

  add_pin("1", p0);
  add_pin("2", p1);

  new text(get_default_font(),
	   LAYER_SILKSCREEN,
	   p0 + (V ? point(-0.1, 0.05): point(0.05, 0)),
	   name,
	   1/40.0);
}
 
}
#endif
