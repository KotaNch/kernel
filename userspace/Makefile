CC     := gcc
CFLAGS := -Wall -Wextra -O2

all: tool

tool: tool.c
	$(CC) $(CFLAGS) -o tool tool.c

clean:
	rm -f tool

.PHONY: all clean