OSC_SRCS = $(wildcard src/*.c)

all: tests

clean:
	rm -rf build

build:
	mkdir -p $@

build/osc_tests: tests/tests.c $(OSC_SRCS) | build
	g++ -g -Wall -Wextra -pthread -Isrc $^ -lpthread -lgtest -lgtest_main -o $@

tests: build/osc_tests
	./build/osc_tests

.PHONY: clean tests
