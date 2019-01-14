cmake_minimum_required(VERSION 3.6.1)
project(startrack C)

set(CMAKE_C_STANDARD 11)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)

configure_file(
        "${PROJECT_SOURCE_DIR}/src/application/config.h.in"
        "${PROJECT_SOURCE_DIR}/src/application/config.h"
)

include_directories(src)
include_directories(lib)

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
        src/application/config.h
        )
if (WIN32)
    target_link_libraries( orion cutest novas pthread wsock32 ws2_32 )
else (WIN32)
    target_link_libraries( orion cutest novas pthread m )
endif (WIN32)

# generate the installation script
#set( ORION_INSTALL "C:/Users/Casey Shields/orion" )
#set( ORION_INSTALL "C:/Users/shiel/orion" )

#example MinGW installation
#set( ORION_INSTALL "C:/MinGW/msys/1.0/home/Casey Shields/orion" )

#set( ORION_INSTALL "${CMAKE_INSTALL_PREFIX}" )
set( ORION_INSTALL "C:/cygwin64/home/Casey/orion" )
message( "\nORION_INSTALL : ${ORION_INSTALL}" )

# generate the installation script
install( TARGETS orion sensor cutest novas
        RUNTIME DESTINATION ${ORION_INSTALL}/bin
        ARCHIVE DESTINATION ${ORION_INSTALL}/lib )

install( DIRECTORY data
        DESTINATION ${ORION_INSTALL} )

install( DIRECTORY src
        DESTINATION ${ORION_INSTALL}/include
        FILES_MATCHING PATTERN "*.h" )

#install( DIRECTORY docs DESTINATION ${ORION_INSTALL_LOCATION} )
# TODO instead I should invoke the doxygen tool rather than pushing all this into the repo...
find_package( Doxygen )
if (DOXYGEN_FOUND)
    message("\nFound Doxygen version ${DOXYGEN_VERSION}\n")
    #so CMAKE generates it's own doxyfile and doesn't accept one as an argument.
    # I think I need to remove the documentation from the master branch so the user always generates it locally.
    # I need to change the git hub page to host from the doc branch where I will commit the docs
    # and I need to remove the doxygen command from the normal build process since it takes awhile...
    #doxygen_add_docs(
    #    orion
    #    ${PROJECT_SOURCE_DIR}
    #    COMMENT "Generate man pages"
    #)
ENDIF(DOXYGEN_FOUND)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
        "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
include (CPack)