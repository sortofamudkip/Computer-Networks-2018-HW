CLIENT_SOURCES := $(shell find src -type f -name "client*.cpp")
SERVER_SOURCES := $(shell find src -type f -name "server*.cpp")

PORT=19798
SERV_IP=127.0.0.1

all: $(CLIENT_SOURCES) $(SERVER_SOURCES)
	g++ -std=c++17 $(CLIENT_SOURCES) -Iinclude -o client
	g++ -std=c++17 $(SERVER_SOURCES) -Iinclude -o server -lpthread -Wno-write-strings

client: $(CLIENT_SOURCES)
	g++ -std=c++17 $(CLIENT_SOURCES) -Iinclude -o client

server: $(SERVER_SOURCES)
	g++ -std=c++17 $(SERVER_SOURCES) -Iinclude -o server -lpthread -Wno-write-strings

rc:
	./client $(SERV_IP) $(PORT)	 

rs:
	./server $(PORT)

clean: 
	rm -rf client server *_file_dir
