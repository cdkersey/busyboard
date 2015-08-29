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
  LAYER_MASK0, LAYER_MASK1,                       // Solder mask (inverted)
  LAYER_CU0, LAYER_CU1, LAYER_CU2, LAYER_CU3,     // Copper
  LAYER_PTH, LAYER_NPTH                           // Thru-holes (plated/non)
};

double quantize(double x, double step) {
  x = round(x * (1/step))/step;
  return x;
}

struct point {
  point(): x(0), y(0) {}
  point(double x, double y): x(x), y(y) {}
  
  double x, y;

  bool close(const point &r) const { return d2(r) < THRESH; }
  bool operator==(const point &r) const { return x == r.x && y == r.y; }

  bool operator!=(const point &r) const { return !operator==(r); }
  point operator+(const point &r) const { return point(x + r.x, y + r.y); }
  point operator-(const point &r) const { return point(x - r.x, y - r.y); }
  
  point operator+=(const point &r) { x += r.x; y += r.y; }
  point operator-=(const point &r) { x -= r.x; y -= r.y; }

  bool operator<(const point &r) const {
    return (x < r.x) || ((x == r.x) && y < r.y);
  }

  point neighborhood() const { return point(quantize(x, 1e-4), quantize(y, 1e-4)); }

  point scale(double r) const { return point(r*x, r*y); }

  double r2() const { return x*x + y*y; }
  double d2(const point &r) const { return pow(r.x - x, 2) + pow(r.y - y, 2); }
  constexpr static double THRESH = 1e-8;
};

ostream &operator<<(ostream &o, const point &p) {
  o << '(' << p.x << ", " << p.y << ')';
}

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

font &get_default_font();

class text : public drawable {
public:
  text(const font &f, int layer, point pos, string str, double scale):
    f(f), l(layer), p(pos), str(str), scale(scale) { add_priority(100); }

  void draw(int pri, layer l, gerber &g);
  
private:
  int l;
  string str;
  const font &f;
  double scale;
  point p;
};

class net;

class wire {
public:
  wire(): n(NULL) { wires.insert(this); }
  virtual ~wire() { wires.erase(this); }
  virtual void get_points(set<pair<point, int> > &s) = 0;

  void mark();
  
  void set_net(net *p) { n = p; }
  net *get_net() { return n; }

  static void print_wires();
  static void expand_nets();
  static void check_nets();

private:
  net *n;
  static set<wire*> wires;
};

set<wire*> wire::wires;

class net {
public:
  net(string name): name(name) { nets.insert(this); }
  virtual ~net() { nets.erase(this); }

  void add_wire(wire &w) { wires.insert(&w); w.set_net(this); }

  string get_name() { return name; }
  
  void mark() { for (auto w : wires) w->mark(); }
  
  static void print_nets();

private:
  set<wire*> wires;
  string name;

  static set<net*> nets;
};

set<net*> net::nets;

class pin : public wire {
public:
  pin() {}
  pin(string name, point p, const set<layer> &l): name(name), p(p), l(l) {}
  pin(string name, point p): name(name), p(p) {
    for (int i = 0; i < N_CU; ++i) l.insert(layer(LAYER_CU0 + i));
  }

  point get_loc() const { return p; }
  string get_name() const { return name; }

  void get_points(set<pair<point, int> > &s);
  
private:
  set<layer> l;
  point p;
  string name;
};

class component {
public:
  component(string name): name(name) {}
  virtual ~component() {}
  string get_name() const { return name; }
  virtual string get_type_name() = 0;
  pin &get_pin(string name) { return pins.find(name)->second; }

protected:
  void add_pin(string name, point loc)
    { pins[name] = pin(name, loc); }
  
  void add_pin(string name, point loc, const set<layer> &l)
    { pins[name] = pin(name, loc, l); }
  
  map<string, pin> pins;
  string name;
};

template <char X, unsigned L, bool V = false> class twoprong : public component
{
public:
  twoprong(string name, point pos);
  string get_type_name() { return string(X, 1); }
};

template <unsigned L, unsigned W, bool V = false> class dip : public component
{
public:
  dip(string name, point pos);
  string get_type_name() { return "U"; }
};

// Linear (as in non-branching, not necessarily straight) circuit trace.
class track : public drawable, public wire {
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

  void get_points(set<pair<point, int> > &s);
  
private:
  int layer_idx;
  double thickness, clearance;
  vector<point> points;
};

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

  void get_points(set<pair<point, int> > &s);
  
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
  if (l >= LAYER_CU0 && l < LAYER_CU0 + N_CU) {
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
  if (pri == 100 && l == lx) {
    point pos;
    unsigned i;
    for (i = 0, pos = p; i < str.length(); i++)
      pos = f.draw_char(g, str[i], pos, scale);
  }
}

