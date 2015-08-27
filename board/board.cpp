// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <cmath>

using namespace std;

enum errcode {
  ERR_MODE_SET, ERR_POINT_SET, ERR_APERTURE_SET
};

void err(errcode e);

const unsigned N_CU(4); // Number of total copper layers in layer enum.

enum layer {
  LAYER_SILKSCREEN,                               // Screen
  LAYER_MASK,                                     // Solder mask (inverted)
  LAYER_CU0, LAYER_CU1, LAYER_CU2, LAYER_CU3,     // Copper
  LAYER_PTH, LAYER_NPTH                           // Thru-holes (plated/non)
};

struct point {
  point(): x(0), y(0) {}
  point(double x, double y): x(x), y(y) {}
  
  double x, y;

  bool operator==(const point &r) const { return r.x == x && r.y == y; }
  bool operator!=(const point &r) const { return r.x != x || r.y != y; }
  point operator+(const point &r) const { return point(x + r.x, y + r.y); }

  point scale(double r) const { return point(r*x, r*y); }
};

// The gerber file writer; the graphical output API.
class gerber {
public:
  gerber(ostream &out):
    out(out),
    mode_set(false), mode_clear(false), point_set(false), aperture_set(false)
  { init(); }

  ~gerber() { out << "M02*" << endl; }

  // Set the drawing mode.
  void set_dark();
  void set_clear();
  
  // Set the current aperature (only circular supported)
  void set_aperture(double d);

  // Set current position.
  void move(point to);
  
  // Draw line segments in current aperture
  void draw(point to);
  void draw(point from, point to);

  // Draw "flash" in current aperture.
  void flash();
  void flash(point pt);
  
private:
  bool mode_set, mode_clear, point_set, aperture_set;
  
  ostream &out;
  map<double, string> apertures; // Cache so we don't repeat ourselves.
  point p; // Current point.
  double a; // Current aperture.

  void init();

  void coord(point pt); // Output coordinate in weird gerber format.
  void num(double d); // Output number in weird gerber format.
};

// An object that can can be drawn. Class also keeps track of all drawables, so
// that they can be output to appropriate gerber files.
class drawable {
public:
  drawable() { drawables.insert(this); }
  virtual ~drawable() { drawables.erase(this); }

  virtual void draw(int pri, layer l, gerber &g) = 0;

  static void draw_layer(layer l, gerber &g);
  
private:
  static set<drawable *> drawables;
  static set<int> priorities;

protected:
  static void add_priority(int pri);
};

class font {
public:
  font(string filename) { ifstream in(filename); load(in); }
  point draw_char(gerber &g, char c, point p, double scale) const;
  
private:
  void load(istream &in);

  map<char, vector<point> > f;
};

class text : public drawable {
public:
  text(const font &f, int layer, point pos, string str, double scale):
    f(f), l(layer), p(pos), str(str), scale(scale) { add_priority(-100); }

  void draw(int pri, layer l, gerber &g);
  
private:
  int l;
  string str;
  const font &f;
  double scale;
  point p;
};

class component : public drawable {
public:
  component() {}

  virtual void draw(int pri, layer l, gerber &g) = 0;
};

// Linear (as in non-branching, not necessarily straight) circuit trace.
class track : public drawable {
public:
  track(int layer_idx, double thickness, double clearance):
    layer_idx(layer_idx), thickness(thickness), clearance(clearance)
  { add_priority(-50); add_priority(0); }

  track(int layer_idx, double thickness):
    layer_idx(layer_idx), thickness(thickness), clearance(thickness*1.25)
  { add_priority(-50); add_priority(0); }

  track &add_point(point pt) { points.push_back(pt); return *this; }
  track &add_point(double x, double y) { return add_point(point(x, y)); }
  
  void draw(int pri, layer l, gerber &g);
  
private:
  int layer_idx;
  double thickness, clearance;
  vector<point> points;
};

// Non-buried (all-layers) via.
class via : public drawable {
public:
  via(point center, double outer, double inner):
    center(center), outer(outer), inner(inner), clearance(outer*1.25)
  { add_priority(-50); add_priority(0); }

  via(point center, double outer, double inner, double clearance):
    center(center), outer(outer), inner(inner), clearance(clearance)
  { add_priority(-50); add_priority(0); }
  
  void draw(int pri, layer l, gerber &g);
  
private:
  double clearance, outer, inner;
  point center;
};

set<drawable*> drawable::drawables;
set<int> drawable::priorities;

void drawable::add_priority(int pri) { priorities.insert(pri); }

void drawable::draw_layer(layer l, gerber &g) {
  for (auto pri : priorities)
    for (auto &p : drawables)
      p->draw(pri, l, g);
}

// Set the current aperature (only circular supported)
void gerber::set_aperture(double d) {
  if (aperture_set && a == d) return;

  aperture_set = true;
  a = d;
  
  string name;

  if (!apertures.count(d)) {
    ostringstream oss;
    oss << 'D' << setw(2) << setfill('0') << apertures.size() + 10;
    apertures[d] = oss.str();

    // Declare the aperture as a circle with the given diameter.
    out << "%AD" << apertures[d] << "C," << d << "*%" << endl;
  }

  name = apertures[d];

  // Switch to the aperture.
  out << name << '*' << endl;
}

// Set current position.
void gerber::move(point to) {
  point_set = true;
  coord(to);
  out << "D02*" << endl;
  p = to;
}
  
