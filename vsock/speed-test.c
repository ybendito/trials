/*
typical qemu command-line
for vsock:
-device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=3,bus=pci.0,addr=0x4
for serial:
-chardev socket,path=/tmp/foo,id=foo -device virtserialport,bus=virtio-serial0.0,nr=2,chardev=foo,id=test0,name=test0
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
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

static int write_splitted(int sockfd, char *buf, int size, int split)
{
	int chunk = 0x4000;
	int done  = 0;
	int res;
	if (!split) {
		return write(sockfd, buf, size);
	}
	while (size)
	{
		if (size < chunk)
			chunk = size;
		res = write(sockfd, buf, chunk);
		if (res <= 0) {
			return res;
		}
		done += res;
		buf  += res;
		size -= res;
	}
	return done;
}

static uint64_t time_ms()
{
	struct timespec ts;
	uint64_t res;
	if (clock_gettime(CLOCK_MONOTONIC, &ts))
		return 0;
	res = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	return res;
}

static void do_client_job(int sockfd, int split)
{
	req_header r;
	int size = 1024 * 1024;
	int max_size = 64 * size;
	void *p = malloc(max_size);
	for (; size <= max_size; size *= 2)
	{
		uint64_t t1 = time_ms(), t2;
		r.magic = MAGIC;
		r.size  = size;
		if (write(sockfd, &r, sizeof(r)) < 0) {
			printf("Can't write %d, error %d\n", (int)sizeof(r), errno);
			return;
		}
		if (write_splitted(sockfd, p, size, split) < 0) {
			printf("Can't write %d, error %d\n", (int)size, errno);
			return;
		}
		r.magic = r.size = 0;
		if (read(sockfd, &r, sizeof(r)) < 0) {
			printf("Can't read header, error %d\n", errno);
			return;
		}
		if (r.magic != MAGIC || r.size != size) {
			printf("Wrong header received for size %d\n",(int)size);
			return;
		}
		t2 = time_ms();
   		printf("%d transferred in %d ms\n", size, (int)(t2 - t1));
		sleep(3);		   
	}
	r.size = 0;
	if (write(sockfd, &r, sizeof(r)) < 0) {
		printf("Can't write %d, error %d\n", sizeof(r), errno);
		return;
	}
	free(p);
}

static int do_server_job(int sockfd)
{
	int max_size = 128 * 1024 * 1024;
	void *p = malloc(max_size);
	req_header r;
	int res;
	int done;
	do
	{
		char *buf = p;
		if (read(sockfd, &r, sizeof(r)) < 0) {
			printf("Can't read header, error %d\n", errno);
			return 1;
		}
		if (r.magic != MAGIC) {
			printf("Wrong header received\n");
			return 1;
		}
		if (r.size == 0)
			break;
		if (r.size > max_size)
		{
			printf("too large block\n");
			return 1;
		}
		done = 0;
		do
		{
			res = read(sockfd, buf, r.size - done);
			if (res < 0) {
				printf("Can't read data, error %d\n", errno);
				return 1;
			}
			if (res == 0) {
				printf("Disconnected, error %d\n", errno);
				return 1;
			}
			done += res;
			buf += res;
		} while (done < r.size);

		if (write(sockfd, &r, sizeof(r)) < 0) {
			printf("Can't write %d, error %d\n", (int)sizeof(r), errno);
			return 1;
		}
	} while (1);
	
	free(p);
	return 0;
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
		do_client_job(sockfd, vsock == 0);
	} else {
		while (!do_server_job(sockfd) && !vsock);
	}

	close(sockfd);

	return 0;
}