void pad::draw(int pri, layer l, gerber &g) {
  via::draw(pri, l, g);

  if (pri == 0 && (top && l == LAYER_MASK1) || (!top && l == LAYER_MASK0)) {
    g.set_dark();
    g.set_aperture(outer);
    g.flash(center);
  }
}

template <char X, unsigned L, bool V>
  twoprong<X,L,V>::twoprong(string name, point p0): component(name)
{
  point p1 = point(p0.x + (V ? 0 : L*0.1), p0.y + (V ? L*0.1 : 0));

  new pad(p0, 0.08, 0.035);
  new pad(p1, 0.08, 0.035);

  add_pin("0", p0);
  add_pin("1", p1);

  new text(get_default_font(),
	   LAYER_SILKSCREEN,
	   p0 + (V ? point(0.1, 0.05): point(0.05, 0)),
	   name,
	   1/40.0);
}

template <unsigned L, unsigned W, bool V>
  dip<L,W,V>::dip(string name, point p0): component(name)
{
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
	   p0 + (V ? point(-0.1, -0.3): point(0.1, 0.05)),
	   name,
	   1/25.0);
}

void pin::get_points(set<pair<point, int> > &s) {
  for (auto x : l)
    s.insert(make_pair(p, (int)x - LAYER_CU0));
}

void via::get_points(set<pair<point, int> > &s) {
  for (unsigned l = 0; l < N_CU; ++l)
    s.insert(make_pair(center, l));
}

void track::get_points(set<pair<point, int> > &s) {
  for (auto &p : points)
    s.insert(make_pair(p, layer_idx));
}

void wire::print_wires() {
  for (auto &w : wires) {
    set<pair<point, int> > s;
    w->get_points(s);

    cout << "Wire " << &w << ':';
    for (auto &x : s)
      cout << ' ' << x.first << ' ' << x.second;
    cout << endl;
  }
}

void wire::mark() {
  set<pair<point, int> > s;
  get_points(s);
  for (auto &p : s)
    new text(get_default_font(), LAYER_SILKSCREEN, p.first, "X", 1/80.0);
}
  


void net::print_nets() {
  for (auto p : nets) {
    cout << p->name << ":" << endl;
    for (auto wp : p->wires)
      cout << "  " << wp << endl;
  }
}

void wire::expand_nets() {
  map<pair<point, int>, set<wire*> > l; // The location->wire map
  map<pair<point, int>, net*> g; // The location->net map/graph
  set<wire*> active;

  // Populate g with initial nets.
  for (auto w : wires) {
    set<pair<point, int> > s;
    w->get_points(s);
    for (auto &p : s)
      l[make_pair(p.first.neighborhood(), p.second)].insert(w);

    if (w->get_net()) {
      active.insert(w);
      for (auto &p : s) {
	if (g.count(p) && g[p] != w->get_net()) {
	  cout << "Warning: Net contention! (build)" << endl;
	}
        g[make_pair(p.first.neighborhood(), p.second)] = w->get_net();
      }
    }
  }


  
  // Keep propagating nets until there are none left to propagate.
  bool change;
  do {
    change = false;
    vector<pair<wire*, net*> > set_nets;
    set<wire*> next_active;

    for (auto wp : active) {
      set<pair<point, int> > s;
      wp->get_points(s);
      for (auto &p : s) {
	net *n(wp->get_net());

	for (auto w : l[make_pair(p.first.neighborhood(), p.second)]) {
	  if (w == wp) continue;
	  if (w->get_net() && w->get_net() != n)
            cout << "Warning: Net contention! (propagate)" << endl;
	  if (w->get_net() == NULL) {
            set_nets.push_back(make_pair(w, n));
	    next_active.insert(w);			       
	  }
	}
      }
    }

    if (set_nets.size()) change = true;
    for (auto &x : set_nets) {
      x.second->add_wire(*x.first);
      set<pair<point, int> > s;
      x.first->get_points(s);
      for (auto &y : s)
	g[make_pair(y.first.neighborhood(), y.second)] = x.second;
    }

    active = next_active;
  } while (active.size());
}

// Report if any nets are split into multiple parts or if there are any wires to
// which nets are not assigned.
void wire::check_nets() {
  expand_nets(); // Won't take much time if it's already been done.
  
  map<net*, set<wire*> > g;
  map<wire*, wire*> tag;
  map<pair<point, int>, set<wire*> > l;

  for (auto w : wires) {
    g[w->get_net()].insert(w);
    tag[w] = w;

    set<pair<point, int> > s;
    w->get_points(s);
    for (auto &p : s)
      l[make_pair(p.first.neighborhood(), p.second)].insert(w);
  }

  int merges;
  do {
    merges = 0;
    // If there's more than one wire tag at any location, merge their tags.
    for (auto &p : l) {
      set<wire*> tags;
      for(auto x : p.second)
	tags.insert(tag[x]);
      if (tags.size() > 1) {
	++merges;
	wire *new_tag(*tags.begin());
	for (auto x : p.second)
	  tag[x] = new_tag;
      }
    }
  } while (merges);

  for (auto &p : g) {
    set<wire*> tags;
    for (auto x : p.second)
      tags.insert(tag[x]);

    if (p.first) {
      if (tags.size() > 1)
        cout << "ERROR: Net " << p.first->get_name() << " in " 
             << tags.size() << " pieces." << endl;
    } else {
      cout << "ERROR: " << tags.size() << " orphan nets." << endl;
    }
  }
}

