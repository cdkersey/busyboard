/* Busyboard control program/library */

#include "busyboard.h"

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  unsigned char a[] = {
    0x7e, 0xff, 0xc3, 0xc3, 0x66, 0x00,       /* C */
    0xff, 0xff, 0x10, 0x10, 0xff, 0xff, 0x00, /* H */
    0xff, 0xff, 0x81, 0x81, 0x7e, 0x00,       /* D */
    0xff, 0xff, 0x80, 0x80, 0xc0              /* L */
  };
  
  int i, j;
  for (;;) {
    for (j = 0; j < 32; ++j) {
      bb.trimask = 0x3f;
      for (i = 0; i < BUSYBOARD_N_PORTS; i++)
	bb.out_state[i] = ((j < 24) ? a[j%32] : 0);
      busyboard_out(&bb);
      busyboard_in(&bb);
      for (i = 0; i < 6; ++i)
	if (bb.in_state[i] != bb.out_state[i])
	  puts("Warning: state mismatch.");
      busyboard_in(&bb);
      for (i = 0; i < 6; ++i)
        if (bb.in_state[i] != bb.out_state[i])
	  puts("Warning: state mismatch.");
      usleep(1500);
    }
  }
  
  close_busyboard(&bb);

  return 0;
}
