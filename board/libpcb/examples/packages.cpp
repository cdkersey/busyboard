// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <libpcb/text.h>
#include <libpcb/basic.h>
#include <libpcb/poly.h>

using namespace std;
using namespace libpcb;

// SMD pad
class smdpad : public drawable, public wire {
public:
  smdpad(layer l, point p0, point p1, double width, double clearance);

  virtual void draw(int pri, layer l, gerber &g);
  void get_points(std::set<std::pair<point, int> > &s);
  
private:
  layer l;
  point p0, p1;
  double width, clearance;
};

smdpad::smdpad(layer l, point p0, point p1, double width, double clearance):
  l(l), p0(p0), p1(p1), width(width), clearance(clearance), wire()
{}

void smdpad::draw(int pri, layer d, gerber &g) {
  double sm_clearance = width + 2*0.003;
  if (pri == 0 && l != LAYER_CU0 && d == LAYER_MASK1) {
    // Draw clearance in silkscreen
    g.set_dark();
    g.set_aperture(sm_clearance);
    g.move(p0);
    g.draw(p1);
  } else if (pri == 0 && l == LAYER_CU0 && d == LAYER_MASK0) {
    // Draw clearance in silkscreen
    g.set_dark();
    g.set_aperture(sm_clearance);
    g.move(p0);
    g.draw(p1);
  } else if (pri == -50 && d == l) {
    // Draw clearance
    g.set_clear();
    g.set_aperture(clearance);
    g.move(p0);
    g.draw(p1);
  } else if (pri == 0 && d == l) {
    // Draw track
    g.set_dark();
    g.set_aperture(width);
    g.move(p0);
    g.draw(p1);
  }
}

void smdpad::get_points(std::set<std::pair<point, int> > &s) {
  s.insert(make_pair(p0, l));
  s.insert(make_pair(p1, l));
}

// Variable length/width SOIC
template <unsigned L, bool V = false, bool T = true>
  class soic : public component
{
public:
  soic(std::string name, point pos);
  std::string get_type_name() { return "U"; }
};

template <unsigned L, bool V, bool T>
  soic<L,V,T>::soic(std::string name, point p0): component(name)
{
  using namespace std;

  const double pitch = 0.05, padlen = 0.05, padwidth = 0.02, clearance = 0.032,
               width = 0.15;
  
  point p = p0;
  point pin_offset = (V ? point(padlen, 0) : point(0, padlen));
  for (unsigned i = 0; i < L*2; ++i) {
    new smdpad((T?LAYER_CU1:LAYER_CU0), p, p + pin_offset, padwidth, clearance);
    
    ostringstream oss;
    oss << i + 1;
    add_pin(oss.str(), p);

    if (i == L-1)
      p += point((V ? -width : 0), (V ? 0 : width));
    else
      if (i < L)
        p += point((V ? 0 : pitch), (V ? pitch : 0));
      else
        p -= point((V ? 0 : pitch), (V ? pitch : 0));
  }
  
  new text(get_default_font(),
	   LAYER_SILKSCREEN,
	   p0 + (V ? point(padlen/2 + width/2, padwidth):
		     point(padwidth, padlen/2 + width/2)),
	   name, 1/128.0);

  (new poly(LAYER_SILKSCREEN))->
    add_point(p0 + point(0, padlen/2)).
    add_point(p0 + point(0, width + padlen/2)).
    add_point(p0 + point(pitch*(L-1), width + padlen/2)).
    add_point(p0 + point(pitch*(L-1), padlen/2));

  (new poly(LAYER_SILKSCREEN))->
    add_point(p0 + point(0, padlen/2)).
    add_point(p0 + point(0, padlen/2 + width/2)).
    add_point(p0 + point(pitch, padlen/2));  
}

template <unsigned L, unsigned H, bool T = true>
  class qfp : public component
{
public:
  qfp(std::string name, point pos, double pitch=0.0315, double gap = 0.03125);
  std::string get_type_name() { return "U"; }
private:
  double pitch, gap;
};

