# ISO C23 (GCC 12: -std=c2x).  make && make test
CC ?= gcc
CSTD ?= c2x
CFLAGS ?= -std=$(CSTD) -pedantic -Wall -Wextra -Werror -O2 -Irdc
LDFLAGS ?=

LIB = rdc/rdc_engine.c rdc/rdc_history.c rdc/rdc_store.c
CLI = cli/rdc_cli.c
TEST = test/rdc_test.c

.PHONY: all test clean

all: dropcounter rdc_test

dropcounter: $(LIB) $(CLI)
	$(CC) $(CFLAGS) -o $@ $(LIB) $(CLI) $(LDFLAGS)

rdc_test: $(LIB) $(TEST)
	$(CC) $(CFLAGS) -o $@ $(LIB) $(TEST) $(LDFLAGS)

test: rdc_test
	./rdc_test

clean:
	rm -f dropcounter rdc_test
