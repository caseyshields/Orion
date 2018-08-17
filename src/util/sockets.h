/** @file socket.h
 * @brief A crude adapter so we can link to either winsock or unix sockets.
 *
 * This header is just an adapter to the sockets implementation of the build environment. It is
 * not a completly insulating layer; there is enough overlap between winsock and posix sockets
 * that we use the same method names and just swap out the header for all but a few methods.
 *
 * @author Casey Shields */

#ifndef STARTRACK_SOCKETS_H
#define STARTRACK_SOCKETS_H

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
#include <errno.h>

#define INVALID_SOCKET (unsigned int)(~0)
#define SOCKET_ERROR -1

#define SLEEP_RESOLUTION 1

#endif // WIN32

/** Loads socket resources for the platform. */
int socket_load();

/** Obtains more detailed platform error information, if available */
int socket_error();

/** Close the connection then close the socket. */
int socket_close( unsigned int socket );

/** Release any platform resources needed for sockets */
int socket_unload();

#endif //STARTRACK_SOCKETS_H
