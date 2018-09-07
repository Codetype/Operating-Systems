all: compile program clean

compile:
	gcc -c main.c test.c
	gcc main.o -o main
	gcc test.o -o test1

program:
	./main 'test.txt'

clean:
	rm -f *.o
	rm -f *.out
	rm -f main test1
