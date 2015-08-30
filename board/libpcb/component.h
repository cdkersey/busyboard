#ifndef LIBPCB_COMPONENT_H
#define LIBPCB_COMPONENT_H

#include <string>
#include <map>
#include <set>
#include <string>

#include "pin.h"

namespace libpcb {

class component {
public:
  component(std::string name): name(name) {}
  virtual ~component() {}
  std::string get_name() const { return name; }
  virtual std::string get_type_name() = 0;
  pin &get_pin(std::string name) { return pins.find(name)->second; }

protected:
  void add_pin(std::string name, point loc)
    { pins[name] = pin(name, loc); }
  
  void add_pin(std::string name, point loc, const std::set<layer> &l)
    { pins[name] = pin(name, loc, l); }
  
  std::map<std::string, pin> pins;
  std::string name;
};

}

#endif
