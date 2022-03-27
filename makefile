CC = g++
CFLAGS = -Wall

TARGET = server.out client.out

all: $(TARGET)

server.out: server.o jsoncpp.o
	$(CC) $(CFLAGS) -o server.out server.o jsoncpp.o

client.out: client.o jsoncpp.o
	$(CC) $(CFLAGS) -o client.out client.o jsoncpp.o

server.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

client.o: client.cpp
	$(CC) $(CFLAGS) -c client.cpp

jsoncpp.o: jsoncpp/*
	$(CC) $(CFLAGS) -c jsoncpp/dist/jsoncpp.cpp

clean:
	rm *.o *.out