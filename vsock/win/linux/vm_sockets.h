#ifndef __vm_sockets_h__
#define __vm_sockets_h__

struct sockaddr_vm
{
    UINT svm_family;
    UINT svm_port;
    UINT svm_cid;
};

#define AF_VSOCK 0
#define VMADDR_PORT_ANY 0

struct sockaddr_un
{
    UINT family;
    char sun_path[32];
};

#endif
