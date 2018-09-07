all: compile program clean

compile:
	gcc -c -Wall forkdir.c
	gcc forkdir.o -o main

program:
	./main '../GedlekPawel-cw01' '>' '2018-03-01 20:00:00'

testMe:
	./main '../GedlekPawel-cw01' '>' '2018-03-01 20:00:00'
	./main '../GedlekPawel-cw02' '>' '2018-03-01 20:00:00'
	./main '../GedlekPawel-cw03' '>' '2018-03-01 20:00:00'

clean:
	rm -f *.o
	rm -f *.txt
	rm -f *.out
	rm -f main
