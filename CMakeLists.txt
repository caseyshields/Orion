cmake_minimum_required(VERSION 3.10)
project(startrack C)

set(CMAKE_C_STANDARD 11)

include_directories(src)
include_directories(src/novasc3.1)
include_directories(src/novasc3.1/ext)

add_executable(startrack
        src/novasc3.1/novas.c
        src/novasc3.1/novas.h
        src/novasc3.1/novascon.c
        src/novasc3.1/novascon.h
        src/novasc3.1/nutation.c
        src/novasc3.1/nutation.h
        src/novasc3.1/readeph0.c
        src/novasc3.1/solarsystem.h
        src/novasc3.1/solsys3.c
        src/legacy/FK6.c
        src/legacy/FK6.h
        src/legacy/Heap.c
        src/legacy/Heap.h
        src/legacy/StarMap.c
        src/legacy/StarMap.h
        src/legacy/test.c)

add_executable(checkout
        src/novasc3.1/test/checkout-stars.c
        src/novasc3.1/novas.c
        src/novasc3.1/novas.h
        src/novasc3.1/novascon.c
        src/novasc3.1/novascon.h
        src/novasc3.1/nutation.c
        src/novasc3.1/nutation.h
        src/novasc3.1/readeph0.c
        src/novasc3.1/solarsystem.h
        src/novasc3.1/solsys3.c src/catalog.h)

add_executable(tracker
        src/novasc3.1/novas.c
        src/novasc3.1/novas.h
        src/novasc3.1/novascon.c
        src/novasc3.1/novascon.h
        src/novasc3.1/nutation.c
        src/novasc3.1/nutation.h
        src/novasc3.1/readeph0.c
        src/novasc3.1/solarsystem.h
        src/novasc3.1/solsys3.c
        src/legacy/FK6.c
        src/legacy/FK6.h
        src/legacy/Heap.c
        src/legacy/Heap.h
        src/tracker.h
        src/tracker.c src/client.c src/catalog.h src/catalog.c src/vmath.c src/vmath.h)