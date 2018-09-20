/** @file test.h
 * @brief A test suite which invokes modules' unit tests, as well as higher level integration tests.
 * @author Casey Shields */

#ifndef STARTRACK_TEST_H
#define STARTRACK_TEST_H

#include <assert.h>
#include "util/crc.h"
#include "util/vmath.h"
#include "util/jday.h"
#include "util/io.h"
#include "data/tats.h"
#include "data/iers.h"
#include "engine/tracker.h"
#include "engine/catalog.h"
#include "../lib/cutest-1.5/CuTest.h"

/** Runs the entire test suite and prints the results to standard out.
 * Program then exits, returning the number of failed tests. */
void test_run();

/** @return a CuTest suite of tests for Orion. */
CuSuite * test_suite();

/** An integration test which compares Orion results against USNO reference implementation results.
 * of Tracker, IERS, Entry & jday. */
void test_prediction( CuTest * test );

#endif //STARTRACK_TEST_H
