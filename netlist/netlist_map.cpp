#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

map<string, set<int> > vdd, gnd; // Pins designated as rails
map<string, vector<int> > pinmap; // device -> pin numbers in CHDL order
map<string, string> dmap; // Designator codes for various devices
map<string, int> bom_count; // designator class to count
map<string, string> bom; // designator to dev type
map<string, set<string> > rbom; // dev type to designator set
map<string, vector<pair<string, int> > > netlist; // net to designator and pin

string read_chdl_module(istream &in) {
  string r; char c;
  while (!!in && in.get() != '<');
  while (!!in && ((c = in.get()) != '>')) r = r + c;

  return r;
}

void read_pmap() {
  ifstream in("PMAP");

  while (!!in) {
    string dev = read_chdl_module(in);
    int idx = 1;
    while (!!in && in.peek() != '\n') {
      string pin;
      in >> pin;
      if (!in) break;
      if (pin == "vdd") vdd[dev].insert(idx);
      else if (pin == "gnd") gnd[dev].insert(idx);
      else if (pin == "nc") /* do nothing */;
      else {
	istringstream iss(pin);
	int pnum(-1);
	iss >> pnum;
	if (pnum > pinmap[dev].size()) pinmap[dev].resize(pnum);
	pinmap[dev][pnum - 1] = idx;
      }
      idx++;
    }
  }

  for (auto &x : pinmap) {
    cout << x.first << ':';
    for (auto &y : x.second)
      cout << ' ' << y;
    cout << endl;
  }
}

void read_tmap() {
  ifstream in("TMAP");

  while (!!in) {
    string dev = read_chdl_module(in), des;
    in >> des;
    if (!in) break;
    dmap[dev] = des;
    cout << "dmap[\"" << dev << "\"] = " << des << ';' << endl;
  }
}

string bom_new(string dev) {
  bom_count[dmap[dev]]++;

  int idx = bom_count[dmap[dev]];
  ostringstream des;
  des << dmap[dev] << idx;

  bom[des.str()] = dev;
  rbom[dev].insert(des.str());

  return des.str();
}

void add_global_net(map<string, set<int> > &g, string net) {
  for (auto &x : g) {
    for (auto &des : rbom[x.first]) {
      for (auto pin : x.second) {
	netlist[net].push_back(make_pair(des, pin));
        cout << net << ", add " << des << " pin " << pin << endl;
      }
    }
  }
}

void read_netlist(string filename) {
  ifstream in(filename);

  // Read in the netlist
  while (!!in) {
    string dev = read_chdl_module(in);
    if (!in) break;
    string des = bom_new(dev);
    cout << "Got " << dev << ", " << des << endl;
    int idx = 0;
    while (!!in && in.peek() != '\n') {
      string net;
      in >> net;
      netlist[net].push_back(make_pair(des, pinmap[dev][idx]));
      cout << net << ", add " << des << " pin " << pinmap[dev][idx] << endl;
      idx++;
    }
  }

  // Add the vdd and gnd pins to the netlist
  add_global_net(vdd, "vdd");
  add_global_net(gnd, "gnd");
}

void dump_bom() {
  ostream &out(cout);
  for (auto &x : rbom) {
    cout << x.first << ':';
    for (auto &y : x.second)
      cout << ' ' << y;
    cout << endl;
  }
}

void dump_netlist() {
  ostream &out(cout);

  for (auto &x : netlist) {
    out << x.first;
    for (auto &y : x.second)
      out << ' ' << y.first << ' ' << y.second;
    out << endl;
  }
}

int main(int argc, char **argv) {
  read_pmap();
  read_tmap();
  read_netlist("final.nand");
  cout << "=== BOM ===" << endl;
  dump_bom();
  cout << "=== NETS ===" << endl;
  dump_netlist();
}
