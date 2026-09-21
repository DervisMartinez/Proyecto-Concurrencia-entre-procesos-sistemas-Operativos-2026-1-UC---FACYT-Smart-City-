# ============================================================================
# Makefile - Compilación del proyecto Smart City
# ============================================================================
# Uso:
#   make          → Compila el proyecto
#   make clean    → Elimina archivos compilados
# ============================================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -pthread -O2
LDFLAGS = -pthread

TARGET  = smart_city
SRCS    = main.c intersection.c vehicles.c orchestrator.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
