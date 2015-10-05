/* Busyboard control program/library */
#include <stdio.h>

#include "busyboard.h"

/* SPI test: pinout
     A0 - CLK     A1 - MOSI (master->slave data)     A2 - CS0     A3 - CS1
     A4 - CS2     A5 - CS3                           A6 - CS4     A7 - CS5
     B0 - MISO (slave->master data)

     This allows support for up to 6 SPI devices on the same bus.
*/

void do_delay() {
  //usleep(10000);
}

void spi_init(struct busyboard *bb) {
  bb->trimask = 1;
  bb->out_state[0] = 0xfc;
  busyboard_out(bb);
}

void spi_clear_cs(struct busyboard *bb) {
  bb->out_state[0] |= ~3;
  busyboard_out(bb);
}

void spi_set_cs(struct busyboard *bb, int id) {
  unsigned char csbit = 1<<(id + 2);
  bb->out_state[0] |= ~3;
  bb->out_state[0] &= ~csbit;
  busyboard_out(bb);
}

void spi_set_clk(struct busyboard *bb) {
  bb->out_state[0] |= 1; /* Set clk */
  busyboard_out(bb);
}

void spi_clear_clk(struct busyboard *bb) {
  bb->out_state[0] &= ~1; /* Set clk */
  busyboard_out(bb);
}

int spi_adc_read(struct busyboard *bb) {
  int i, val;
  spi_set_cs(bb, 0);

  for (i = 0; i < 3; ++i) {
    do_delay();
    spi_set_clk(bb);
    do_delay();
    spi_clear_clk(bb);
  }

  for (i = val = 0; i < 10; ++i) {
    do_delay();
    spi_set_clk(bb);
    do_delay();
    busyboard_in(bb);
    val = (val << 1) | (bb->in_state[1] & 1);
    spi_clear_clk(bb);
  }

  spi_clear_cs(bb);

  return val;
}

void plot(int x) {
  #define GRAPH
  #ifdef GRAPH
  x = x * 79 / 1023;
  int i;
  for (i = 0; i < x; ++i) putc(' ', stdout);
  putc('*', stdout);
  putc('\n', stdout);
  #else
  printf("%d\n", x);
  #endif
}

int main(int argc, char **argv) {
  int i;
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  spi_init(&bb);
  spi_clear_cs(&bb);

  for (i = 0; i < 1000000; i++) plot(spi_adc_read(&bb));
  
  close_busyboard(&bb);

  return 0;
}
