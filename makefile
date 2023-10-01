WARNS=-Wall -ansi -pedantic -std=c89
LEVEL=-O0
MACOS=-F/Library/Frameworks -framework SDL2
LINUX=-lSDL2 -lm

obj:
	g++ tests/obj.cpp $(LEVEL) $(LINUX) -Wno-narrowing -o prog && ./prog

boxes:
	gcc tests/boxes.c $(LEVEL) $(WARNS) $(LINUX) -o prog && ./prog

boxes-undefined-check:
	gcc -O3 tests/boxes.c -fsanitize=undefined $(LINUX) -o prog && ./prog

game:
	gcc tests/game.c $(LEVEL) $(WARNS) $(LINUX) -o prog && ./prog

rockets:
	gcc tests/rockets.c $(LEVEL) $(WARNS) $(LINUX) -o prog && ./prog

debug:
	gcc tests/boxes.c -O0 $(WARNS) $(LINUX) -g -o prog && gdb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	
