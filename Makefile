$(CC) ?= gcc
$(AR) ?= ar
$(MAKE) ?= make
CFLAGS += -fPIC -Werror -Wall -pedantic -std=gnu11 -iquote ./core/inc  -DUSE_TEST_DELAY
LFLAGS += -Werror -Wall -pthread -lm

.PHONY: run test clean

DEPS = *.h
OBJ = ./core/src/sky.o ./core/src/reverse.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

run: ./core/src/spec_test.o $(OBJ)
	$(CC) -o spec_test.out $^ $(CFLAGS) $(LFLAGS)
	./spec_test.out

spinup: ./core/src/main.o $(OBJ)
	$(CC) -o main.out $^ $(CFLAGS) $(LFLAGS)
	./main.out


test:
	$(MAKE) -C test test

clean:
	rm -f *.o *.so
	rm -f ./core/src/*.o
	$(MAKE) -C test clean

package: clean
	tar -czf ../BlueSky.tar.gz ./
	mv ../BlueSky.tar.gz ./BlueSky.tar.gz
	zip -r ../BlueSky.zip ./
	mv ../BlueSky.zip ./BlueSky.zip
