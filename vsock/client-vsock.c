/* --- client.c --- */
#include <sys/socket.h>
#include <sys/types.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <linux/vm_sockets.h>

int main(int argc, char *argv[])
{
	int sockfd = 0, n = 0, address = 2;
	int type = SOCK_DGRAM;
//	int type = SOCK_STREAM;
	char recvBuff[1024];
	struct sockaddr_vm serv_addr;
	struct sockaddr_vm local_addr;

	if(argc >= 2)
	{
		address = *argv[1] - '0';
		printf("Using server address %d\n", address);
	}

	memset(recvBuff, '0',sizeof(recvBuff));

	/* a socket is created through call to socket() function */
	if((sockfd = socket(AF_VSOCK, type, 0)) < 0)
	{
		printf("Error : Could not create socket \n");
		return 1;
	}

	memset(&local_addr, 0, sizeof(local_addr));

	if (0) {
		local_addr.svm_family = AF_VSOCK;
		local_addr.svm_port = VMADDR_PORT_ANY;
		local_addr.svm_cid = 2;

		if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
			printf("Bind failed, error %d\n", errno);
			return 1;
		}
	}

	memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.svm_family = AF_VSOCK;
	serv_addr.svm_port = 5000;
	serv_addr.svm_cid = address;

	if(type == SOCK_STREAM && connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		unsigned int len = sizeof(local_addr);
		printf("Failed to connect to %d:%d, error %d\n", serv_addr.svm_cid, serv_addr.svm_port, errno);
		if (getsockname(sockfd, (struct sockaddr *)&local_addr, &len) == 0) {
			printf("bound to %d:%d\n", local_addr.svm_cid, local_addr.svm_port);
		}
		return 1;
	}

	/* Once the sockets are connected, the server sends the data (date+time)
	 * on clients socket through clients socket descriptor and client can read it
	 * through normal read call on the its socket descriptor.
	 */
	while (type == SOCK_STREAM && (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
	{
		recvBuff[n] = 0;
		if(fputs(recvBuff, stdout) == EOF)
		{
			printf("\n Error : Fputs error\n");
		}
	}

	if (type == SOCK_DGRAM)
	{
		time_t ticks = time(NULL);
		snprintf(recvBuff, sizeof(recvBuff), "%.24s\r\n", ctime(&ticks));
		if (sendto(sockfd, recvBuff, strlen(recvBuff)+1, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
			printf("sendto %d:%d error %d\n",
			 serv_addr.svm_cid, serv_addr.svm_port, errno);
		}
	}

	if(n < 0)
	{
		printf("\n Read error \n");
	}

	return 0;
}