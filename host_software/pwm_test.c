/* ~100Hz Pulse-width modulation test. */
#include <math.h>
#include <stdio.h>

#include "busyboard.h"

void pwm_cycle(struct busyboard *bb, double duty) {
  int a = duty*10000, b = 10000 - a;
  //printf("%d %d\n", a, b);
  /* Turn the output on. */
  if (a) {
    bb->out_state[0] |= 1;
    busyboard_out(bb);
  }
  
  usleep(a);
  
  /* Turn the output off. */
  if (b) {
    bb->out_state[0] &= ~1;
    busyboard_out(bb);
  }
  
  usleep(b);
}

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");
  bb.trimask = 0x3f;
  busyboard_out(&bb);
  
  int i, j;
  for (i = 0; i < 100; ++i)
    for (j = 0; j < 300; ++j)
      pwm_cycle(&bb, pow((sin(2*M_PI*(j/300.0)) + 1)/2.0, 3));

  close_busyboard(&bb);

  return 0;
}