// Draw line segments in current aperture
void gerber::draw(point to) {
  if (!point_set) err(ERR_POINT_SET);
  if (!mode_set) err(ERR_MODE_SET);
  if (!aperture_set) err(ERR_APERTURE_SET);
  
  coord(to);
  out << "D01*" << endl;
  p = to;
}

void gerber::draw(point from, point to) {
  if (!point_set || from != p)
    move(from);

  draw(to);
}

// Draw "flash" in current aperture.
void gerber::flash() {
  if (!point_set) err(ERR_POINT_SET);
  if (!mode_set) err(ERR_MODE_SET);
  if (!aperture_set) err(ERR_APERTURE_SET);
  
  out << "D03*" << endl;
}

void gerber::flash(point pt) {
  point_set = true;
  coord(pt);
  flash();

  p = pt;
}

void gerber::init() {
  out << "G04 Output from Chad's Board Thing.*" << endl
      << "%FSLAX26Y26*%" << endl
    // << "%TF.Part,Other*%" << endl // TODO: layer name
      << "%MOIN*%" << endl;
}

// Output coordinate in COBOLesque gerber format.
void gerber::coord(point pt) {
  if (!point_set || pt.x != p.x) {
    out << 'X';
    num(pt.x);
  }
  
  if (!point_set || pt.y != p.y) {
    out << 'Y';
    num(pt.y);
  }
}

// Output number in fixed-point gerber format.
void gerber::num(double x) {
  if (x < 0) {
    out << '-';
    x = -x;
  }
  
  out << int(x) << setw(6) << setfill('0') << int(fmod(x, 1)*1000000);
}

void gerber::set_dark() {
  if (!mode_set || (mode_set && mode_clear))
    out << "%LPD*%" << endl;
  mode_set = true;
  mode_clear = false;
}

void gerber::set_clear() {
  if (!mode_set || (mode_set && !mode_clear))
    out << "%LPC*%" << endl;
  mode_set = true;
  mode_clear = true;
}

void err(errcode e) {
  cerr << "ERROR: ";
  
  switch(e) {
  case ERR_POINT_SET: cerr << "Point not set." << endl;      break;
  case ERR_MODE_SET:  cerr << "Dark/clear not set." << endl; break;
  }

  abort();
}

void track::draw(int pri, layer l, gerber &g) {
  if (l == LAYER_CU0 + layer_idx) {
    if (pri == -50) {
      g.set_clear();
      g.set_aperture(clearance);
      g.move(points[0]);
      for (unsigned i = 1; i < points.size(); ++i)
        g.draw(points[i]);
    } else if (pri == 0) {
      g.set_dark();
      g.set_aperture(thickness);
      g.move(points[0]);
      for (unsigned i = 1; i < points.size(); ++i)
        g.draw(points[i]);
    }
  }
}

void via::draw(int pri, layer l, gerber &g) {
  if (l >= LAYER_CU0 && l <= LAYER_CU0 + N_CU) {
    if (pri == -50) {
      g.set_clear();
      g.set_aperture(clearance);
      g.flash(center);
    } else if (pri == 0) {
      g.set_dark();
      g.set_aperture(outer);
      g.flash(center);
    }
  } else if (l == LAYER_PTH) {
    if (pri == 0) {
      g.set_dark();
      g.set_aperture(inner);
      g.flash(center);
    }
  }
}

point font::draw_char(gerber &g, char c, point p, double scale) const {
  if (c == ' ') return point(p.x + 3*scale, p.y);
  if (!f.count(c)) return p;
  
  const vector<point> &v(f.find(c)->second);

  double width = 0;
  g.set_dark();
  g.set_aperture(scale/5.0);
  for (unsigned i = 0; i < v.size(); i += 2) {
    g.draw(p + v[i].scale(scale), p + v[i+1].scale(scale));
    if (v[i].x > width) width = v[i].x;
    if (v[i + 1].x > width) width = v[i + 1].x;
  }

  p.x += width*scale + scale * 1.3;

  return p;
}
  
void font::load(istream &in) {
  while (!!in) {
    char c;
    in >> c;
    if (!in) break;
    while (in.peek() != '\n') {
      double x, y;
      in >> x >> y;
      f[c].push_back(point(x, y));
    }
  }
}

void text::draw(int pri, layer lx, gerber &g) {
  if (pri == -100 && l == lx) {
    point pos;
    unsigned i;
    for (i = 0, pos = p; i < str.length(); i++)
      pos = f.draw_char(g, str[i], pos, scale);
  }
}

int main() {
  ofstream gfile("dump.grb");
  gerber g(gfile);

  for (unsigned i = 0; i < 8; ++i) {
    track *t = new track(0, 1.0/20.0);
    t->add_point(0.1*i, 0.15).add_point(0.1*i, 1.0);

    via *v = new via(point(0.1*i, 1.0), 0.08, 0.035);
  }

  for (unsigned i = 0; i < 8; ++i) {
    track *t = new track(0, 1.0/20.0);
    t->add_point(0.1*i, -0.15).add_point(0.1*i, -1.0);

    via *v = new via(point(0.1*i, -1.0), 0.08, 0.035);
  }

  font f("FONT");
  new text(f, LAYER_CU0, point(0, -1.5), "A10a20a30", 1.0/16);
  new text(f, LAYER_CU0, point(0, 1.5), "1 2 3", 1.0/8);
  
  drawable::draw_layer(LAYER_CU0, g);
  
  return 0;
}

