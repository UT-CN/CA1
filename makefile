CC = g++
CFLAGS = -Wall

TARGET = server.out client.out

all: $(TARGET)

server.out: server.o jsoncpp.o
	$(CC) $(CFLAGS) -o server.out server.o jsoncpp.o

client.out: client.cpp
	$(CC) $(CFLAGS) -o client.out client.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

jsoncpp.o: jsoncpp
	$(CC) $(CFLAGS) -c jsoncpp/dist/jsoncpp.cpp

clean:
	rm *.o *.out