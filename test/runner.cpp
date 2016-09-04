/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_ClockTest_init = false;
#include "ClockTest.h"

static ClockTest suite_ClockTest;

static CxxTest::List Tests_ClockTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_ClockTest( "ClockTest.h", 3, "ClockTest", suite_ClockTest, Tests_ClockTest );

static class TestDescription_suite_ClockTest_testClockTick : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_ClockTest_testClockTick() : CxxTest::RealTestDescription( Tests_ClockTest, suiteDescription_ClockTest, 6, "testClockTick" ) {}
 void runTest() { suite_ClockTest.testClockTick(); }
} testDescription_suite_ClockTest_testClockTick;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
