run: build
	./server.out

build: lib.o server.out client.out

client: client.out
	./client.out

client.out: client.o lib.o
	g++ client.o lib.o -o client.out

client.o: client.cpp
	g++ client.cpp -c -o client.o


server: server.out
	./server.out

server.out: server.o lib.o
	g++ server.o lib.o -o server.out

server.o: server.cpp
	g++ server.cpp -c -o server.o


lib.o: lib.cpp
	g++ lib.cpp -c lib.o


test:
	g++ test_server.cpp -o test_server.out
	g++ test_client.cpp -o test_client.out