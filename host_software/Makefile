LDLIBS = -lm
APPS = scope pov_test spi_test spi_adc_test pwm_test mem_test z80_test \
       28c256_test lcd_test

all: $(APPS)

scope: scope.o busyboard.o
pov_test : pov_test.o busyboard.o
spi_test: spi_test.o busyboard.o
pwm_test: pwm_test.o busyboard.o
mem_test: mem_test.o busyboard.o
z80_test: z80_test.o busyboard.o
spi_adc_test: spi_adc_test.o busyboard.o
28c256_test: 28c256_test.o busyboard.o
lcd_test: lcd_test.o busyboard.o

busyboard.o: busyboard.c

clean:
	$(RM) $(APPS) *.o *~
