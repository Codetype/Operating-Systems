all: compile program clean

compile:
	gcc -c main.c mainchild.c
	gcc main.o -o main
	gcc mainchild.o -o mainchild

program:
	./main

program2:
	./mainchild './timer.sh'

clean:
	rm -f *.o
	rm -f main mainchild
