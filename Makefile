CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude
LIBS = -lpcap -lhiredis

SRC = \
	src/main.cpp \
	src/sip_server.cpp \
	src/udp_socket.cpp \
	src/thread.cpp \
	src/logger.cpp \
	src/sip_parser.cpp \
	src/call_session.cpp \
	src/call_session_manager.cpp \
	src/rtp_relay.cpp \
	src/sdp_parser.cpp \
	src/env_reader.cpp

OBJ = $(SRC:.cpp=.o)

TARGET = sip_proxy

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

# 🔥 FIXED RULE
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)