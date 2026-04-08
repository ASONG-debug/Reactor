CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude -Isrc
LDFLAGS := -lpthread

SRC := src
SRV_OBJS := $(SRC)/main_echoserver.cpp $(SRC)/InetAddress.cpp $(SRC)/Socket.cpp $(SRC)/Epoll.cpp \
            $(SRC)/Channel.cpp $(SRC)/EventLoop.cpp $(SRC)/TcpServer.cpp $(SRC)/Acceptor.cpp \
            $(SRC)/Connection.cpp $(SRC)/Buffer.cpp $(SRC)/EchoServer.cpp $(SRC)/ThreadPool.cpp \
            $(SRC)/Timestamp.cpp $(SRC)/ServerStats.cpp $(SRC)/Config.cpp

all: client echoserver benchmark

client: $(SRC)/client.cpp
	$(CXX) $(CXXFLAGS) -o client $(SRC)/client.cpp

echoserver: $(SRV_OBJS)
	$(CXX) $(CXXFLAGS) -o echoserver $(SRV_OBJS) $(LDFLAGS)

benchmark: $(SRC)/benchmark.cpp
	$(CXX) $(CXXFLAGS) -o benchmark $(SRC)/benchmark.cpp

clean:
	rm -f client echoserver benchmark

.PHONY: all clean
