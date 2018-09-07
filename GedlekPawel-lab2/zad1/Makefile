all: compile testMe clean

compile:
	gcc -c main.c libFOperations.c sysFOperations.c
	gcc main.o libFOperations.o sysFOperations.o -o main

testMe:
	./main generate test.txt 4000 4
	./main copy test.txt testLib.txt 4000 4
	./main sort testLib.txt 4000 4
	./main copy test.txt testLib.txt 4000 4 sys
	./main sort testLib.txt 4000 4 sys
	rm -f *.txt
	./main generate test.txt 4000 512
	./main copy test.txt testLib.txt 4000 512
	./main sort testLib.txt 4000 512
	./main copy test.txt testLib.txt 4000 512 sys
	./main sort testLib.txt 4000 512 sys
	rm -f *.txt
	./main generate test.txt 4000 4096
	./main copy test.txt testLib.txt 4000 4096
	./main sort testLib.txt 4000 4096
	./main copy test.txt testLib.txt 4000 4096 sys
	./main sort testLib.txt 4000 4096 sys
	rm -f *.txt
	./main generate test.txt 4000 8192
	./main copy test.txt testLib.txt 4000 8192
	./main sort testLib.txt 4000 8192
	./main copy test.txt testLib.txt 4000 8192 sys
	./main sort testLib.txt 4000 8192 sys

clean:
	rm -f *.o
	rm -f *.txt
	rm -f *.out
	rm -f main
