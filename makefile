CC = g++
CFLAGS = -Wall

TARGET = server.out client.out

all: $(TARGET)

server.out: server.o jsoncpp.o
	$(CC) $(CFLAGS) -o server.out server.o jsoncpp.o

client.out: client.o jsoncpp.o
	$(CC) $(CFLAGS) -o client.out client.o jsoncpp.o

server.o: server.cpp server.hpp
	$(CC) $(CFLAGS) -c server.cpp

client.o: client.cpp client.hpp
	$(CC) $(CFLAGS) -c client.cpp

jsoncpp.o: jsoncpp/*
	$(CC) $(CFLAGS) -c jsoncpp/dist/jsoncpp.cpp

clean:
	rm *.o *.out