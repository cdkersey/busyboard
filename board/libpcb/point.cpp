#include <cmath>

#include "point.h"

using namespace libpcb;
using namespace std;

static double quantize(double x, double step) {
  x = round(x * (1/step))/step;
  return x;
}

point libpcb::point::neighborhood() const {
  return point(quantize(x, THRESH), quantize(y, THRESH));
}

ostream &libpcb::operator<<(ostream &o, const point &p) {
  o << '(' << p.x << ", " << p.y << ')';
}
