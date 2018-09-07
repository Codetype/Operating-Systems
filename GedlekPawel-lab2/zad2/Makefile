all: compile testMe clean

compile:
	gcc -c -Wall main.c
	gcc main.o -o main

testMe:
	./main '../GedlekPawel-cw02' '>' '2018-03-01 20:00'
	./main '../GedlekPawel-cw02' '>' '2018-03-01 20:00' nftw

clean:
	 rm -f *.o
	 rm -f main
