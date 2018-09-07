CC = gcc
CFLAGS = -c -Wall
MY_LIB = lib
all: staticLib sharedLib clean

staticLib:
	$(CC) $(CFLAGS) $(MY_LIB).c -o $(MY_LIB).o

	ar rcs $(MY_LIB).a -o $(MY_LIB).o

sharedLib:
	$(CC) -fPIC $(CFLAGS) $(MY_LIB).c -o $(MY_LIB).o

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out
