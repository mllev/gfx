CC=cc
WARNS=-Wall -ansi -pedantic -std=c89
ENTRY=main.c
OSX_RELEASE=$(CC) $(ENTRY) -O3 -framework SDL2 $(WARNS)
OSX_DEV=$(CC) $(ENTRY) -O0 -framework SDL2 $(WARNS)

dev:
	$(OSX_DEV) -o prog && ./prog

release:
	$(OSX_RELEASE) -o prog && ./prog

debug:
	$(OSX_DEV) -g -o prog && lldb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	