font &get_default_font() {
  static font* p = new font("FONT");
  return *p;
}

// Some nice, but not essential typedefs.

template <unsigned L, bool V = false> using Ll = twoprong<'L', L, V>;
template <unsigned L, bool V = false> using Cl = twoprong<'C', L, V>;
template <unsigned L, bool V = false> using Rl = twoprong<'R', L, V>;
template <unsigned L, bool V = false> using Dl = twoprong<'D', L, V>;

typedef Rl<6> R6;
typedef Rl<3> R3;

typedef Rl<6, true> R6V;
typedef Rl<3, true> R3V;

typedef dip<8, 3, false> DIP16;
typedef dip<8, 3, true> DIP16V;

typedef dip<20, 6, false> DIP40;
typedef dip<20, 6, true> DIP40V;

int main() {
  net &vdd(*(new net("VDD"))), &gnd(*(new net("GND"))), &x(*(new net("X")));

  #if 1
  for (unsigned i = 0; i < 8; ++i) {
    track *t = new track(0, 1.0/20.0);
    t->add_point(0.1*i, 0.15).add_point(0.1*i,0.5).add_point(0.1*i, 1.0);

    via *v = new via(point(0.1*i, 1.0), 0.08, 0.035);
  }

  for (unsigned i = 0; i < 8; ++i) {
    track *t = new track(0, 1.0/20.0);
    t->add_point(0.1*i, -0.15).add_point(0.1*i,-0.5).add_point(0.1*i, -1.0);

    via *v = new via(point(0.1*i, -1.0), 0.08, 0.035);
  }

  track *t = new track(1, 1/20.0);
  t->add_point(0.7, -1.0).add_point(1.7, -1.0);
  
  new text(get_default_font(), LAYER_CU0, point(0, -1.5),
	   "01237?$@#AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz",
	   1.0/16);

  DIP16 &u0(*(new DIP16("U0", point(0, -0.15))));

  vdd.add_wire(u0.get_pin("8"));
  gnd.add_wire(u0.get_pin("16"));
  gnd.add_wire(u0.get_pin("1"));
  gnd.add_wire(u0.get_pin("2"));
  gnd.add_wire(u0.get_pin("3"));

  new R3("A0", point(0.7, 0.5));
  new R6("B1", point(0.7, -0.5));

  new R3V("A3", point(1.7, -1.0));
  new Dl<1, true>("d3", point(1.7, -0.6));
  (new track(1, 1/20.0))->add_point(1.7, -0.7).add_point(1.7, -0.6);

  new DIP40("U1", point(0, 1.5));

  (new track(0, 1/20.0))->add_point(0,-1).add_point(0.1,-1).add_point(0.2,-1);
  (new track(0, 1/20.0))->add_point(0, 0.15).add_point(0,-0.15);
  
  #else

  R3 *r0 = new R3("A0", point(0, 0));
  Dl<1, false> *d0 = new Dl<1, false>("D0", point(0.4, 0));
  (new track(1, 1/20.0))->add_point(0.3,0).add_point(0.4,0);
  (new track(0, 1/20.0))->add_point(0.3,0).add_point(0.3,-1);
  (new via(point(0.3, -1), 0.08, 0.035));
  (new track(1, 1/20.0))->add_point(-1,-1).add_point(0.3,-1).add_point(1,-1);

  vdd.add_wire(r0->get_pin("0"));
  x.add_wire(d0->get_pin("0"));
  gnd.add_wire(d0->get_pin("1"));

  #endif

  wire::expand_nets();  
  wire::check_nets();
  // net::print_nets();

  //gnd.mark();
  //vdd.mark();
  x.mark();
  
  {
    ofstream gfile("dump.cu0.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_CU0, g);
  }

  {
    ofstream gfile("dump.cu1.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_CU1, g);
  }

  {
    ofstream gfile("dump.ss.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_SILKSCREEN, g);
  }

  {
    ofstream gfile("dump.mask0.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_MASK0, g);
  }

  {
    ofstream gfile("dump.mask1.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_MASK1, g);
  }

  {
    ofstream gfile("dump.pth.grb");
    gerber g(gfile);
    drawable::draw_layer(LAYER_PTH, g);
  }

  return 0;
}

