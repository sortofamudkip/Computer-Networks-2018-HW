#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/file.h>
#include <stdlib.h>

#define die(a) { perror(a); exit(1); }

int main(int argc, char const *argv[]) {
	// parse args
	if (argc != 2) puts("usage: ./server listen_port"), exit(0);
	int port = atoi(argv[1]);

	char hostname[128] = {};
	gethostname(hostname, sizeof(hostname));

	// 1: socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) die("sockfd");

	// 2: bind
	// fill in info about port
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) die("bind");
	// 3: listen
	if (listen(sockfd, 10)) die("listen");

	// 4: all of this, basically

	char client_ip_ports[1024][128] = {};
	fprintf(stderr, "Starting server on %s, port %d...\n", hostname, port);

	// this is for multiplexing
	fd_set master_set;
	FD_ZERO(&master_set);
	FD_SET(sockfd, &master_set);
	int maxfd = sockfd;
	// this is not important
	struct timeval timeout;
	timeout.tv_sec = 0; 
	timeout.tv_usec = 10;

	while (1) {
		fd_set read_set; read_set = master_set; // those who are actually pinging us rn
		int fd_ready = select(maxfd + 1, &read_set, NULL, NULL, &timeout); // number of ppl who are pinging
		// if we get a new connection	
		if(FD_ISSET(sockfd, &read_set)) { 
		    fd_ready--;
		    // set up connection; ignore the conn_fd < 0 part
		    struct sockaddr_in cliaddr; 
		    int clilen = sizeof(cliaddr);
		    int conn_fd = accept(sockfd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
		    if (conn_fd < 0) {
		        if (errno == EINTR || errno == EAGAIN) continue;  // try again
		        if (errno == ENFILE) {
		            (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
		            continue;
		        }
		        die("accept");
		    }
		    // new: adding info from client
		    char ip[INET_ADDRSTRLEN] = {}; 
		    inet_ntop(AF_INET, &(cliaddr.sin_addr), ip, INET_ADDRSTRLEN); 
		    sprintf(client_ip_ports[conn_fd], "%s:%d", ip, ntohs(cliaddr.sin_port));
		    // fprintf(stderr, "getting a new request (fd: %d, ip: %s, port: %d)\n", conn_fd, ip, ntohs(cliaddr.sin_port));
		    maxfd = (conn_fd > maxfd) ? conn_fd : maxfd;
		    // also turn on conn_fd's bit in the MASTER fd_set
		    FD_SET(conn_fd, &master_set);            
		}
		for (int conn_fd = sockfd+1; conn_fd <= maxfd && fd_ready > 0; ++conn_fd) {
			if (FD_ISSET(conn_fd, &read_set)) { //if there is something to read
				// read from them
				char buf[256] = {};
				int msglen = recv(conn_fd, buf, 256, 0);
				if (msglen < 0) {printf("bad read from fd %d\n", conn_fd); continue;}
				if (msglen == 0) {
					// printf("connection from fd %d closed\n", conn_fd);
					FD_CLR(conn_fd, &master_set);
					client_ip_ports[conn_fd][0] = '\0';
					close(conn_fd);
					continue;
				}
				else {
					// read message and then write back
					// printf("read from %d: |%s|\n", conn_fd, buf);
					printf("recv from %s\n", client_ip_ports[conn_fd]);
					char return_msg[256] = {};
					sprintf(return_msg, "bitch");  // this should be like something else
					// usleep(1000*200);
					// sleep(2);
					send(conn_fd, return_msg, strlen(return_msg), 0);
				}

			}
		}

	}
	return 0;
}