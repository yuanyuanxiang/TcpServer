make:server.cpp client.cpp
	g++ -g -c server.cpp -o server.o
	g++ server.o -o server
	g++ -g -c client.cpp -o client.o
	g++ client.o -o client

server:server.cpp
	g++ -g -c server.cpp -o server.o
	g++ server.o -o server

client:client.cpp
	g++ -g -c client.cpp -o client.o
	g++ client.o -o client

clean:
	rm -rf *.o
	rm -rf server
	rm -rf client

