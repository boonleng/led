CC=gcc
CFLAGS = -std=gnu99 -Wall -Wno-unknown-pragmas -Os -I /usr/local/include
LDFLAGS = -L /usr/local/lib -lpigpio -lpthread -lrt -lm

PROGS = led

all: $(PROGS)

$(PROGS): %: %.c
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)

clean:
	rm -f $(PROGS)
