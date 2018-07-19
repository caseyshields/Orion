#include "h/sockets.h"

int socket_load() {
    int result = 0;
#ifdef WIN32
    // in windows we need to load the winsock DLL
    WSADATA wsadata;
    result = WSAStartup( MAKEWORD(2, 2), &wsadata);
#endif
    return result;
}

int socket_error() {
    int error = -1;
#ifdef WIN32
    error = WSAGetLastError();
#else
    error = errno;
#endif
    return error;
}

int socket_close(unsigned int socket) {
    int status = 0;
#ifdef WIN32
    status = shutdown(socket, SD_BOTH);
    if (status == 0) { status = closesocket(socket); }
#else
    status = shutdown(socket, SHUT_RDWR);
    if (status == 0) { status = close(socket); }
#endif
    return status;
}

int socket_unload() {
    int result = 0;
#ifdef WIN32
    // unloads the winsock libraries
    result = WSACleanup(); // no posix equivalent
#endif
    return result;
}