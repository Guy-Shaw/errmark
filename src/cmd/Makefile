
SOURCES := $(wildcard *.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
PROGRAMS := $(patsubst %.c,%,$(SOURCES))
DISTBIN := /usr/local/bin

CC := gcc
CPPFLAGS := -I../inc
CFLAGS := -Wall -Wextra -g

LIBERRMARK := ../liberrmark/liberrmark.a
LIBCSCRIPT := ../libcscript/libcscript.a

.PHONY: all cscope clean install show-targets

all: $(PROGRAMS)

$(PROGRAMS): $(OBJECTS) $(LIBERRMARK) $(LIBCSCRIPT)

$(LIBERRMARK):
	cd ../liberrmark && make liberrmark.a

$(LIBCSCRIPT):
	cd ../libcscript && make libcscript.a

valgrind:
	valgrind --leak-check=full ./errmark fuser .

cscope:
	cscope-init

clean:
	cd ../liberrmark && make clean
	rm -f $(PROGRAMS) *.o

install: all
	cp $(PROGRAMS) $(DISTBIN)
	cd $(DISTBIN) && chmod 755 $(PROGRAMS)

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
