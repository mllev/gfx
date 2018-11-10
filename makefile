WARNS=-Wall -ansi -pedantic -std=c89
LEVEL=-O3
FRAMEWORK=-F/Library/Frameworks -framework

obj:
	g++ tests/obj.cpp $(LEVEL) $(FRAMEWORK) SDL2 -o prog && ./prog

boxes:
	gcc tests/boxes.c $(LEVEL) $(WARNS) $(FRAMEWORK) SDL2 -o prog && ./prog

game:
	gcc tests/game.c $(LEVEL) $(WARNS) $(FRAMEWORK) SDL2 -o prog && ./prog

rockets:
	gcc tests/rockets.c $(LEVEL) $(WARNS) $(FRAMEWORK) SDL2 -o prog && ./prog

debug:
	gcc tests/boxes.c -O0 $(WARNS) $(FRAMEWORK) SDL2 -g -o prog && lldb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	