template <unsigned L, unsigned H, bool T>
  qfp<L, H, T>::qfp(std::string name, point pos, double pitch, double gap):
  component(name), pitch(pitch), gap(gap)
{
  using namespace std;

  double pin_length = 0.0775, padwidth = 0.018, clearance = padwidth + 0.012;
  point p = pos;
  for (unsigned i = 0; i < 2*L + 2*H; ++i) {
    point pin_offset;
    if (i < L) pin_offset = point(0, -pin_length); // Down
    else if (i < L + H) pin_offset = point(pin_length, 0); // Left
    else if (i < 2*L + H) pin_offset = point(0, pin_length); // Up
    else pin_offset = point(-pin_length, 0); // Left

    new smdpad((T?LAYER_CU1:LAYER_CU0), p, p + pin_offset, padwidth, clearance);

    ostringstream oss;
    oss << i + 1;
    add_pin(oss.str(), p);
    
    if (i < L-1) p += point(pitch, 0);
    else if (i == L-1) p = pos + point(pitch*(L-1) + gap, gap);
    else if (i < L + H - 1) p += point(0, pitch);
    else if (i == L + H - 1) p = pos + point(pitch*(L-1), pitch*(H-1) + 2*gap);
    else if (i < 2*L + H - 1) p += point(-pitch, 0);
    else if (i == 2*L + H - 1) p = pos + point(-gap, pitch*(H-1) + gap);
    else p += point(0, -pitch);
  }
  new text(get_default_font(),
    LAYER_SILKSCREEN,
    pos + point(gap, gap + 2*pitch), name, 1/128.0
  );

  (new poly(LAYER_SILKSCREEN))->
    add_point(pos + point(0, gap)).
    add_point(pos + point(0, (H-1)*pitch + gap)).
    add_point(pos + point((L-1)*pitch, (H-1)*pitch + gap)).
    add_point(pos + point((L-1)*pitch, gap));

  (new poly(LAYER_SILKSCREEN))->
    add_point(pos + point(0, gap)).
    add_point(pos + point(0, gap + pitch)).
    add_point(pos + point(pitch, gap));

}


void dips(point o) {
  new dip<7, 3>("DIP14", o + point(0.0, 0.0));
  new dip<8, 3>("DIP16", o + point(0.0, 0.5));
  new dip<9, 3>("DIP18", o + point(0.0, 1.0));
  new dip<10, 3>("DIP20", o + point(0.0, 1.5));
  new dip<12, 6>("DIP24", o + point(1.5, 0.0));
  new dip<14, 6>("DIP28", o + point(1.5, 0.75));
  new dip<16, 6>("DIP32", o + point(1.5, 1.5));
  new dip<20, 6>("DIP40", o + point(1.5, 2.25));

  (new poly(LAYER_SILKSCREEN))->
    add_point(o-point(0.2, 0.2)).add_point(o + point(-0.2, 3)).
    add_point(o + point(3.5, 3)).add_point(o + point(3.5, -0.2));
}

void soics(point o) {
  new soic<7>("SOIC14", o + point(0.0, 0.0));
  new soic<8>("SOIC16", o + point(0.0, 0.5));
  new qfp<11, 11>("QFP44", o + point(1, 1));
  new qfp<24, 16>("QFP80", o + point(0, 1.5));

  (new poly(LAYER_SILKSCREEN))->
    add_point(o-point(0.2, 0.2)).add_point(o + point(-0.2, 3)).
    add_point(o + point(1.75, 3)).add_point(o + point(1.75, -0.2));

  new plane(LAYER_CU1, o - point(0.2, 0.2), o + point(1.75, 3));
}

int main() {
  dips(point(0, 0));
  soics(point(4.0, 0));
  
  { ofstream gfile("packages.ss.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_SILKSCREEN, g); }
  { ofstream gfile("packages.mask_b.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_MASK0, g); }
  { ofstream gfile("packages.mask_t.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_MASK1, g); }
  { ofstream gfile("packages.cu0.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_CU0, g); }
  { ofstream gfile("packages.cu1.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_CU1, g); }
  { ofstream gfile("packages.pth.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_PTH, g); }
  { ofstream gfile("packages.route.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_ROUTE, g); }
  
  return 0;
}
