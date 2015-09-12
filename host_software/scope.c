/* Busyboard control program/library */
#include <stdio.h>

#include "busyboard.h"

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  int i, j;
  for (;;) {
    bb.trimask = 0;
    busyboard_out(&bb);
    busyboard_in(&bb);
    for (i = 0; i < BUSYBOARD_N_PORTS; i++)
      for (j = 0; j < 8; j++)
        printf("%d", (bb.in_state[i]>>j)&1);
    putc('\n', stdout);
    //usleep(1000);
  }
  
  close_busyboard(&bb);

  return 0;
}
