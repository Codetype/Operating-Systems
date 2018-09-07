MY_LIB = lib
all: static shared dynamic test clean_all

static:
	gcc -Wall -c $(MY_LIB).c -o $(MY_LIB).o
	ar rcs lib$(MY_LIB).a $(MY_LIB).o
	gcc -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way

shared:
	gcc -Wall -c -fPIC $(MY_LIB).c -o $(MY_LIB).o
	gcc -shared $(MY_LIB).o -o lib$(MY_LIB).so
	gcc main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way

dynamic:
	gcc -Wall -c -fPIC $(MY_LIB).c -o $(MY_LIB).o
	gcc -shared $(MY_LIB).o -o lib$(MY_LIB).so
	gcc -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way -ldl -D DLL

test:
	./main_static_way 1000 10 1 -crt -del
	./main_dynamic_way 1000 10 1 -crt -del
	./main_shared_way 1000 10 1 -crt -del
	./main_static_way 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way 1000 10 1 -crt -fndblk 1000 -del

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out

clean_all: clean
	rm -f main_static_way
	rm -f main_dynamic_way
	rm -f main_shared_way
	rm -f main
