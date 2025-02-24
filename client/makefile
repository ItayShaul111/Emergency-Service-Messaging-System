CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

# Target for the main executable
all: StompEMIClient

# Build the executable
StompEMIClient: bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/StompProtocol.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/StompProtocol.o $(LDFLAGS)

# Compile the ConnectionHandler.cpp file
bin/ConnectionHandler.o: src/ConnectionHandler.cpp include/ConnectionHandler.h
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

# Compile the StompClient.cpp file
bin/StompClient.o: src/StompClient.cpp include/StompProtocol.h include/ConnectionHandler.h
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

# Compile the event.cpp file
bin/event.o: src/event.cpp include/event.h
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

# Compile the StompProtocol.cpp file
bin/StompProtocol.o: src/StompProtocol.cpp include/StompProtocol.h include/ConnectionHandler.h include/event.h
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

# Clean up the generated files
.PHONY: clean
clean:
	rm -f bin/*