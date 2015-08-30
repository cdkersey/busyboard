// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <libpcb/basic.h>
#include <libpcb/track.h>
#include <libpcb/component.h>

using namespace std;
using namespace libpcb;

// Breadboard terminal strip. 6.5"x.43" with terminal strips 0.2" in from edges,
// in 2 sets of 27 with a 0.9" gap in between. 
class tstrip : public component {
public:
  tstrip(string name, point pos);
  std::string get_type_name() { return "J"; }
};

tstrip::tstrip(string name, point pos): component(name) {

  point p(pos);
  
  for (unsigned i = 0; i < 54; ++i) {
    ostringstream oss;
    oss << i + 1;
    add_pin(oss.str(), p);

    new pad(p, 0.08, 0.035);
    
    if (i == 26) p += point(0.9, 0);
    else p += point(0.1, 0);
  }
  
  // Outline
  (new poly(LAYER_SILKSCREEN))->
    add_point(pos + point(-0.2, -0.1)).
    add_point(pos + point(-0.2, 0.33)).
    add_point(pos + point(6.3, 0.33)).
    add_point(pos + point(6.3, -0.1));

  // Label
  new text(get_default_font(), LAYER_SILKSCREEN, pos + point(0.2, 0.15),
    "BREADBOARD TERMINAL STRIP - " + name, 1.0/32
  );
}

void place_breadboard(point pos) {
  (new poly(LAYER_SILKSCREEN))->
    add_point(pos + point(-0.2, 0)).
    add_point(pos + point(-0.2, 2.2)).
    add_point(pos + point(6.3, 2.2)).
    add_point(pos + point(6.3, 0));

  // Label
  new text(get_default_font(), LAYER_SILKSCREEN, pos + point(0.2, 0.15),
    "SOLDERLESS BREADBOARD", 1.0/32
  );
}

// Read netlist.
void load_nets(map<string, component*> &c, map<string, net*> &nets) {
  ifstream in("final.nets");
  
  while (!!in) {
    string nstr;
    in >> nstr;
    nets[nstr] = new net(nstr);
    while (in.peek() != '\n') {
      string des;
      int pin;
      in >> des >> pin;
      if (!in) break;

      if (!c.count(des)) {
	cout << "ERROR: designator " << des << " not found." << endl;
	continue;
      }

      ostringstream pstr;
      pstr << pin;

      c[des]->get_pin(pstr.str()).set_net(nets[nstr]);
    }
  }
}

void make_board() {
  map<string, component*> c;

  c["C5"] = new Cl<3, true>("C5", point(-0.1, 0));
  c["U1"] = new DIP16("U1", point(0, 0));
  c["C6"] = new Cl<3, true>("C6", point(0.9, 0));
  c["U2"] = new DIP16("U2", point(1, 0));
  c["C7"] = new Cl<3, true>("C7", point(1.9, 0));
  c["U3"] = new DIP16("U3", point(2, 0));
  c["C8"] = new Cl<3, true>("C8", point(2.9, 0));
  c["U4"] = new DIP16("U4", point(3, 0));
  c["C9"] = new Cl<3, true>("C9", point(3.9, 0));
  c["U5"] = new DIP16("U5", point(4, 0));
  c["C10"] = new Cl<3, true>("C10", point(4.9, 0));
  c["U6"] = new DIP16("U6", point(5, 0));
  c["C11"] = new Cl<3, true>("C11", point(6.4, 0.5));
  c["U7"] = new DIP16("U7", point(6.5, 0.5));

  
  c["C12"] = new Cl<3, true>("C12", point(0.4, 0.75));
  c["U8"] = new DIP16("U8", point(0.5, 0.75));
  c["C13"] = new Cl<3, true>("C13", point(1.4, 0.75));
  c["U9"] = new DIP16("U9", point(1.5, 0.75));
  c["C14"] = new Cl<3, true>("C14", point(2.4, 0.75));
  c["U10"] = new DIP16("U10", point(2.5, 0.75));
  c["C15"] = new Cl<3, true>("C15", point(3.4, 0.75));
  c["U11"] = new DIP16("U11", point(3.5, 0.75));
  c["C16"] = new Cl<3, true>("C16", point(4.4, 0.75));
  c["U12"] = new DIP16("U12", point(4.5, 0.75));
  c["C17"] = new Cl<3, true>("C17", point(5.4, 0.75));
  c["U13"] = new DIP16("U13", point(5.5, 0.75));
  
  c["J2"] = new tstrip("J2", point(0, -0.6));
  place_breadboard(point(0, -2.9));

  // Route ground
  track *gnd_lower = new track(0, 1/20.0), *gnd_upper = new track(0, 1/20.0);
  for (unsigned i = 0; i < 6; ++i) {
    gnd_lower->add_point(i - 0.1, 0.1); // cap
    gnd_upper->add_point(i + 0.4, 0.85);
    gnd_lower->add_point(i + 0.7, 0.1); // IC ground pin
    gnd_upper->add_point(i + 1.2, 0.85);
    
    (new track(0, 1/20.0))->add_point(i - 0.1, 0.1).add_point(i - 0.1, 0);
    (new track(0, 1/20.0))->add_point(i + 0.7, 0.1).add_point(i + 0.7, 0);
    (new track(0, 1/20.0))->add_point(i + 0.4, 0.85).add_point(i + 0.4, 0.75);
    (new track(0, 1/20.0))->add_point(i + 1.2, 0.85).add_point(i + 1.2, 0.75);
  }
  (new track(0, 1/20.0))-> // Grounded input on U8
    add_point(0.4, 0.85).add_point(0.7, 0.85).add_point(0.7, 1.05);
  (new track(1, 1/20.0))-> // Ground inputs for tstrip
    add_point(5.7, 0).add_point(5.7,-0.6);
  (new track(0, 1/20.0))->
    add_point(5.6, -0.6).add_point(5.7,-0.6).add_point(5.8,-0.6);
  (new track(0, 1/20.0))-> // Connect lower and upper ground tracks
    add_point(5.7, 0.1).add_point(6.2,0.1);
  (new track(1, 1/20.0))->
    add_point(6.2,0.1).add_point(6.2,0.6).add_point(6.2,0.75);
  (new track(0, 1/20.0))->
    add_point(6.2,0.6).add_point(6.4,0.6).add_point(6.4,0.5);
  (new track(0, 1/20.0))->
    add_point(6.4,0.6).add_point(7.2, 0.6).add_point(7.2,0.5);
  (new via(point(6.2,0.1), 0.08, 0.035));
  (new via(point(6.2,0.6), 0.08, 0.035));
  
  map<string, net*> nets;
  load_nets(c, nets);

  wire::expand_nets();  
  wire::check_nets();

  nets["vdd"]->mark();
}

int main() {
  using namespace libpcb;

  make_board();
  
  // net::print_nets();

  //gnd.mark();
  //vdd.mark();
  
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

