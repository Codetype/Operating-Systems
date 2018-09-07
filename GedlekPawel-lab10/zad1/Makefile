compile:
	gcc -o server server.c -lpthread
	gcc -o client client.c -lpthread

clean:
	rm -f client server

unix_server:
	./server 8080 unix_server

unix_client:
	./client unix_client1 UNIX unix_server

web_server:
	./server 8080 web_server

web_client:
	./client web_client WEB 127.0.0.1 8080
