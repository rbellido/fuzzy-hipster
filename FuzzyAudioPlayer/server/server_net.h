#ifndef __SERVER_NET_H
#define __SERVER_NET_H

#include "utils.h"

SOCKET createServer(WSADATA * wsadata, int protocol, SOCKADDR_IN * udpaddr = 0);

#endif
