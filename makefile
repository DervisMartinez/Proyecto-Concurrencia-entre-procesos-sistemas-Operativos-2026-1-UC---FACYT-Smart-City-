CC=gcc
CFLAGS=-I.
DEPS = intersection.h

OBJFILES = main.o intersection.o car.o orchestrator.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

RUN: $(OBJFILES)
	$(CC) -o $@ $^ $(CFLAGS)