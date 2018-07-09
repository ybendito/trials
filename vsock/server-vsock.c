#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <linux/vm_sockets.h>


static int get_cid()
{
	int i = 2;
	return i;
}

int main(int argc, char *argv[])
{
	int listenfd = 0, connfd = 0, error;
	struct sockaddr_vm serv_addr;

	char sendBuff[1025];
	time_t ticks;

	/* creates an UN-named socket inside the kernel and returns
	 * an integer known as socket descriptor
	 * This function takes domain/family as its first argument.
	 * For Internet family of IPv4 addresses we use AF_INET
	 */
	listenfd = socket(AF_VSOCK, SOCK_STREAM, 0);
	
	if (listenfd < 0) {
		printf("\nError on socket: %d\n", errno);
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.svm_family = AF_VSOCK;
	serv_addr.svm_cid = 2;
	serv_addr.svm_port = 5000;

	/* The call to the function "bind()" assigns the details specified
	 * in the structure 'serv_addr' to the socket created in the step above
	 */
	error = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	if (error < 0) {
		printf("\nError on bind: %d\n", errno);
		return 1;
	}
	printf("\nbound to %d:%d\n", serv_addr.svm_cid, serv_addr.svm_port);


	/* The call to the function "listen()" with second argument as 10 specifies
	 * maximum number of client connections that server will queue for this listening
	 * socket.
	 */
	error = listen(listenfd, 10);
	if (error < 0) {
		printf("\nError on listen: %d\n", errno);
		return 1;
	}

	while(1)
	{
		struct sockaddr_vm  client_addr;
		connfd = accept(listenfd, (struct sockaddr*)&client_addr, NULL);

		if (connfd < 0) {
			printf("\nError on accept %d\n", connfd);
			return 1;
		}
		printf("\nConnection accepted %d:%d\n", client_addr.svm_cid,  client_addr.svm_port);

		/* As soon as server gets a request from client, it prepares the date and time and
		 * writes on the client socket through the descriptor returned by accept()
		 */
		ticks = time(NULL);
		snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
		write(connfd, sendBuff, strlen(sendBuff)+1);

		close(connfd);
		sleep(1);
	}
}
