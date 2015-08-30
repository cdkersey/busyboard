#ifndef LIBPCB_NET_H
#define LIBPCB_NET_H

#include "point.h"

#include <set>
#include <map>

namespace libpcb {

class net;

class wire {
public:
  wire(): n(NULL) { wires.insert(this); }
  virtual ~wire() { wires.erase(this); }
  virtual void get_points(std::set<std::pair<point, int> > &s) = 0;

  void mark();
  
  void set_net(net *p) { n = p; }
  net *get_net() { return n; }

  static void print_wires();
  static void expand_nets();
  static void check_nets();

private:
  net *n;
  static std::set<wire*> wires;
};


class net {
public:
  net(std::string name): name(name) { nets.insert(this); }
  virtual ~net() { nets.erase(this); }

  void add_wire(wire &w) { wires.insert(&w); w.set_net(this); }
  std::string get_name() { return name; }

  void mark() { for (auto w : wires) w->mark(); }
  
  static void print_nets();

private:
  std::set<wire*> wires;
  std::string name;

  static std::set<net*> nets;
};

}

#endif
