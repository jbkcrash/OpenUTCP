CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.cpp arphandler.cpp sockets.cpp tcphandshake.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=myprogram

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
    $(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
    $(CC) $(CFLAGS) $< -o $@

clean:
    rm -f $(OBJECTS) $(EXECUTABLE)
