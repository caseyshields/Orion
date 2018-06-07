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
target_link_libraries( checkout novas )

# These file comprise the source of the Orion star hunter software
set( ORION_SOURCE
        src/tracker.h
        src/tracker.c
        src/catalog.h
        src/catalog.c
        src/vmath.c
        src/vmath.h
        src/fk6.c
        src/fk6.h)

# This is an interactive commandline program for benchmarking and testing program components
add_executable(tester ${ORION_SOURCE} src/test.c)
target_link_libraries( tester novas )

add_executable( server ${ORION_SOURCE} src/orion.c src/orion.h )
target_link_libraries( server novas wsock32 ws2_32 )