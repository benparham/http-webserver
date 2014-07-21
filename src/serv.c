#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <request_parser.h>

#define BACKLOG 				10

#define HOST_LOOKUP_CMD 		"ifconfig | grep -P 'inet (?!127.0.0.1)'"
#define MAX_HOST_LEN			128

#define RECV_BUF_SIZE			128

struct thread_args {
	int socket_fd;
};

void* listen_to_client(void *temp_args) {

	struct thread_args *args = (struct thread_args *) temp_args;

	// Create request parser
	int error;
	rp_parser *parser;
	if ((error = rp_parser_create(&parser))) {
		printf("Failed to create parser: %s\n", rp_strerr(error));
	}

	while (1) {

		rp_parser_reset(parser);

		int completed = 0;
		int bytes_left = 0;
		while (!completed) {

			if (bytes_left >= RECV_BUF_SIZE) {
				printf("Unable to parse request. Single line of request longer than buffer\n");
				goto exit;
			}

			int bytes_to_rec = RECV_BUF_SIZE - bytes_left;
			char buf[bytes_to_rec];

			int bytes_received = recv(args->socket_fd, buf + bytes_left, bytes_to_rec, 0);
			
			if (bytes_received == 0) {
				printf("Client has closed connection\n");
				goto exit;
			}

			if (bytes_received == -1) {
				printf("Recv error\n");
				goto exit;
			}

			printf("Received message (%d):\n", bytes_received);
			printf("%s\n", buf);

			printf("Parsing request...\n");
			if ((error = rp_parse(parser, buf, &bytes_left, &completed))) {
				printf("Failed to parse request: %s\n", rp_strerr(error));
				goto exit;
			}
		}

		printf("Request successfully parsed!\n");
	}

exit:
	printf("Terminating client connection...\n");

	// Destroy request parser
	rp_parser_destroy(parser);
	// Free args
	free(args);
	// Terminate thread
	pthread_exit(NULL);
}

static int handle_option(char *opt) {
	if (strcmp(opt, "-h") == 0) {
		printf("Help Menu goes here\n");
		return 1;
	}

	printf("Unknown option specified\n");
	return 1;
}

int main(int argc, char *argv[]) {

	// Parse args and set port number to use
	int port_num = -1;
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (strncmp(argv[i], "-", 1) == 0) {
				if (handle_option(argv[i])) {
					return 1;
				}
			} else {
				port_num = atoi(argv[i]);
			}
		}
	}

	if (port_num == -1) {
		// Obtain http service
		struct servent *serv  = getservbyname("http", "tcp");
		// Does not require htons as getservbyname returns port number in network byte order (big endian)
		port_num = serv->s_port;
		endservent();
	}

	printf("Initiating Web Server...\n\n");
	
	struct sockaddr_in addr;
	
	// Create socket
	int sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_listen < 0) {
		printf("Failed to create listening socket\n");
		return 1;
	}
	else {
		printf("Created socket with file descriptor: %d\n", sock_listen);
	}
	
	// Setup the address (port number) to bind to
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port_num;
	addr.sin_addr.s_addr = INADDR_ANY;
	
	// Bind the socket to the address
	if (bind(sock_listen, (struct sockaddr *) &addr, sizeof(addr)) == 0) {
		printf("Successfully bound socket to port %d\n", ntohs(addr.sin_port));
		// printf("Successfully bound socket to port %d (%d)\n", ntohs(addr.sin_port), addr.sin_port);
	}
	else {
		printf("Failed to bind socket to port %d: %s\n", ntohs(addr.sin_port), strerror(errno));
		close(sock_listen);
		return 1;
	}
	
	// Listen on socket
	if (listen(sock_listen, BACKLOG) == 0) {
		printf("Listening on socket %d\n", sock_listen);

		// Get current hosting ip address
		FILE *ptr = popen(HOST_LOOKUP_CMD, "r");
		if (ptr != NULL) {
			printf("Host(s): ");

			char buf[MAX_HOST_LEN];
			char *success;
			do {
				memset(buf, 0, MAX_HOST_LEN);
				success = fgets(buf, MAX_HOST_LEN, ptr);
				if (buf != NULL) {
					printf("%s", buf);
				}
			} while(success != NULL);
		}
	}
	else {
		printf("Failed to listen on socket %d\n", sock_listen);
		close(sock_listen);
		return 1;
	}
	
	struct sockaddr client_address;
	socklen_t address_len;
	
	// Loop for accepting connections
	printf("Ready for client connections...\n");
	while (1) {
		int sock_accept = accept(sock_listen, &client_address, &address_len);
		if (sock_accept == -1) {
			printf("Failed to accept connection on listening socket %d\n", sock_listen);
		}
		else {
			printf("Accepted new connection. Created socket with file descriptor: %d\n", sock_accept);

			pthread_t new_thread;
			struct thread_args *args = malloc(sizeof(struct thread_args));
			args->socket_fd = sock_accept;

			if (pthread_create(&new_thread, NULL, listen_to_client, (void *) args)) {
				printf("Failed to create thread for connection with file descriptor: %d\n", sock_accept);
			}
		}
	}
	
	// Cleanup
	close(sock_listen);

	return 0;
}