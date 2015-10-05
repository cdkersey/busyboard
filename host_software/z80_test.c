/* SRAM Read/write test. */
/* Ports:
    A0: {clk, #int, #nmi, #reset, #busreq, #wait} O (busyboard->z80)
    B - Data (I/O)
    C - Address[7:0] I (z80->busyboard)
    D - Address[15:8] I (z80->busyboard)
    E - {#mreq, #iorq, #halt, #rfsh, #busack, #rd, #wr, #m1} O (z80->busyboard)
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

void z80_init(busyboard_t *bb) {
  int i;
  
  bb->out_state[0] = ~1; // clk clear, inverted control signals de-asserted
  bb->trimask = 1; // control bus output, everything else input
  assert_reset(bb);

  // 10 clocks with reset asserted.
  for (i = 0; i < 10; ++i) { set_clk(bb); clear_clk(bb); }

  deassert_reset(bb); // Now we're ready to go.
}

int z80_get_addr(busyboard_t *bb) {
  busyboard_in(bb);
  return (bb->in_state[3]<<8) | bb->in_state[2];
}

enum z80_status {
  Z80_STATUS_MREQ = 0x01,
  Z80_STATUS_IORQ = 0x02,
  Z80_STATUS_HALT = 0x04,
  Z80_STATUS_RFRSH = 0x08,
  Z80_STATUS_BUSACK = 0x10,
  Z80_STATUS_RD = 0x20,
  Z80_STATUS_WR = 0x40,
  Z80_STATUS_M1 = 0x80
};

const char *z80_status_str[] = {
  "mreq", "iorq", "halt", "rfrsh", "busack", "rd", "wr", "m1"
};

int z80_get_status(busyboard_t *bb) {
  busyboard_in(bb);
  return ~bb->in_state[4];
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
  int i, addr = z80_get_addr(bb), status = z80_get_status(bb);
  printf("addr: %04x", addr);
  print_data_bus(bb);
  for (i = 0; i < 8; i++) if ((status >> i)&1) printf(" %s", z80_status_str[i]);
  putc('\n', stdout);
}

unsigned char mem[0x10000];

void load_hex(const char *filename) {
  FILE *f = fopen(filename, "r");
  int i = 0;
  while (!feof(f)) {
    unsigned int val;
    fscanf(f, "%x\n", &val);
    mem[i++] = val;
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

void z80_emulate_cyc(busyboard_t *bb) {
  int addr = z80_get_addr(bb), status = z80_get_status(bb);

  if (status & Z80_STATUS_RD) {
    if (status & Z80_STATUS_MREQ) {
      bb->out_state[1] = mem[addr];
    } else if (status & Z80_STATUS_IORQ) {
      bb->out_state[1] = 0;
    }

    bb->trimask |= 2;
  } else {
    bb->trimask &= ~2;
    busyboard_out(bb);
    if (status & Z80_STATUS_WR) {
      busyboard_in(bb);
      unsigned val = bb->in_state[1];
      if (status & Z80_STATUS_MREQ) {
	mem[addr] = val;
      } else if (status & Z80_STATUS_IORQ) {
	printf("I/O write, port %02x, val %02x\n", addr&0xff, mem[addr]);
      }
    }
  }

  busyboard_out(bb);
}

int main(int argc, char **argv) {
  int i;
  busyboard_t bb;
  load_hex("hello.hex");
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");
  z80_init(&bb);

  for (i = 0; i < 100000; ++i) {
    set_clk(&bb);
    z80_emulate_cyc(&bb);
    clear_clk(&bb);
    print_bus_status(&bb);
  }

  dump_hex();
  
  close_busyboard(&bb);

  return 0;
}
