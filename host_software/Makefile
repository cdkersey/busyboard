APPS = scope pov_test

all: $(APPS)

scope: scope.c busyboard.o
pov_test : pov_test.c busyboard.o

busyboard.o: busyboard.c

clean:
	$(RM) $(APPS) busyboard.o *~
