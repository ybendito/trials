/* --- client.c --- */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <linux/vm_sockets.h>
#include <linux/un.h>

#define MAGIC 0xaabbccff

typedef struct
{
	int magic;
	int size;
} req_header;

static void do_client_job(int sockfd)
{
	req_header r;
	int size = 1024 * 1024;
	int max_size = 16 * size;
	void *p = malloc(max_size);
	for (; size <= max_size; size *= 2)
	{
		clock_t t1 = clock(), t2;
		r.magic = MAGIC;
		r.size  = size;
		if (write(sockfd, &r, sizeof(r)) < 0) {
    		printf("Can't write %d, error %d", sizeof(r), errno);
			return;
		}
		if (write(sockfd, p, size) < 0) {
    		printf("Can't write %d, error %d", size, errno);
			return;
		}
		r.magic = r.size = 0;
		if (read(sockfd, &r, sizeof(r)) < 0) {
    		printf("Can't read header, error %d", errno);
			return;
		}
		if (r.magic != MAGIC || r.size != size) {
    		printf("Wrong header received for size %d", size);
			return;
		}
		t2 = clock();
   		printf("%d transferred in %d ms", size, t2 - t1);
	}
	r.magic = r.size = 0;
	if (write(sockfd, &r, sizeof(r)) < 0) {
		printf("Can't write %d, error %d", sizeof(r), errno);
		return;
	}
	free(p);
}

static void do_server_job(int sockfd)
{
	int max_size = 64 * 1024 * 1024;
	void *p = malloc(max_size);
	req_header r;
	int res;
	do
	{
		if (read(sockfd, &r, sizeof(r)) < 0) {
    		printf("Can't read header, error %d", errno);
			return;
		}
		if (r.magic != MAGIC) {
    		printf("Wrong header received");
			return;
		}
		if (r.size == 0)
			break;
		if (r.size > max_size)
		{
    		printf("too large block");
			return;
		}
		res = read(sockfd, p, r.size);
		if (res < 0) {
    		printf("Can't read data, error %d", errno);
			return;
		}
		if (res < r.size) {
    		printf("partial read of %d < %d", res, r.size);
			return;
		}
		if (write(sockfd, &r, sizeof(r)) < 0) {
			printf("Can't write %d, error %d", sizeof(r), errno);
			return;
		}
	} while (1);
}


static int usage(char *s)
{
    printf("%s [-v|-s] [-c]\n", s);
    printf("\t-v\t\tUse vsync\n");
    printf("\t-s\t\tUse serial\n");
    printf("\t-c\t\tClient\n");
    return 1;
}

int main(int argc, char *argv[])
{
	int sockfd = 0, ret;
    int client = 0, vsock = 0;
	int type = SOCK_STREAM;
	struct sockaddr_vm serv_addr = { 0 };
	struct sockaddr_vm local_addr = { 0 };
    struct  sockaddr_un unix_addr = { AF_UNIX, "/tmp/foo" };

	if(argc < 2)
	{
        return usage(*argv);
    }

	local_addr.svm_family = AF_VSOCK;
	local_addr.svm_port = VMADDR_PORT_ANY;
	local_addr.svm_cid = 3;

	serv_addr.svm_family = AF_VSOCK;
	serv_addr.svm_port = 5000;
	serv_addr.svm_cid = 2;


	if(argc >= 2)
	{
		int i;
		for (i = 1; i < argc; ++i)
		{
			char *s = argv[i];
			if (s[0] != '-') {
                return usage(*argv);
            }
            switch(s[1])
            {
                case 'c': client = 1; break;
                case 's': vsock = 0; break;
                case 'v': vsock = 1; break;
                default: return usage(*argv);
            }
		}
	}

    if (vsock)
        sockfd = socket(AF_VSOCK, type, 0);
    else if (!client)
        sockfd = socket(AF_UNIX, type, 0);
    else
        sockfd = open("/dev/virtio-ports/test0", O_RDWR);

    if (sockfd <= 0) {
		printf("error : Could not create socket, %s\n", strerror(errno));
		return 1;
    }

    if (vsock) {
        struct sockaddr_vm *a = client ? &local_addr : &serv_addr;
        ret = bind(sockfd, (struct sockaddr*)a, sizeof(local_addr));
    } else if (!client) {
        unlink(unix_addr.sun_path);
        ret = bind(sockfd, (struct sockaddr*)&unix_addr, sizeof(unix_addr));
    } else {
        ret = 0;
    }

	if (ret < 0) {
		printf("error %d on bind\n", errno);
		return 1;
	}

    if (!client) {
		ret = listen(sockfd, 10);
		if (ret < 0) {
			printf("error on listen: %d\n", errno);
			return 1;
		}
    } 

	if (vsock && client) {
		ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
		if (ret < 0) {
			printf("Failed to connect to %d:%d, error %d\n", serv_addr.svm_cid, serv_addr.svm_port, errno);
			return 1;
		}
	}

	if (!client) {
		int listenfd = sockfd; 
		socklen_t len = sizeof(local_addr);
		sockfd = accept(listenfd, (struct sockaddr*)&local_addr, &len);
		if (ret < 0) {
			printf("Failed on accept, error %d\n", errno);
			return 1;
		}
		close(listenfd);
	}

	if (client) {
		do_client_job(sockfd);
	} else {
		do_server_job(sockfd);
	}

	close(sockfd);

	return 0;
}