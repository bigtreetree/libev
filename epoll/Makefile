all:server client

server:server.o
	g++ -g -o $@ $^
server.o:server.cpp
	g++ -g -c $^

client:client.o
	g++ -g -o $@ $^
client.o:client.cpp
	g++ -g -c $^

clean:
	rm -rf *.o server client
