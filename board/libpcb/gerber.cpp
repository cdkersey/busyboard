#include <iomanip>
#include <sstream>

#include "gerber.h"
#include "error.h"

using namespace std;
using namespace libpcb;

// Set the current aperature (only circular supported)
void libpcb::gerber::set_aperture(double d) {
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
void libpcb::gerber::move(point to) {
  point_set = true;
  coord(to);
  out << "D02*" << endl;
  p = to;
}

// Draw line segments in current aperture
void libpcb::gerber::draw(point to) {
  if (!mode_region) {
    if (!point_set) err(ERR_POINT_SET);
    if (!mode_set) err(ERR_MODE_SET);
    if (!aperture_set) err(ERR_APERTURE_SET);
  }
  
  coord(to);
  out << "D01*" << endl;
  p = to;
}

void libpcb::gerber::draw(point from, point to) {
  if (!point_set || from != p)
    move(from);

  draw(to);
}

// Draw "flash" in current aperture.
void libpcb::gerber::flash() {
  if (!point_set) err(ERR_POINT_SET);
  if (!mode_set) err(ERR_MODE_SET);
  if (!aperture_set) err(ERR_APERTURE_SET);
  
  out << "D03*" << endl;
}

void libpcb::gerber::flash(point pt) {
  point_set = true;
  coord(pt);
  flash();

  p = pt;
}

void libpcb::gerber::init() {
  out << "G04 Output from Chad's Board Thing.*" << endl
      << "%FSLAX26Y26*%" << endl
    // << "%TF.Part,Other*%" << endl // TODO: layer name
      << "%MOIN*%" << endl;
}

// Output coordinate in COBOLesque gerber format.
void libpcb::gerber::coord(point pt) {
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
void libpcb::gerber::num(double x) {
  if (x < 0) {
    out << '-';
    x = -x;
  }
  
  out << int(x) << setw(6) << setfill('0') << int(fmod(x, 1)*1000000);
}

void libpcb::gerber::set_dark() {
  if (mode_region) err(ERR_MODE_SET);
  
  if (!mode_set || (mode_set && mode_clear))
    out << "%LPD*%" << endl;
  mode_set = true;
  mode_clear = false;
}

void libpcb::gerber::set_clear() {
  if (mode_region) err(ERR_MODE_SET);

  if (!mode_set || (mode_set && !mode_clear))
    out << "%LPC*%" << endl;
  mode_set = true;
  mode_clear = true;
}

void libpcb::gerber::start_region() {
  if (!mode_region) {
    out << "G36*" << endl;

    mode_set = mode_clear = false;
    mode_region = true;
  } else {
    err(ERR_MODE_SET);
  }
}

void libpcb::gerber::end_region() {
  if (mode_region) {
    out << "G37*" << endl;

    mode_region = false;
  } else {
    err(ERR_MODE_SET);
  }
}
