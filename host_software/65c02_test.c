/* 65c02 CPU test. */
/* Ports:
    A - {phi2, #irq, #nmi, #res, be, ready, #so} O (busyboard->65c02)
    B - Data (I/O)
    C - Address[7:0] I (65c02->busyboard)
    D - Address[15:8] I (65c02->busyboard)
    E - {#ml, sync, #rw, #vp} I (65c02->busyboard)
*/

#include <stdio.h>
#include <stdlib.h>

#include "busyboard.h"

// #define DELAY 1000

void do_delay(void) {
  #ifdef DELAY
  usleep(DELAY);
  #endif
}

void set_clk(busyboard_t *bb) {
  bb->out_state[0] |= 1;
  busyboard_out(bb);

  do_delay();
}

void clear_clk(busyboard_t *bb) {
  bb->out_state[0] &= ~1;
  busyboard_out(bb);

  do_delay();
}

void assert_reset(busyboard_t *bb) {
  bb->out_state[0] &= ~8;
  busyboard_out(bb);

  do_delay();
}

void deassert_reset(busyboard_t *bb) {
  bb->out_state[0] |= 8;
  busyboard_out(bb);

  do_delay();
}

void cpu_init(busyboard_t *bb) {
  int i;
  
  bb->out_state[0] = ~1; // clk clear, inverted control signals de-asserted
  bb->trimask = 1; // control bus output, everything else input
  assert_reset(bb);

  // 10 clocks with reset asserted.
  for (i = 0; i < 10; ++i) { set_clk(bb); clear_clk(bb); }

  deassert_reset(bb); // Now we're ready to go.
}

int cpu_get_addr(busyboard_t *bb) {
  busyboard_in(bb);
  return (bb->in_state[3]<<8) | bb->in_state[2];
}

enum cpu_status {
  STATUS_ML = 0x01,
  STATUS_SYNC = 0x02,
  STATUS_WR = 0x04,
  STATUS_VP = 0x08
};

const char *cpu_status_str[] = {
  "ml", "sync", "wr", "vp"
};

int cpu_get_status(busyboard_t *bb) {
  busyboard_in(bb);
  return bb->in_state[4] ^ 0xd;
}

void print_data_bus(busyboard_t *bb) {
  if (bb->trimask & 2) {
    // output
    printf(" %02x(O)", bb->out_state[1]);
  } else {
    // input
    printf(" %02x(I)", bb->in_state[1]);
  }
}

void print_bus_status(busyboard_t *bb) {
  int i, addr = cpu_get_addr(bb), status = cpu_get_status(bb);
  printf("addr: %04x", addr);
  print_data_bus(bb);
  for (i = 0; i < 4; i++) if ((status >> i)&1) printf(" %s", cpu_status_str[i]);
  putc('\n', stdout);
}

unsigned char mem[0x10000];

void load_hex(int base, const char *filename) {
  FILE *f = fopen(filename, "r");
  int i = 0;
  while (!feof(f)) {
    unsigned int val;
    fscanf(f, "%x\n", &val);
    mem[base + i++] = val;
  }
  printf("Initialized memory from %s with %d bytes.\n", filename, i);
}

void dump_hex() {
  int i, j;
  for (i = 0; i < 0x10000; i += 16) {
    printf("%04x: ", i);
    for (j = 0; j < 16; j++) {
      printf("%02x", (unsigned int)mem[i + j]);
      if (j != 15) {
	putc(' ', stdout);
        if (j % 4 == 3) putc(' ', stdout);
      }
    }
    putc('\n', stdout);
  }
}

void cpu_emulate_cyc(busyboard_t *bb) {
  int addr = cpu_get_addr(bb), status = cpu_get_status(bb);

  if (!(status & STATUS_WR)) {
    bb->out_state[1] = mem[addr];

    bb->trimask |= 2;
  } else {
    bb->trimask &= ~2;
    busyboard_out(bb);
    if (status & STATUS_WR) {
      busyboard_in(bb);
      unsigned val = bb->in_state[1];
      mem[addr] = val;
    }
  }

  busyboard_out(bb);
}

int main(int argc, char **argv) {
  int i;
  busyboard_t bb;
  load_hex(0x800, "sieve.hex");

  // Set initial PC
  mem[0xfffc] = 0x00;
  mem[0xfffd] = 0x08;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");
  cpu_init(&bb);

  for (i = 0; i < 20000; ++i) {
    if (i & 1) set_clk(&bb);
    else clear_clk(&bb);

    cpu_emulate_cyc(&bb);
    print_bus_status(&bb);
  }

  dump_hex();
  
  close_busyboard(&bb);

  return 0;
}
