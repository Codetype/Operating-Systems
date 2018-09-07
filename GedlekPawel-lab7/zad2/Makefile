compile:
	gcc barbershop.c barber.c -pthread -DSO -std=gnu99  -o barber -lrt
	gcc -pthread -DSO -std=gnu99 barbershop.c client.c -o client -lrt

clean:
	rm -f barber client
