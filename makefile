CC = g++
CFLAGS = -Wall

TARGET = server.out client.out

all: $(TARGET)

server.out: server.cpp
	$(CC) $(CFLAGS) -o server.out server.cpp

client.out:
	$(CC) $(CFLAGS) -o client.out client.cpp

clean:
	rm $(TARGET)