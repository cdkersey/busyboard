// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <libpcb/text.h>

using namespace std;
using namespace libpcb;

int main() {
  using namespace libpcb;

  new text(get_default_font(), LAYER_SILKSCREEN, point(-2, 0),
	   "The quick brown fox jumps over the lazy dog.",
	   1.0/32);

  new text(get_default_font(), LAYER_SILKSCREEN, point(-2, -0.5),
          "1-244-867-5309",
	   1.0/32);

  new text(get_default_font(), LAYER_SILKSCREEN, point(-2, -1),
           "ABCDEFGHIJKLMNOPQRSTUVWXYZzyxwvutsrqponmlkjihgfedcba",
           1.0/32);

  ofstream gfile("text.ss.grb");
  gerber g(gfile);
  drawable::draw_layer(LAYER_SILKSCREEN, g);

  return 0;
}
