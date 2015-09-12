/* Busyboard control program/library */

#include "busyboard.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>
#include <linux/parport.h>

// #define DELAY 100

int open_parport(const char *devnode); /* Open and init parallel port. */
void close_parport(int fd);

enum ppbit { BIT_STROBE, BIT_DATA, BIT_LATCH_OUT, BIT_LATCH_IN, BIT_N_LD_IN };
const char *ppbit_name[] = {
  "BIT_STROBE", "BIT_DATA", "BIT_LATCH_OUT", "BIT_LATCH_IN", "BIT_N_LD_IN"
};
int inverted[] = { 1, 1, 1, 1, 1 };
int bit_pp[] = { PARPORT_CONTROL_STROBE, 0, PARPORT_CONTROL_AUTOFD, PARPORT_CONTROL_AUTOFD,  PARPORT_CONTROL_SELECT };

void set_bit(int fd, int bit, int val);
int read_data(int fd);

void init_busyboard(struct busyboard *b, const char *devnode) {
  int i;
  
  b->fd = open_parport(devnode);
  b->trimask = 0;

  for (i = 0; i < BUSYBOARD_N_PORTS; ++i) b->out_state[i] = 0;

  set_bit(b->fd, BIT_DATA, 0);
  set_bit(b->fd, BIT_LATCH_OUT, 0);
  set_bit(b->fd, BIT_LATCH_IN, 0);
  set_bit(b->fd, BIT_STROBE, 0);
  set_bit(b->fd, BIT_N_LD_IN, 1);
}

void close_busyboard(struct busyboard *b) {
  close_parport(b->fd);
}

int open_parport(const char *devnode) {
  int fd = open(devnode, O_RDWR);
  if (fd == -1) {
    perror("Could not open parport: ");
    exit(1);
  }

  int result = ioctl(fd, PPCLAIM);
  if (result) {
    perror("Claim of parport failed: ");
    exit(1);
  }
  
  return fd;
}

void close_parport(int fd) {
  ioctl(fd, PPRELEASE);
  close(fd);
}

void busyboard_out(struct busyboard *b) {
  int i, j;

  // Write out the tristate bits.
  for (i = 0; i < 8; ++i) {
    int oe = ((b->trimask >> (7 - i))&1);
    set_bit(b->fd, BIT_DATA, oe);
    set_bit(b->fd, BIT_STROBE, 1);
    set_bit(b->fd, BIT_STROBE, 0);
  }

  // Write out the data bits.
  for (i = 0; i < BUSYBOARD_N_PORTS; ++i) {
    for (j = 0; j < 8; ++j) {
      int bit = ((b->out_state[BUSYBOARD_N_PORTS - 1 - i] >> (7 - j))&1);
      set_bit(b->fd, BIT_DATA, bit);
      set_bit(b->fd, BIT_STROBE, 1);
      set_bit(b->fd, BIT_STROBE, 0);
    }
  }

  // Strobe out all the newly-written bits
  set_bit(b->fd, BIT_LATCH_OUT, 1);
  set_bit(b->fd, BIT_LATCH_OUT, 0);
}

void busyboard_in(struct busyboard *b) {
  int i, j;

  // Strobe in all of the inputs into the input shift register.
  set_bit(b->fd, BIT_LATCH_IN, 1);
  set_bit(b->fd, BIT_LATCH_IN, 0);

  // Transfer all of the strobed-in inputs to the shift register
  set_bit(b->fd, BIT_N_LD_IN, 0);
  set_bit(b->fd, BIT_N_LD_IN, 1);

  // Read in the data bits.
  for (i = 0; i < BUSYBOARD_N_PORTS; ++i) {
    b->in_state[BUSYBOARD_N_PORTS - i - 1] = 0;
    for (j = 0; j < 8; ++j) {
      int bit = read_data(b->fd);
      // set_bit(b->fd, BIT_DATA, bit);
      b->in_state[BUSYBOARD_N_PORTS - i - 1] |= bit << (7 - j);

      set_bit(b->fd, BIT_STROBE, 1);
      set_bit(b->fd, BIT_STROBE, 0);
    }
  }

  /* This has ruined our output state, so refresh it. */
  busyboard_out(b);
}

void set_bit(int fd, int bit, int val) {
  /* printf("Set bit %s to %d\n", ppbit_name[bit], val); */

  if (bit != BIT_DATA) {
    struct ppdev_frob_struct f;
    f.mask = bit_pp[bit];
    f.val = (inverted[bit] ^ val) ? bit_pp[bit] : 0;
    ioctl(fd, PPFCONTROL, &f);
  } else {
    int x = val ? 1 : 0;
    ioctl(fd, PPWDATA, &x);
  }

  #ifdef DELAY
  usleep(DELAY);
  #endif
}

int read_data(int fd) {
  /* Read data from PARPORT_STATUS_ACK; invert */
  unsigned char x;
  ioctl(fd, PPRSTATUS, &x);
  
  return (x & 0x40) ? 1 : 0;
}
