CC=gcc
CFLAGS=-g -std=c11 -Wall -Werror -D_GNU_SOURCE `pkg-config --cflags --libs libcurl sqlite3 uuid`
OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c))
MEMCHECKCMD=valgrind --tool=memcheck
INSTALLDIR=$(HOME)/Downloads/rofi

all: $(OBJECTS)

compile: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -r *.memcheck

memcheck: compile
	@$(foreach obj,$(OBJECTS),echo memcheck $(obj) && $(MEMCHECKCMD) ./$(obj) >/dev/null 2>$(obj).memcheck;)

install: compile
	-mkdir -p $(INSTALLDIR)
	-cp $(OBJECTS) $(INSTALLDIR)

uninstall:
	@$(foreach obj,$(OBJECTS),rm -f $(INSTALLDIR)/$(obj);)
