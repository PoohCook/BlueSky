$(CC) ?= gcc
$(AR) ?= ar
$(MAKE) ?= make
CFLAGS += -fPIC -Werror -Wall -pedantic -std=gnu11 -iquote ./core/inc  -DUSE_TEST_DELAY
LFLAGS += -Werror -Wall -pthread -lm

.PHONY: run test clean

DEPS = *.h
OBJ = ./core/src/sky.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: ./core/src/main.o $(OBJ)
	$(CC) -o main.out $^ $(CFLAGS) $(LFLAGS)
	./main.out

test:
	$(MAKE) -C test test

clean:
	rm -f *.o *.so
	rm -f libs/*
	$(MAKE) -C test clean
