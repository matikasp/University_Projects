CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
LDFLAGS = -lm

SERVER_TARGET = approx-server
CLIENT_TARGET = approx-client

SERVER_SOURCES = server.cpp common.cpp
CLIENT_SOURCES = client.cpp common.cpp

SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

.PHONY: all clean

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(SERVER_OBJECTS) -o $@ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $(CLIENT_OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_TARGET) $(CLIENT_TARGET)

server.o: server.cpp common.h
client.o: client.cpp common.h
common.o: common.cpp common.h
