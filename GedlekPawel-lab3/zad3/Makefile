all: compile testMe clean

compile:
	gcc -c main.c testMem.c testT.c
	gcc main.o -o main
	gcc testMem.o -o testMem
	gcc testT.o -o testT

testMe:
	./main 'test.txt' 10 100
	./main 'test.txt' 2 1000


clean:
	rm -f *.o
	rm -f *.out
	rm -f main testMem testT
