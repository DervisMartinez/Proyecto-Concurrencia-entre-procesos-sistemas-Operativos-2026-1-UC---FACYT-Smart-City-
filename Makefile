CC = gcc
CFLAGS = -Wall -Wextra -pthread -O2
LDFLAGS = -pthread

TARGET = smart_city

OBJS = main.o intersection.o vehicles.o orchestrator.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
