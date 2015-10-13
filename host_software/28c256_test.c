/* EEPROM Program/verify test. */
/* Ports:
    A0: #ce A1: #oe A2: #wr
    B - Data
    C - Address[7:0]
    D - Address[15:8]
    E - Address[23:16]
    F
*/

#include <stdio.h>
#include <stdlib.h>

#include "busyboard.h"

#define DELAY 100000

void do_delay(void) {
  #ifdef DELAY
  usleep(DELAY);
  #endif
}

void eeprom_init(busyboard_t *bb) {
  bb->out_state[0] = 7; // CE, OE, and WR de-asserted
  bb->trimask = 0x3d;
  busyboard_out(bb);

  do_delay();
}

void eeprom_write(busyboard_t *bb, unsigned addr, unsigned char data) {
  bb->out_state[0] = 6; // CE asserted
  bb->out_state[1] = data;
  bb->out_state[2] = addr & 0xff;
  bb->out_state[3] = (addr >> 8) & 0xff;
  bb->out_state[4] = (addr >> 16) & 0xff;
  bb->trimask = 0x3f;
  busyboard_out(bb);
  do_delay();
  
  bb->out_state[0] = 2; // CE and WR asserted, OE de-asserted
  busyboard_out(bb);
  do_delay();

  bb->out_state[0] = 6; // de-assert WR
  busyboard_out(bb);
  do_delay();
  
  eeprom_init(bb);
}

unsigned char eeprom_read(busyboard_t *bb, unsigned addr)
{
  bb->out_state[0] = 4; // CE and OE asserted, WR clear.
  bb->trimask = 0x3d;
  bb->out_state[2] = addr & 0xff;
  bb->out_state[3] = (addr >> 8) & 0xff;
  bb->out_state[4] = (addr >> 16) & 0xff;
  busyboard_out(bb);
  do_delay();
  busyboard_in(bb);
  do_delay();

  eeprom_init(bb);
  
  return bb->in_state[1];
}

int main(int argc, char **argv) {
  busyboard_t bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");

  unsigned int i, count;
  srand(0x1234);
  for (i = 0; i < (1<<15); ++i) {
    unsigned val = rand() & 0xff;
    eeprom_write(&bb, i, val);
    printf("Wrote %x to address %x.\n", val, i);
  }

  srand(0x1234);
  for (i = 0, count = 0; i < (1<<15); ++i) {
    unsigned val = eeprom_read(&bb, i);
    printf("Read %x from address %x.\n", val, i);
    if (val == (rand() & 0xff)) count++;
    else puts("Warning: value mismatch.");
  }

  printf("%d values matched.\n", count);

  close_busyboard(&bb);

  return 0;
}
