/* LCD test-- drive parallel LCD module:
 * A0: RS - Register/data select
 * A1: R/#W - Read/#write
 * A2: E -- Operation strobe, falling edge triggered
 * B0-B7: Parallel data
 */

#include "busyboard.h"

#define DELAY 10000

void delay() {
  usleep(DELAY);
}

void lcd_write_command(struct busyboard *bb, unsigned char cmd) {
  bb->out_state[0] = 4;
  bb->out_state[1] = cmd;
  busyboard_out(bb);
  delay();

  bb->out_state[0] = 0;
  busyboard_out(bb);
  delay();

  bb->out_state[0] = 4;
  busyboard_out(bb);
  delay();
}

void lcd_write_data(struct busyboard *bb, unsigned char data) {
  bb->out_state[0] = 5; /* E high, write mode, data mode */
  bb->out_state[1] = data;
  busyboard_out(bb);
  delay();

  bb->out_state[0] = 1;
  busyboard_out(bb);
  delay();

  bb->out_state[0] = 5;
  busyboard_out(bb);
}

void lcd_init(struct busyboard *bb) {
  bb->trimask = 0x3f;
  bb->out_state[0] = 4; /* E high, write mode, command mode */
  busyboard_out(bb);

  lcd_write_command(bb, 0x30); /* Wake up!!! */
  lcd_write_command(bb, 0x30);
  lcd_write_command(bb, 0x30);

  lcd_write_command(bb, 0x38); /* Recommended start-up sequence */
  lcd_write_command(bb, 0x10);
  lcd_write_command(bb, 0x0c);
  lcd_write_command(bb, 0x06);

  lcd_write_command(bb, 0x02); /* Clear display */
}

void lcd_set_pos(struct busyboard *bb, int row, int col) {
  int addr = (row * 40 + col)&0x3f;
  lcd_write_command(bb, 0x80 | addr);
}

void lcd_write_str(struct busyboard *bb, const char *s) {
  lcd_set_pos(bb, 0, 0);
  for (unsigned i = 0; i < 128 && *s; i++, s++) {
    if (i == 16) lcd_set_pos(bb, 1, 0);
    lcd_write_data(bb, *s);
  }
}

void lcd_clear_screen(struct busyboard *bb) {
  lcd_write_str(bb, "                                ");
}

int main(int argc, char **argv) {
  struct busyboard bb;
  init_busyboard(&bb, (argc >= 2) ? argv[1] : "/dev/parport0");  

  lcd_init(&bb);

  lcd_clear_screen(&bb);
  /*                  012345678901234*0123456789012345 */
  lcd_write_str(&bb, "Busyboard Ver. 0CHDL & LibPCB");
  
  close_busyboard(&bb);

  return 0;
}
