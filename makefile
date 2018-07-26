CC=g++
WARNS=-Wall -ansi -pedantic -std=c89
ENTRY=main.cpp
OSX_RELEASE=$(CC) $(ENTRY) -O3 -framework SDL2
OSX_DEV=$(CC) $(ENTRY) -O0 -framework SDL2

dev:
	$(OSX_DEV) -o prog && ./prog

release:
	$(OSX_RELEASE) -o prog && ./prog

debug:
	$(OSX_DEV) -g -o prog && lldb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	