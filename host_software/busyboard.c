/* Busyboard control program/library */

/*
    bvec<8> lptdata;
    in_data = lptdata[0];
    Centronics(clock, lptdata, latch_out, latch_in, nload_out,
	       out_data, Lit(0), Lit(0), Lit(0), Lit(0));

  node &nstrobe, bvec<8> &data, node &nlf, node &nreset, node &nsel_prn,
  node nack, node nbusy, node paperout, node sel, node err
*/

// PARPORT_CONTROL_STROBE AUTOFD INIT SELECT


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/ppdev.h>

#define N_PORTS 6

/* Busyboard control structure. */
struct busyboard {
  int fd; /* Parallel port file descriptor. */
  unsigned trimask; /* One bit per I/O byte tristate mask, 1=out 0=Hi-Z */
  unsigned char out_state[N_PORTS], in_state[N_PORTS];
};

void init_busyboard(struct busyboard *b, const char *devnode);
void busyboard_out(struct busyboard *b);
void busyboard_in(struct busyboard *b);

int open_parport(const char *devnode); /* Open and init parallel port. */
void close_parport(int fd);

enum ppbit { BIT_STROBE, BIT_DATA, BIT_LATCH_OUT, BIT_LATCH_IN };
const char *ppbit_name[] = {
  "BIT_STROBE", "BIT_DATA", "BIT_LATCH_OUT", "BIT_LATCH_IN"
};

void set_bit(int fd, int bit, int val);
int read_data(int fd);

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  int i;
  for (i = 0; i < N_PORTS; i++) bb.out_state[i] = ~i;
  busyboard_out(&bb);
  
  close_parport(bb.fd);

  return 0;
}

void init_busyboard(struct busyboard *b, const char *devnode) {
  int i;
  
  //b->fd = open_parport(devnode);
  b->trimask = 0;

  for (i = 0; i < N_PORTS; ++i) b->out_state[i] = 0;
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
  for (i = 0; i < N_PORTS; ++i) {
    for (j = 0; j < 8; ++j) {
      int bit = ((b->out_state[N_PORTS - 1 - i] >> (7 - j))&1);
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

  // Read in the data bits.
  for (i = 0; i < N_PORTS; ++i) {
    for (j = 0; j < 8; ++j) {
      int bit = read_data(b->fd);
      set_bit(b->fd, BIT_DATA, bit);
      b->in_state[i] |= bit << j;

      set_bit(b->fd, BIT_STROBE, 1);
      set_bit(b->fd, BIT_STROBE, 0);
    }
  }
}

void set_bit(int fd, int bit, int val) {
  printf("Set bit %s to %d\n", ppbit_name[bit], val);
}

int read_data(int fd) {
  return 0;
}
