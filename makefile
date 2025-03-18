CC = cc
CFLAGS += -std=c11 -Wall -Wextra -g
CPPFLAGS += -Iinclude

MARU_SOURCES := $(shell find maru -name '*.c')
MARU_HEADERS := $(shell find include -name '*.h')

bin/maru: bin $(MARU_SOURCES) $(MARU_HEADERS)
	$(CC) -o bin/maru $(CFLAGS) $(CPPFLAGS) $(MARU_SOURCES)

bin:
	mkdir -p bin

bin/driver: examples/driver.c
	$(CC) -o bin/driver $(CFLAGS) $(CPPFLAGS) examples/driver.c

test: bin/driver bin/maru
	bin/driver

.PHONY: clean
clean:
	rm -rf ./bin examples/**/*.s
