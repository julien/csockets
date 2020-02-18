#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 80
#define MAXLINE 4096
#define SA struct sockaddr

void err_n_die(const char* fmt, ...);

int main(int argc, char* argv[argc+1]) {
	int    sockfd, n;
	int    sendbytes;
	struct sockaddr_in servaddr;
	char   sendline[MAXLINE];
	char   recvline[MAXLINE];

	if (argc != 2)
		err_n_die("usage: %s <server address>", argv[0]);

	/* Create the socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_n_die("error while creating socket!");

	/* Zero out the address */
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	/* htons = host to network, short */
 	/* Chat server */
	servaddr.sin_port = htons(SERVER_PORT);

	/* Translate address: i.e. "1.2.3.4" -> [1,2,3,4] */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_n_die("inet_pton error for %s", argv[1]);

	/* Connect to the server */
	if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
		err_n_die("connect failed");

	/* We're connected, so prepare the message */
	sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
	sendbytes = strlen(sendline);

	/*
		Send the request -- making sure you send it all
		This code is a bit fragile, since it bails if only some of the
		bytes are sent.
		Normally, you'd want to retry, unless the return value was -1
	 */
	if (write(sockfd, sendline, sendbytes) != sendbytes)
		err_n_die("write error");

	memset(recvline, 0, MAXLINE);

	/* Now read the server's response */
	while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0) {
		printf("%s", recvline);
		memset(recvline, 0, MAXLINE);
	}

	if (n < 0)
		err_n_die("read error");

	exit(0);
}

void err_n_die(const char* fmt, ...) {
	int errno_save;
	va_list ap;

	/* Save errno for later (in case it's overwritten) */
	errno_save = errno;

	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
	fflush(stdout);

	/* Print error message if errno was set */
	if (errno_save != 0) {
		fprintf(stdout, "(errno = %d) : %s\n", errno_save,
				strerror(errno_save));
		fprintf(stdout, "\n");
		fflush(stdout);
	}
	va_end(ap);

	/* Terminate with an error */
	exit(1);
}
