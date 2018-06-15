#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "novasc3.1/novas.h"
#include "h/tracker.h"
#include "h/catalog.h"
#include "h/vmath.h"

// we only allow one controller and one sensor
#define MAX_CONNECTIONS 2

#define MAX_BUFFER_SIZE 2<<10

double get_time();
unsigned int configure_server( int argc, char* argv[] );
char* get_arg( int argc, char *argv[], char *name, char* default_value );
void terminate( int status, char* msg );

int mode = 1;
char *buffer;
unsigned int server = INVALID_SOCKET;
unsigned int client = INVALID_SOCKET;

/** A simulator of a TATS sensor used for testing Orion.
 * @author Casey Shields
 */
int main( int argc, char *argv[] ) {
    buffer = malloc( MAX_BUFFER_SIZE );
    memset( buffer, 0, MAX_BUFFER_SIZE );

    // get server address, should be in dotted quad notation
    char* ip = get_arg( argc, argv, "-ip", "127.0.0.1");

    // get server socket port number
    unsigned short port; // port
    char* arg = get_arg( argc, argv, "-port", "43210" );//8080" );
    if( arg ) port = atoi( arg );

    // load the winsock library
    WSADATA wsadata;
    int result = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( result != 0 )
        terminate( result, "Failed to initialize winsock" );

    // create a TCP socket for connecting to the server
    server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( server == INVALID_SOCKET )
        terminate( WSAGetLastError(), "Failed to create socket");

    // construct server address structure
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET; // internet address family
    address.sin_addr.s_addr = inet_addr( ip ); // server ip
    address.sin_port = htons( port ); // local port

    // bind socket to host network
    result = bind( server, (struct sockaddr *) &address, sizeof(address) );
    if( result == SOCKET_ERROR )
        terminate( WSAGetLastError(), "Failed to bind server socket" );

    // set socket to listen for incoming connections
    result = listen( server, MAX_CONNECTIONS );
    if( result < 0 )
        terminate(result, "Failed to listen");

    // accept Orion clients
    while( mode ) { // todo need a way to gracefully exit

        printf("Awaiting client.\n");
        client = accept(server, NULL, NULL);
        if (client == INVALID_SOCKET) {
            //terminate(WSAGetLastError(), "failed to accept client connection\n");
            printf( "[%ld] Failed to accept client connection\n\0", WSAGetLastError() );
            continue;
        }

        printf("Client connected.\n");
        do {
            memset(buffer, 0, MAX_BUFFER_SIZE);
            int result = recv(client, buffer, MAX_BUFFER_SIZE, 0);
            if (result > 0) {
                printf("Recieved message:\n%s\n", buffer);
//          result = send( socket, &entry, sizeof(entry), 0);
//          if( result == SOCKET_ERROR )
//              terminate( WSAGetLastError(), "Failed to send back the entry");

            } else if (result == 0) {
                // terminate(0, NULL);
                printf( "[%ld] Client closed connection\n\0", WSAGetLastError());
                closesocket( client );
                break;
            } else {
                //terminate(WSAGetLastError(), "Failed to read from client\n");
                printf( "[%ld] Failed to read from client\n\0", WSAGetLastError());
                closesocket( client );
                break;
            }

        } while (true);
    }

    terminate(0, NULL);
}

/** Gets an accurate UTC timestamp from the system in seconds since the unix epoch */
double get_time() {
    struct timeval time;
    gettimeofday( &time, NULL );
    return time.tv_sec + (time.tv_usec / 1000000.0);
}

/** retrieves the value subsequent to the specified option. If the default_value
 * is supplied, the function will return it. otherwise the method will print an
 * error message and abort. */
char* get_arg( int argc, char *argv[], char *name, char* default_value ) {
    for( int n=0; n<argc; n++ )
        if( !strcmp(name, argv[n]) )
            return argv[n+1];
    if( default_value ) {
        printf("Parameter '%s' defaulting to '%s'\n", name, default_value);
        return default_value;
    }
    sprintf( "Error: missing parameter '%s'\n", name );
    exit( 1 );
}

void terminate(int status, char* msg) {
    if( msg!=NULL )
        printf( "Error : %s : %d", msg, status );

    // release resources
    if( buffer != NULL )
        free( buffer );

    if( server != INVALID_SOCKET )
        closesocket( server );

    if( client != INVALID_SOCKET )
        closesocket( client );

    // winsock cleanup routine
    WSACleanup();

    exit( status );
}

// retrieve a list of available sockets for the server
//    struct sockaddr_in hints;
//    mwmser(&hints, 0, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//    hints.ai_flags = AI_PASSIVE;
//
//    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, server);
//    if( result!=0 ) {
//        result = WSAGetLastError();
//        freeaddrinfo(server);
//        orion_exit( result, "failed to get server socket information" );
//    }
//    freeaddrinfo(address);// free the linked list of results if getaddrinfo() was used


//unsigned int configure_server(int argc, char* argv[]) {
//    int result;
//
//    // get server address, should be in dotted quad notation
//    char* ip = get_arg( argc, argv, "-ip", "127.0.0.1");
//
//    // get server socket port number
//    unsigned short port; // port
//    char* arg = get_arg( argc, argv, "-port", "43210" );//8080" );
//    if( arg ) port = atoi( arg );
//
//    // load the winsock library
//    WSADATA wsadata;
//    result = WSAStartup( MAKEWORD(2, 2), &wsadata);
//    if( result != 0 )
//        terminate( result, "Failed to initialize winsock" );
//
//    // create a TCP socket for connecting to the server
//    unsigned int server = INVALID_SOCKET; // listening socket descriptor
//    server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if( server == INVALID_SOCKET )
//        terminate( WSAGetLastError(), "Failed to create socket");
//
//
//    // construct server address structure
//    struct sockaddr_in address;
//    memset(&address, 0, sizeof(address));
//    address.sin_family = AF_INET; // internet address family
//    address.sin_addr.s_addr = inet_addr( ip ); // server ip
//    //address.sin_addr.s_addr = htonl( INADDR_ANY ); // any incoming interface
//    address.sin_port = htons( port ); // local port
//
//    // bind socket to host network
//    result = bind( server, (struct sockaddr *) &address, sizeof(address) );
//    if( result == SOCKET_ERROR )
//        terminate( WSAGetLastError(), "Failed to bind server socket" );
//
//    //freeaddrinfo(address);// free the linked list of results if getaddrinfo() was used
//
//    // set socket to listen for incoming connections
//    result = listen( server, MAX_CONNECTIONS );
//    if( result < 0 )
//        terminate(result, "Failed to listen");
//
//    return server;
//}