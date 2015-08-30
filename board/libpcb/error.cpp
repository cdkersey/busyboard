// Using the current set of open-source PCB layout tools feels like using
// MS-paint for Windows 3.1 with hams duct-taped to my fingers. This is an
// attempt to make something a little less bad. If you get it, you get it. If
// not, please tell me what I'm doing wrong.

#include <iostream>
#include <cstdlib>

#include "error.h"

using namespace std;
using namespace libpcb;

void libpcb::err(errcode e) {
  cerr << "ERROR: ";
  
  switch(e) {
  case ERR_POINT_SET: cerr << "Point not set." << endl;      break;
  case ERR_MODE_SET:  cerr << "Dark/clear not set." << endl; break;
  case ERR_APERTURE_SET: cerr << "Aperture not set." << endl; break;
  }

  abort();
}
