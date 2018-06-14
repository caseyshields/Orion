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

Orion * orion_create( Tracker * tracker ) {

}

int orion_select( int id ) {
    // check sensor control thread mode
    // obtain mutex
    // set target
    // release mutex
    return -1;
}

int orion_start( int * mode, pthread_t * control, Tracker * tracker) {
    if( !mode ) {
        pthread_create(&control, NULL, control_loop, &tracker);
        return 0;
    } else {
        printf("Sensor control thread already started.\n");
        return 1;
    }

}

unsigned int connect_sensor ( struct sockaddr * address ) {
    char * error = NULL;
    int status = 0;

    // load the winsock library
    WSADATA wsadata;
    status = WSAStartup( MAKEWORD(2, 2), &wsadata);
    if( status != 0 ) {
        error = "Failed to initialize winsock";
        goto EXIT;
    }

    // create a TCP socket for the client
    unsigned int client = socket(PF_INET/*AF_UNSPEC*/, SOCK_STREAM, IPPROTO_TCP);
    if( client == INVALID_SOCKET ) {
        error = "Failed to create socket";
        goto CLEANUP_WINSOCK;
    }

    // connect the socket to the server
    status = connect( client, (struct sockaddr*)&(address), sizeof(address) );
    if( status == SOCKET_ERROR ) {
        error = "Failed to connect to the server";
        goto CLEANUP_SOCKET;
    }

    CLEANUP_SOCKET:
    status = WSAGetLastError();
    closesocket(client); // just close in POSIX

    CLEANUP_WINSOCK:
    WSACleanup(); // no posix equivalent

    EXIT:
    if (error) {
        printf("Error [%d] %s\n", status, error);
        fflush(stdout);
        return INVALID_SOCKET;
    } else {
        return client;
    }
}

void * sensor_control_loop( void * arg ) {
    int client = *((int*)arg);
    int status = 0;

    // enter a loop where the target is continually updated...
    while( true ) {
        // test connection
        char * teststr = "testy tester testing tests testily";
        int length = strlen( teststr );
        int sent = send( client, teststr, length, 0 );
        if( sent < length ) {
            status = WSAGetLastError();
            printf("Error [%d] Failed to send entire message, sent %d\n", status, sent);

            goto CLEANUP_CONNECTION;
        }
        break;
    }

    CLEANUP_CONNECTION:
    if( shutdown(client, SD_SEND) == SOCKET_ERROR ) // winsock specific
        ;// well we tried
}

int orion_stop(int * mode, pthread_t * control) {
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

    if( shutdown(client, SD_SEND) == SOCKET_ERROR ) // winsock specific
        ;// well we tried

    closesocket(client); // just close in POSIX

    WSACleanup(); // no posix equivalent

    return 0;
}
