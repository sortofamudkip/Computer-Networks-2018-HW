#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/file.h>
#include <stdlib.h>
#include <poll.h>


char has_n = 0, has_timeout = 0;
int n_flag = 0, timeout_flag = 1000;
char servers[1024][256], IPs[1024][256], ports[1024][256]; int server_count = 0;


void print_flags() {
	printf("flag results:\n");
	if (has_n) printf("-n: %d\n", n_flag);
	else puts("no -n");
	if (has_timeout) printf("-t: %d\n", timeout_flag);
	else puts("no -t");
	printf("servers: (%d)\n", server_count);
	for (int i = 0; i < server_count; ++i) {
		// printf("%s ", servers[i]);
		printf("IP: %s, port: %s\n", IPs[i], ports[i]);
	}
	puts("");
}

void parse_ports() {
	for (int i = 0; i < server_count; ++i) {
		int len = strlen(servers[i]) - 1;
		while (len--) {
			if (servers[i][len] == ':') {
				strcpy(ports[i], &servers[i][len+1]);
				servers[i][len] = '\0';
				strcpy(IPs[i], servers[i]);
				break;
			}
		}
	}
}

void send_msg(int sockfd, int i) { //which server you're pinging
	char msg[128] = {};
	sprintf(msg, "hi bitch");
	send(sockfd, msg, strlen(msg), 0);
	// RTT: start time. start timing.
	struct timeval start, end;
	gettimeofday(&start, NULL);
	// wait for response or timeout: use poll
	struct pollfd P = {sockfd, POLLIN, 0};
	if (poll(&P, 1, timeout_flag) == 0) printf("timeout when connect to %s\n", IPs[i]);
	else {
		int n; char buf[128] = {};
		read(sockfd, buf, 128);
		// RTT: end time.
		gettimeofday(&end, NULL);
		long elapsed = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000;
		// printf("recv from %s: |%s|, RTT = %ld msec\n", IPs[i], buf, elapsed);
		printf("recv from %s, RTT = %ld msec\n", IPs[i], elapsed);
	}
	usleep(1000*100);

}

#define die(a) { perror(a); exit(1); }

int main(int argc, char const *argv[]) {
	// parse args
	// printf("usage: ./client [-n number] [-t timeout] host_1:port_1 host_2:port_2 ...\n");
	if (argc == 1) exit(0);

	for (int i = 1; i < argc; ++i) {
		if (!strcmp("-n", argv[i])) { // has -n flag
			i++, has_n = 1, n_flag = atoi(argv[i]);
		}
		else if (!strcmp("-t", argv[i])) {
			i++, has_timeout = 1, timeout_flag = atoi(argv[i]);
		}
		else strcpy(servers[server_count++], argv[i]);
	}
	// debuging
	parse_ports();
	// print_flags();

	// 1) set up socket via connect	2) send info 3) recieve info
	// 1) socket

	for (int i = 0; i < server_count; ++i) {
		struct hostent *he;
		struct in_addr **addr_list;
		he = gethostbyname(IPs[i]);
		if (he != NULL) {
			addr_list = (struct in_addr **) he->h_addr_list;
			strcpy(IPs[i], inet_ntoa(**addr_list));
		}
		// if (he) printf("IP changed to %s\n", inet_ntoa(**addr_list));
		// else printf("IP not changed (still) %s\n", IPs[i]);

		int sockfd = socket(AF_INET, SOCK_STREAM, 0);

		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
		hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
		hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
		hints.ai_flags = 0;     // fill in my IP for me
		struct addrinfo *servinfo;  // will point to the results
		// printf("trying to connect to %s:%s\n", IPs[i], ports[i]);
		if (getaddrinfo(IPs[i], ports[i], &hints, &servinfo)) die("getaddrinfo");

		if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
			if (errno == ECONNREFUSED) printf("timeout when connect to %s\n", IPs[i]);
			else die("connect");
			continue;
		}
		// printf("connection successful!\n");
		// TODO:
		// multiplex multiple servers at the same time 
		// (i.e. send them all a request and multiplex their requests)
		if (n_flag) {
			for (int j = 0; j < n_flag; ++j) { // for n times
				send_msg(sockfd, i);
			} // end of "send n packets"
		}
		else {
			while (1) send_msg(sockfd, i);
		}
		close(sockfd);
	} // end of server loop

	return 0;
}