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
  // usleep(1000);
}

void spi_init(struct busyboard *bb) {
  bb->trimask = 1;
  bb->out_state[0] = 0xfc;
  busyboard_out(bb);
}

void spi_send_byte(struct busyboard *bb, int id, unsigned char x) {
  int i;
  for (i = 0; i < 8; i++, x <<= 1) {
    if (x & 0x80) bb->out_state[0] |= 2; /* Set MOSI to data value. */
    else          bb->out_state[0] &= ~2;
    busyboard_out(bb);
    do_delay();

    bb->out_state[0] |= 1; /* Set clk */
    busyboard_out(bb);
    do_delay();

    bb->out_state[0] &= ~1; /* Clear clk. */
    /* Will be output alongside the next data bit. */
  }
}

void spi_rec_byte(struct busyboard *bb, int id, unsigned char *x) {
  int i;
  for (i = 0, *x = 0; i < 8; i++) {
    *x <<= 1;

    bb->out_state[0] |= 1; /* Set clk */
    busyboard_out(bb);
    do_delay();

    bb->out_state[0] &= ~1; /* Clear clk. */
    busyboard_out(bb);
    do_delay();

    busyboard_in(bb);
    if (bb->in_state[1] & 1) *x |= 1;
  }
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

void spi_send(struct busyboard *bb, int id, unsigned char *buf, int len) {
  int i;
  
  spi_set_cs(bb, id);
  do_delay();
  
  /* Send bytes */
  for (i = 0; i < len; i++)
    spi_send_byte(bb, id, buf[i]);
}

void spi_rec(struct busyboard *bb, int id, unsigned char *buf, int len) {
  int i;
  
  /* Set up chip select */
  spi_set_cs(bb, id);
  do_delay();
  
  /* Send bytes */
  for (i = 0; i < len; i++)
    spi_rec_byte(bb, id, &buf[i]);
}

void spi_sram_write(struct busyboard *bb, int id, int addr, unsigned char data)
{
  printf("Write %x: %x\n", addr, (unsigned int)data);
  
  spi_clear_cs(bb);
  do_delay();

  char buf[4];
  buf[0] = 0x02; /* Write command */
  buf[1] = (addr >> 16) & 0xff; /* Address */
  buf[2] = (addr >> 8) & 0xff;
  buf[3] = addr & 0xff;
  buf[4] = data;

  spi_send(bb, id, buf, 5);
  
  spi_clear_cs(bb);
  do_delay();
}

unsigned char spi_sram_read(struct busyboard *bb, int id, int addr) {
  spi_clear_cs(bb);
  do_delay();

  char buf[4];
  buf[0] = 0x03; /* Read command */
  buf[1] = (addr >> 16) & 0xff; /* Address */
  buf[2] = (addr >> 8) & 0xff;
  buf[3] = addr & 0xff;

  spi_send(bb, id, buf, 4);

  unsigned char val;
  spi_rec_byte(bb, id, &val);
  
  spi_clear_cs(bb);
  do_delay();
  
  return val;
}

unsigned char spi_sram_rdmr(struct busyboard *bb, int id) {
  char buf[1];
  buf[0] = 0x05; /* Read mode register */
  
  spi_send(bb, id, buf, 1);

  unsigned char val;
  spi_rec_byte(bb, id, &val);

  spi_clear_cs(bb);
  do_delay();

  return val;
}

void spi_sram_wrmr(struct busyboard *bb, int id, unsigned char mr) {
  char buf[2];
  buf[0] = 0x01;
  buf[1] = mr;

  spi_send(bb, id, buf, 2);

  spi_clear_cs(bb);
  do_delay();
}

void spi_sram_reset(struct busyboard *bb, int id) {
  char buf[1];
  buf[0] = 0xff;
  spi_send(bb, id, buf, 1);
  spi_clear_cs(bb);
  do_delay();
}

#define RSEED 100

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  spi_init(&bb);

  //spi_sram_reset(&bb, 0);

  spi_sram_wrmr(&bb, 0, 0x00);
  
  unsigned mr = spi_sram_rdmr(&bb, 0);
  printf("Mode register: 0x%x\n", mr);

  srand(RSEED);
  
  int i;
  for (i = 0; i < (1<<8); i++)
    spi_sram_write(&bb, 0, i, rand() & 0xff);

  srand(RSEED);

  int count = 0;
  for (i = 0; i < (1<<8); i++) {
    int x = spi_sram_read(&bb, 0, i);
    printf("%x: %x\n", i, x);
    if (x == (rand() & 0xff)) ++count;
  }

  printf("%d matching positions.\n", count);
  
  close_busyboard(&bb);

  return 0;
}
