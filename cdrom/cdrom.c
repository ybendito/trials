/* --- cdrom.c --- */
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <linux/cdrom.h>

typedef struct _tCmd
{
    char cmd;
    const char *name;
    uint32_t code1;
    uint32_t code2;
}tCmd;

static const tCmd commands[] =
{
    { 'e', "eject", CDROM_LOCKDOOR, CDROMEJECT },
    { 'i', "insert", CDROMCLOSETRAY, 0 },
};


int main(int argc, char *argv[])
{
    char cmd = 0;
    int i;
    if(argc >= 3 && *argv[1] == '-')
	{
		char *p = argv[1];
        cmd = p[1];
	}
    else
    {
        puts("<command> <device>");
        for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i)
        {
            printf("-%c\t\t%s\n", commands[i].cmd, commands[i].name);
        }
        return 1;
    }
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i)
    {
        if (commands[i].cmd == cmd)
        {
            int error;
            int fd = open(argv[2], O_RDONLY | O_NONBLOCK);
            if (fd > 0) {
                error = ioctl(fd, commands[i].code1, 0);
                if (error) {
                    printf("[1] can't %s %s, res %d, errno %d\n", commands[i].name, argv[2], error, errno);
                }
                else if (commands[i].code2)
                {
                    error = ioctl(fd, commands[i].code2, 0);
                    if (error) {
                        printf("[2] can't %s %s, res %d, errno %d\n", commands[i].name, argv[2], error, errno);
                    }
                }
                if (!error) {
                    printf("OK %s %s\n", commands[i].name, argv[2]);
                }
                close(fd);
            }
        }
    }

	return 0;
}