CC=gcc
CFLAGS=-g -O2 -std=c11 -pipe -Wall -Wextra -pedantic
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
MEMCHECKCMD=valgrind --tool=memcheck
INSTALLDIR=$(HOME)/.config/rofi

all: $(OBJECTS)

test: clean memcheck

compile: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o

memcheck: compile
	@$(foreach obj,$(OBJECTS),$(MEMCHECKCMD) ./$(obj))

install: compile
	-mkdir -p $(INSTALLDIR)
	-cp $(OBJECTS) $(INSTALLDIR)

uninstall:
	@$(foreach obj,$(OBJECTS),rm -f $(INSTALLDIR)/$(obj);)
