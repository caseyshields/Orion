cmake_minimum_required(VERSION 3.6.1)
project(startrack C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(lib/novasc3.1)
include_directories(lib/novasc3.1/ext)

# Compiles a static library for Novas 3.1 in reduced accuracy mode
set( NOVAS_SOURCE
        lib/novasc3.1/novas.c
        lib/novasc3.1/novas.h
        lib/novasc3.1/novascon.c
        lib/novasc3.1/novascon.h
        lib/novasc3.1/nutation.c
        lib/novasc3.1/nutation.h
        lib/novasc3.1/readeph0.c
        lib/novasc3.1/solarsystem.h
        lib/novasc3.1/solsys3.c)
add_library( novas STATIC ${NOVAS_SOURCE} )

# Create a static library of the C Unit Test framework
set( CUTEST_SOURCE
        lib/cutest-1.5/CuTest.h
        lib/cutest-1.5/CuTest.c )
add_library( cutest STATIC ${CUTEST_SOURCE} )

# this is the test program for the test library
#add_executable( cutest CUTEST_SOURCE lib/cutest-1.5/CuTestTest.c )

# These sources comprise the data, engine and utility layers of the Orion project
set( ORION_SOURCE
        src/util/vmath.c
        src/util/io.c
        src/util/jday.c
        src/util/crc.c
        src/util/sockets.c
        src/data/fk6.c
        src/data/iers.c
        src/data/tats.c
        src/engine/tracker.c
        src/engine/catalog.c
        )

# Create a TATS sensor simulator
add_executable( sensor
        ${ORION_SOURCE}
        src/application/sensor.c
        )
if (WIN32)
    target_link_libraries( sensor cutest novas wsock32 ws2_32 )
else(WIN32)
    target_link_libraries( sensor cutest novas m )
endif(WIN32)

# Create the Orion command line application
add_executable( orion
        ${ORION_SOURCE}
        src/controller/orion.c
        src/application/main.c
        src/application/test.c
        )
if (WIN32)
    target_link_libraries( orion cutest novas pthread wsock32 ws2_32 )
else (WIN32)
    target_link_libraries( orion cutest novas pthread m )
endif (WIN32)

## This executable was converted to a unit test and added to the main test suite
# This is the test program that came with Novas for reduced accuracy mode;
# Its output is meant to match "data/novas/checkout-stars-usno.txt"
#add_executable( checkout lib/novasc3.1/test/checkout-stars.c)
#if(WIN32)
#    target_link_libraries( checkout novas )
#else(WIN32)
#    target_link_libraries( checkout novas m )
#endif(WIN32)