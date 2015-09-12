#ifndef BUSYBOARD_H
#define BUSYBOARD_H

/* Busyboard control program/library */

/* Busyboard control structure. */
#define BUSYBOARD_N_PORTS 6

struct busyboard {
  int fd; /* Parallel port file descriptor. */
  unsigned trimask; /* One bit per I/O byte tristate mask, 1=out 0=Hi-Z */
  unsigned char out_state[BUSYBOARD_N_PORTS],
                in_state[BUSYBOARD_N_PORTS];
};

void init_busyboard(struct busyboard *b, const char *devnode);
void close_busyboard(struct busyboard *b);

/* Write outstate, trimask to board. */
void busyboard_out(struct busyboard *b);

/* Read in_state from board. */
void busyboard_in(struct busyboard *b);

#endif
