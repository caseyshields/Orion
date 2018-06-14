#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pthread.h>
#include <winsock.h>

#include "novasc3.1/novas.h"
#include "tracker.h"
#include "vmath.h"
#include "orion.h"

Orion* orion_create( Orion * orion ) {
    if (!orion)
        orion = malloc(sizeof(orion));
    if (!orion) {
        memset(orion, 0, sizeof(orion));
        orion->client = INVALID_SOCKET;
        pthread_mutex_init(orion->mutex, NULL);
    }
    return orion;
}

int orion_is_connected (Orion * orion) {
    return orion != INVALID_SOCKET;
}

int orion_is_running ( Orion * orion ) {
    pthread_mutex_lock( orion->mutex );
    int running = orion->mode;
    pthread_mutex_unlock( orion->mutex ); //https://www.cs.nmsu.edu/~jcook/Tools/pthreads/library.html
    return running;
}

int orion_connect ( Orion * orion, char * ip, unsigned short port ) {
    char * error = NULL;
    int status = 0;

    // construct server address structure
    struct sockaddr_in address = {
            .sin_zero = {0,0,0,0,0,0,0,0},
            .sin_family = AF_INET, // internet address family
            .sin_addr.s_addr = inet_addr(ip), // server ip
            .sin_port = htons(port) // server port
    };

    // load the winsock library
    WSADATA wsadata;
    status = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( status != 0 ) {
        error = "Failed to initialize winsock";
        goto EXIT;
    }

    // create a TCP socket for the client
    orion->client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if( orion->client == INVALID_SOCKET ) {
        error = "Failed to create socket";
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    status = connect( orion->client, (struct sockaddr*)&(address), sizeof(address) );
    if( status == SOCKET_ERROR ) {
        error = "Failed to connect to the server";
        goto CLEANUP_SOCKET;
    }

    status = 0;
    goto EXIT;

    CLEANUP_SOCKET:
    status = WSAGetLastError();
    closesocket(orion->client); // just close in POSIX

    CLEANUP_WINSOCK:
    WSACleanup(); // no posix equivalent

    EXIT:
    if (error)
        sprintf(orion->error, "[%d] %s\n", status, error);
    return status;
}

void * orion_control_loop( void * arg ) {
    Orion * orion = (Orion*)arg;
    int status = 0;

    // enter a loop where the target is continually updated...
    while( orion->mode ) {
        // test connection
        char * teststr = "testy tester testing tests testily";
        int length = strlen( teststr );
        int sent = send( orion->client, teststr, length, 0 );
        if( sent < length ) {
            status = WSAGetLastError();
            printf("Error [%d] Failed to send entire message, sent %d\n", status, sent);

            goto CLEANUP_CONNECTION;
        }
        break;
    }



    return (void*) orion;
}

int orion_start( Orion * orion ) {//int * mode, pthread_t * control, Tracker * tracker) {
    // get mutex
    // check server is off
    if( !orion->mode ) {
        orion->mode = 1;
        pthread_create(&orion->control, NULL, orion_control_loop, &orion);
        return 0;
    } else {
        printf("Sensor control thread already started.\n");
        return 1;
    }

}

int orion_select( int id ) {
    // check sensor control thread mode
    // obtain mutex
    // set target
    // release mutex
    return -1;
}

int orion_stop(Orion * orion) {//int * mode, pthread_t * control) {
    if( mode ) {
        int* value = NULL;
        pthread_join( *control, (void**)&value );
        printf( "thread returned with value %d", *value );
        free(value);
        return 0;
    } else {
        printf( "Sensor control thread not running.\n");
        return 1;
    }
}

int disconnect_sensor(unsigned int client) {

    if(client==INVALID_SOCKET)
        return 1;

    //CLEANUP_CONNECTION:
    if( shutdown(client, SD_SEND) == SOCKET_ERROR ) // winsock specific
        ;// well we tried

    closesocket(client); // just close in POSIX

    WSACleanup(); // no posix equivalent

    return 0;
}
