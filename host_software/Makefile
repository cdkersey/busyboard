APPS = scope pov_test spi_test

all: $(APPS)

scope: scope.o busyboard.o
pov_test : pov_test.o busyboard.o
spi_test: spi_test.o busyboard.o

busyboard.o: busyboard.c

clean:
	$(RM) $(APPS) *.o *~
