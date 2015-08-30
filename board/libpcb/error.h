// error.h - Define error codes and an error handling function.

#ifndef LIBPCB_ERROR_H
#define LIBPCB_ERROR_H

namespace libpcb {

enum errcode {
  ERR_MODE_SET, ERR_POINT_SET, ERR_APERTURE_SET
};

void err(errcode e);

}

#endif
