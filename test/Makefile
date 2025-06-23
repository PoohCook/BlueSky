$(CC) ?= gcc
$(AR) ?= ar
$(MAKE) ?= make
CFLAGS += -fPIC -Werror -Wall -pedantic -std=gnu11 -iquote ../core/inc -iquote ./eeyore/inc -DUSE_TEST_DELAY
LFLAGS += -Werror -Wall -pthread -lm

DEPS = *.h
OBJ = eeyore/src/Eeyore.o eeyore/src/Events.o eeyore/src/Logger.o eeyore/src/Semaphores.o eeyore/src/Threads.o eeyore/src/Alloc.o \
	SpinupTests.o ../core/src/sky.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: TestMain.o $(OBJ)
	$(CC) -o Test.out $^ $(CFLAGS) $(LFLAGS)
	./Test.out

clean:
	rm -rf tmp
	rm -rf tmp_file
	rm -rf gpio_tmp
	rm -f *.o *.out
	rm -f *.a
