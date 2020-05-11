

Compalition:
gcc server.c -o server
gcc client.c -o client

Run:
./client [-n number] [-t timeout] host_1:port_1 host_2:port_2 ...
./server listen_port

Things done:

	Client:
	* Client can connect to multiple servers (though not simultaneously)
	* Client can detect timeouts while awaiting responses (but not during connect)
	* Client can supports all flags
	* Client can convert host names to IP

	Server:
	* Server can listen and respond to multiple clients using multiplexing
