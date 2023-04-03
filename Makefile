CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=./src/main.cpp ./src/arphandler.cpp ./src/sockets.cpp ./src/tcphandshake.cpp
OBJECTS=$(SOURCES:%.cpp=./build/%.o)
EXECUTABLE=utcpEcho

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
    $(CC) $(CFLAGS) $< -o $@

clean:
    rm -f $(OBJECTS) $(EXECUTABLE)
