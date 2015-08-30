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



int main(int argc, char **argv) {
  read_pmap();
  read_netlist("final.nand");
}
