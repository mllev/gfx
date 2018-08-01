WARNS=-Wall -ansi -pedantic -std=c89

obj:
	g++ tests/obj.cpp -O3 -framework SDL2 -o prog && ./prog

boxes:
	gcc tests/boxes.c -O3 $(WARNS) -framework SDL2 -o prog && ./prog

debug:
	gcc tests/boxes.c -O0 $(WARNS) -framework SDL2 -g -o prog && lldb prog

clean:
	rm -f prog && rm -f -rf prog.dSYM
	