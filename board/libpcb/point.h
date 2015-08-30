#ifndef LIBPCB_POINT_H
#define LIBPCB_POINT_H

#include <iostream>
#include <cmath>

namespace libpcb {

struct point {
  point(): x(0), y(0) {}
  point(double x, double y): x(x), y(y) {}
  
  double x, y;

  bool operator==(const point &r) const { return x == r.x && y == r.y; }

  bool operator!=(const point &r) const { return !operator==(r); }
  point operator+(const point &r) const { return point(x + r.x, y + r.y); }
  point operator-(const point &r) const { return point(x - r.x, y - r.y); }
  
  point operator+=(const point &r) { x += r.x; y += r.y; }
  point operator-=(const point &r) { x -= r.x; y -= r.y; }

  bool operator<(const point &r) const {
    return (x < r.x) || ((x == r.x) && y < r.y);
  }

  point neighborhood() const;

  point scale(double r) const { return point(r*x, r*y); }

  double r2() const { return x*x + y*y; }

  double d2(const point &r) const {
    return std::pow(r.x - x, 2) + std::pow(r.y - y, 2);
  }

  constexpr static double THRESH = 1e-4;
};

std::ostream &operator<<(std::ostream &o, const point &p);
 
}

#endif
