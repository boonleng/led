KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell cat .git/HEAD | cut -d'/' -f3)

CC = gcc
CFLAGS += -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -D_GNU_SOURCE
ifeq ($(GIT_BRANCH), debug)
	CFLAGS += -ggdb -DDEBUG
endif
ifneq ($(GIT_BRANCH), master)
	CFLAGS += -DBETA
endif
CFLAGS += -DBETA
CFLAGS += -I /usr/local/include -I ./ps -I ./
LDFLAGS += -L /usr/local/lib -lpigpio -L ./ps -lps -lpthread -lrt -lm

OBJS = color.o
OBJS_PATH = objects
OBJS_WITH_PATH = $(addprefix $(OBJS_PATH)/, $(OBJS))

PROGS = led

all: showinfo $(PROGS)

showinfo:
	@echo $(ECHO_FLAG) "KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[m"
	@echo $(ECHO_FLAG) "KERNEL = \033[38;5;15m$(KERNEL)\033[m"
	@echo $(ECHO_FLAG) "MACHINE = \033[38;5;220m$(MACHINE)\033[m"
	@echo $(ECHO_FLAG) "GIT_BRANCH = \033[38;5;46m$(GIT_BRANCH)\033[m"

$(OBJS): %.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

led: %: %.c $(OBJS)
	@echo $(ECHO_FLAG) "\033[38;5;203m$@\033[0m"
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJS) $(LDFLAGS)

clean:
	rm -f *.o *.s
	rm -f $(OBJS)
	rm -f $(PROGS)
	rm -rf *.dSYM
