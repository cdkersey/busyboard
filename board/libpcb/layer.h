#ifndef LIBPCB_LAYER_H
#define LIBPCB_LAYER_H

namespace libpcb {

const unsigned N_CU(4); // Number of total copper layers in layer enum.

enum layer {
  LAYER_SILKSCREEN,                           // Screen
  LAYER_MASK0, LAYER_MASK1,                   // Solder mask (inverted)
  LAYER_CU0, LAYER_CU1, LAYER_CU2, LAYER_CU3, // Copper
  LAYER_PTH, LAYER_NPTH                       // Thru-holes (plated/non)
};

}

#endif
