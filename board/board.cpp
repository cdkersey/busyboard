// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <libpcb/basic.h>
#include <libpcb/track.h>
#include <libpcb/component.h>

using namespace std;
using namespace libpcb;

int main() {
  using namespace libpcb;

  net &vdd(*(new net("VDD"))), &gnd(*(new net("GND"))), &x(*(new net("X")));

  new plane(LAYER_CU0, point(-3, -3), point(3, 3));
  new plane(LAYER_CU1, point(-3, -3), point(3, 3));
  
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
	   "1-244-867-5309?$@*&#AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz",
	   1.0/16);

  DIP16 &u0(*(new DIP16("U0", point(0, -0.15))));

  vdd.add_wire(u0.get_pin("8"));
  gnd.add_wire(u0.get_pin("16"));
  gnd.add_wire(u0.get_pin("1"));
  gnd.add_wire(u0.get_pin("2"));
  gnd.add_wire(u0.get_pin("3"));

  new R3("R0", point(0.7, 0.5));
  new R6("R1", point(0.7, -0.5));

  (new track(1, 1/20.0))->
    add_point(1.0, 0.5).add_point(1.0,0).add_point(1.3, -0.5);
  (new track(1, 1/20.0))->
    add_point(1.3, -0.5).add_point(1.7, -0.5);
  
  new R3V("R3", point(1.7, -1.0));
  new Dl<1, true>("D3", point(1.7, -0.6));
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

