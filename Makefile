CC=g++
CFLAGS=-c -Wall
LDFLAGS= -l pthread
SOURCES= ./src/arphandler.cpp ./src/usockets.cpp ./src/tcphandshake.cpp ./src/ubinding.cpp ./src/ucommon.cpp ./src/uecho.cpp ./src/uicmp.cpp ./src/main.cpp
OBJECTS=$(SOURCES:./src/%.cpp=./build/%.o)
EXECUTABLE=utcpEcho

all: CREATEDIR $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $(patsubst src/%.cpp, build/%.o, $<)

CREATEDIR:
	mkdir -p build

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
