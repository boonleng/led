CC       = gcc

CFLAGS   += -std=gnu99 -Wall -Wno-unknown-pragmas -Os -I /usr/local/include -I ./ps
LDFLAGS  += -L /usr/local/lib -lpigpio -L ./ps -lps -lpthread -lrt -lm


all: led

led: led.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)

clean:
	rm -f $(PROGS)

