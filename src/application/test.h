/** @file test.h
 * @brief A test suite used for development and troubleshooting.
 * @author Casey Shields */

#ifndef STARTRACK_TEST_H
#define STARTRACK_TEST_H

#include <assert.h>
#include "util/crc.h"
#include "util/vmath.h"
#include "util/io.h"
#include "data/tats.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "../lib/cutest-1.5/CuTest.h"

/** Runs the entire test suite and prints the results to standard out.
 * Program then exits, returning the number of failed tests. */
void test_run();

/** @return a CuTest suite of tests for Orion. */
CuSuite * test_suite();

/** tests conversion between angle representations
 * @param test the CuTest structure which holds test results. */
void test_angles( CuTest * test );

/** tests conversions between the novas jday based time representation and a few others
 * @param test the CuTest structure which holds test results. */
void test_time( CuTest * test );

/** Tests whether the memory layout of Tats structures are correct
 * @param test the CuTest structure which holds test results. */
void test_tats( CuTest * test );

/** Tests CRC checks on a few example inputs. Modified from code written by Bob Felice.
 * http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926
 * @param test the CuTest structure which holds test results. */
void test_crc( CuTest * test );

/** Loads an example fk6 dataset, and only performs some crude sanity checks
 * @param test the CuTest structure which holds test results. */
void test_FK6( CuTest * test );

/** Performs some calculations on a small set of stars and tests them against precomputed coordinates.
 * Directly taken from 'checkout-stars.c' from novas 3.1.
 * @param test the CuTest structure which holds test results. */
void test_novas( CuTest * test );

#endif //STARTRACK_TEST_H
