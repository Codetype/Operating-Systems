CC = gcc
CFLAGS = -Wall
all: compile

compile:
	$(CC) $(CFLAGS) main.c -o main -lm -lpthread

test:
	./main 2 "./balloons.ascii.pgm" "./filter.txt" "./out_e2.pgm"
	./main 4 "./balloons.ascii.pgm" "./filter.txt" "./out_e4.pgm"
	./main 8 "./balloons.ascii.pgm" "./filter.txt" "./out_e8.pgm"
	./main 2 "./balloons.ascii.pgm" "./filter2.txt" "./out_t2.pgm"
	./main 4 "./balloons.ascii.pgm" "./filter2.txt" "./out_t4.pgm"
	./main 8 "./balloons.ascii.pgm" "./filter2.txt" "./out_t8.pgm"

test2:
	./main 10 "./feep.ascii.pgm" "./filter.txt" "./out.pgm"

clean:
	rm -f *.o main
