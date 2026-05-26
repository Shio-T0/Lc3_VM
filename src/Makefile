.PHONY: all
all: lc3vm

lc3vm: main.cpp
	g++ -O2 -Wall -Wextra -o lc3vm main.cpp

lc3vm-debug: main.cpp
	g++ -g -D DEBUG -Wall -Wextra -o lc3vm-debug main.cpp

release: lc3vm
debug: lc3vm-debug

clean:
	rm -f lc3vm lc3vm-debug
