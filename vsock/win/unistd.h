typedef ULONGLONG uint64_t;
#define unlink _unlink
#define sleep(x) Sleep(x * 1000)
#define CLOCK_MONOTONIC 0
#define socket(a, b, c) (-1)
#define listen(a, b) (-1)
#define accept(a, b, v) (-1)
#define connect(a, b, c) (-1)
#define bind(a, b, c) (-1)

#if 0
#define open  openx
#define close closex
#define read readx
#define write writex
#define FD_VALUE    5
static HANDLE hf;

static __inline int openx(char *name, int mode)
{
    hf = CreateFileA(name, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hf == INVALID_HANDLE_VALUE)
        hf = NULL;
    return hf ? FD_VALUE : -1;
}

static __inline void closex(int fd)
{
    if (fd == FD_VALUE && hf) CloseHandle(hf);
}

static __inline int readx(int fd, void *buf, int size)
{
    ULONG done = 0;
    if (!ReadFile(hf, buf, size, &done, NULL))
        return -1;
    else
        return (int)done;
}

static __inline int writex(int fd, void *buf, int size)
{
    ULONG done = 0;
    if (!WriteFile(hf, buf, size, &done, NULL))
        return -1;
    else
        return (int)done;
}
#else

#define open _open
#define close _close
#define read _read
#define write _write

#endif
