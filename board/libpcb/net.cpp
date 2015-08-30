#include <vector>

#include "text.h"
#include "net.h"

using namespace std;
using namespace libpcb;

set<wire*> libpcb::wire::wires;
set<net*> libpcb::net::nets;

void libpcb::wire::print_wires() {
  for (auto &w : wires) {
    set<pair<point, int> > s;
    w->get_points(s);

    cout << "Wire " << &w << ':';
    for (auto &x : s)
      cout << ' ' << x.first << ' ' << x.second;
    cout << endl;
  }
}

void libpcb::wire::mark() {
  set<pair<point, int> > s;
  get_points(s);
  for (auto &p : s)
    new text(get_default_font(), LAYER_SILKSCREEN, p.first, "X", 1/80.0);
}

void libpcb::net::print_nets() {
  for (auto p : nets) {
    cout << p->name << ":" << endl;
    for (auto wp : p->wires)
      cout << "  " << wp << endl;
  }
}

void libpcb::wire::expand_nets() {
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
void libpcb::wire::check_nets() {
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
