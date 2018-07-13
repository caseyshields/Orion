#ifndef STARTRACK_SOCKETS_H
#define STARTRACK_SOCKETS_H

/** This header is just an adapter to the sockets implementation of the build environment */

#ifdef WIN32
#include <winsock.h>

#define SLEEP_RESOLUTION 1000
//todo where should I really shoehorn this timer stuff...

#else
#include <netdb.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define INVALID_SOCKET (unsigned int)(~0)
#define SOCKET_ERROR -1

#define SLEEP_RESOLUTION 1

#endif // WIN32

#endif //STARTRACK_SOCKETS_H
