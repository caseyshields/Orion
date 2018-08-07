cmake_minimum_required(VERSION 3.6.1)
project(startrack C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(src/novasc3.1)
include_directories(src/novasc3.1/ext)

# Compiles a static library for Novas 3.1 in reduced accuracy mode
set( NOVAS_SOURCE
        src/novasc3.1/novas.c
        src/novasc3.1/novas.h
        src/novasc3.1/novascon.c
        src/novasc3.1/novascon.h
        src/novasc3.1/nutation.c
        src/novasc3.1/nutation.h
        src/novasc3.1/readeph0.c
        src/novasc3.1/solarsystem.h
        src/novasc3.1/solsys3.c )
add_library( novas STATIC ${NOVAS_SOURCE} )

# This is the test program that came with Novas for reduced accuracy mode;
# Its output is meant to match "data/novas/checkout-stars-usno.txt"
add_executable( checkout src/novasc3.1/test/checkout-stars.c )
if(WIN32)
    target_link_libraries( checkout novas )
else(WIN32)
    target_link_libraries( checkout novas m )
endif(WIN32)


# These files make up the C Unit Test library
set( CUTEST_SOURCE
        lib/cutest-1.5/CuTest.h
        lib/cutest-1.5/CuTest.c )
add_library( cutest STATIC ${CUTEST_SOURCE} )

# These file comprise the source of the Orion star hunter software
set( ORION_SOURCE
        src/util/vmath.c
        src/util/io.c
        src/util/jday.c
        src/util/crc.c
        src/data/fk6.c
        src/data/tats.c
        src/engine/tracker.c
        src/engine/catalog.c
        )

# This is an interactive commandline program for benchmarking and testing program components
add_executable( tester ${ORION_SOURCE} src/application/test.c)
if(WIN32)
    target_link_libraries( tester novas cutest )
else(WIN32)
    target_link_libraries( tester novas cutest m )
endif(WIN32)

add_executable( sensor ${ORION_SOURCE} src/util/sockets.c src/application/sensor.c)
if (WIN32)
    target_link_libraries( sensor novas wsock32 ws2_32 )
else(WIN32)
    target_link_libraries( sensor novas m )
endif(WIN32)

#add_executable( sensor2 ${ORION_SOURCE} src/util/sockets.c src/application/sensor.c)
#target_link_libraries( sensor2 novas )

add_executable( orion ${ORION_SOURCE} src/util/sockets.c src/controller/orion.c src/application/main.c)
if (WIN32)
    target_link_libraries( orion novas pthread wsock32 ws2_32 )
else (WIN32)
    target_link_libraries( orion novas pthread m )
endif (WIN32)

# this is the test program for the test library
#add_executable( cutest CUTEST_SOURCE )