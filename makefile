WARNS=-Wall -ansi -pedantic -std=c89
LEVEL=-O0
MACOS=-rpath /Library/Frameworks -F/Library/Frameworks -framework SDL2
LINUX=-lSDL2 -lm
PLATFORM=$(MACOS)

obj:
	g++ tests/obj.cpp $(LEVEL) $(PLATFORM) -Wno-narrowing -o prog && ./prog

boxes:
	gcc tests/boxes.c $(LEVEL) $(WARNS) $(PLATFORM) -o prog && ./prog

boxes-undefined-check:
	gcc -O3 tests/boxes.c -fsanitize=undefined $(PLATFORM) -o prog && ./prog

game:
	gcc tests/game.c $(LEVEL) $(WARNS) $(PLATFORM) -o prog && ./prog

rockets:
	gcc tests/rockets.c -fsanitize=undefined $(LEVEL) $(WARNS) $(PLATFORM) -o prog && ./prog

debug:
	gcc tests/boxes.c -O0 $(WARNS) $(PLATFORM) -g -o prog && gdb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	
