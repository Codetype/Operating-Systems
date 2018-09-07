MY_LIB = lib
OP_0 = 0
OP_1 = 1
OP_2 = 2
OP_3 = 3
OP_4 = s
all: loading 1st 2nd 3rd 4th 5th 6th test clean_all

loading:
	gcc -Wall -c $(MY_LIB).c -o $(MY_LIB).o
	ar rcs lib$(MY_LIB).a $(MY_LIB).o
	gcc -Wall -c -fPIC $(MY_LIB).c -o $(MY_LIB).o
	gcc -shared $(MY_LIB).o -o lib$(MY_LIB).so

1st:
	gcc -O -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way1
	gcc -O main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way1
	gcc -O -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way1 -ldl -D DLL

2nd:
	gcc -O$(OP_0) -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way2
	gcc -O$(OP_0) main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way2
	gcc -O$(OP_0) -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way2 -ldl -D D

3rd:
	gcc -O$(OP_1) -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way3
	gcc -O$(OP_1) main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way3
	gcc -O$(OP_1) -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way3 -ldl -D D

4th:
	gcc -O$(OP_2) -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way4
	gcc -O$(OP_2) main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way4
	gcc -O$(OP_2) -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way4 -ldl -D D

5th:
	gcc -O$(OP_3) -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way5
	gcc -O$(OP_3) main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way5
	gcc -O$(OP_3) -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way5 -ldl -D D

6th:
	gcc -O$(OP_4) -Wall -std=c11  main.c  lib$(MY_LIB).a -o main_static_way6
	gcc -O$(OP_4) main.c -Wl,-rpath=. -Wall -L . -l$(MY_LIB) -o main_shared_way6
	gcc -O$(OP_4) -Wall -Wl,-rpath=. -L. main.c -l$(MY_LIB) -o main_dynamic_way6 -ldl -D D

test:
	./main_static_way1 1000 10 1 -crt -del
	./main_dynamic_way1 1000 10 1 -crt -del
	./main_shared_way1 1000 10 1 -crt -del
	./main_static_way1 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way1 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way1 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way1 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way1 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way1 1000 10 1 -crt -fndblk 1000 -del
	./main_static_way2 1000 10 1 -crt -del
	./main_dynamic_way2 1000 10 1 -crt -del
	./main_shared_way2 1000 10 1 -crt -del
	./main_static_way2 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way2 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way2 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way2 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way2 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way2 1000 10 1 -crt -fndblk 1000 -del
	./main_static_way3 1000 10 1 -crt -del
	./main_dynamic_way3 1000 10 1 -crt -del
	./main_shared_way3 1000 10 1 -crt -del
	./main_static_way3 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way3 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way3 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way3 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way3 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way3 1000 10 1 -crt -fndblk 1000 -del
	./main_static_way4 1000 10 1 -crt -del
	./main_dynamic_way4 1000 10 1 -crt -del
	./main_shared_way4 1000 10 1 -crt -del
	./main_static_way4 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way4 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way4 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way4 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way4 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way4 1000 10 1 -crt -fndblk 1000 -del
	./main_static_way5 1000 10 1 -crt -del
	./main_dynamic_way5 1000 10 1 -crt -del
	./main_shared_way5 1000 10 1 -crt -del
	./main_static_way5 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way5 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way5 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way5 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way5 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way5 1000 10 1 -crt -fndblk 1000 -del
	./main_static_way6 1000 10 1 -crt -del
	./main_dynamic_way6 1000 10 1 -crt -del
	./main_shared_way6 1000 10 1 -crt -del
	./main_static_way6 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_dynamic_way6 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_shared_way6 1000 10 1 -crt -delblks 10 210 -addblks 10 210 -del
	./main_static_way6 1000 10 1 -crt -fndblk 1000 -del
	./main_dynamic_way6 1000 10 1 -crt -fndblk 1000 -del
	./main_shared_way6 1000 10 1 -crt -fndblk 1000 -del
clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.out

clean_all: clean
	rm -f main_static_way1
	rm -f main_static_way2
	rm -f main_static_way3
	rm -f main_static_way4
	rm -f main_static_way5
	rm -f main_static_way6
	rm -f main_dynamic_way1
	rm -f main_dynamic_way2
	rm -f main_dynamic_way3
	rm -f main_dynamic_way4
	rm -f main_dynamic_way5
	rm -f main_dynamic_way6
	rm -f main_shared_way1
	rm -f main_shared_way2
	rm -f main_shared_way3
	rm -f main_shared_way4
	rm -f main_shared_way5
	rm -f main_shared_way6
	rm -f main
