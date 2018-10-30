#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define die(str, args...) do { \
        perror(str); \
        exit(EXIT_FAILURE); \
    } while(0)

#define ELEMENTS_IN(a) sizeof(a)/sizeof(a[0]) 

typedef struct
{
    int key;
    int scan;
}  tKeyScan;

static tKeyScan keytable[] = 
{
    { 'a', KEY_A },
    { 'b', KEY_B },
    { 'c', KEY_C },
    { 'd', KEY_D },
    { 'e', KEY_E },
    { 'f', KEY_F },
    { 'g', KEY_G },
    { 'h', KEY_H },
    { 'i', KEY_I },
    { 'j', KEY_J },
    { 'k', KEY_K },
    { 'l', KEY_L },
    { 'm', KEY_M },
    { 'n', KEY_N },
    { 'o', KEY_O },
    { 'p', KEY_P },
    { 'q', KEY_Q },
    { 'r', KEY_R },
    { 's', KEY_S },
    { 't', KEY_T },
    { 'u', KEY_U },
    { 'v', KEY_V },
    { 'w', KEY_W },
    { 'x', KEY_X },
    { 'y', KEY_Y },
    { 'z', KEY_Z },
};

static int findscan(int c)
{
    int i;
    for (i = 0; i < ELEMENTS_IN(keytable); ++i)
    {
        if (keytable[i].key == c)
            return keytable[i].scan;
    }
    return -1;
}

int main(void)
{
    int                    fd;
    struct uinput_user_dev uidev;
    int                    i;

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0)
        die("error: open");

    if(ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        die("error: ioctl");
    for (i = 0; i < ELEMENTS_IN(keytable); ++i)
    {
        if(ioctl(fd, UI_SET_KEYBIT, keytable[i].scan) < 0)
            die("error: ioctl");
    }
#if 0
    if(ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_X) < 0)
        die("error: ioctl");
    if(ioctl(fd, UI_SET_RELBIT, REL_Y) < 0)
        die("error: ioctl");
#endif        

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0)
        die("error: write");

    if(ioctl(fd, UI_DEV_CREATE) < 0)
        die("error: ioctl");

    puts("type a word:");

    do {
        struct input_event ev;
        i = getchar();
        if (i == ' ') break;
        i = findscan(i);
        if (i < 0) continue;
        ev.type = EV_KEY;
        ev.code = i;
        ev.value = 1;
        if (write(fd, &ev, sizeof(ev)) < 0)
            break;
        usleep(1);
        ev.value = 0;            
        if (write(fd, &ev, sizeof(ev)) < 0)
            break;
        ev.type = EV_SYN;
        ev.code = 0;
        ev.value = 0;
        if (write(fd, &ev, sizeof(ev)) < 0)
            break;
    } while (1);

    if(ioctl(fd, UI_DEV_DESTROY) < 0)
        die("error: ioctl");

    close(fd);

    return 0;
}
