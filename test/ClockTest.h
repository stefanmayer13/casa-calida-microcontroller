#include <cxxtest/TestSuite.h>

class ClockTest : public CxxTest::TestSuite
{
public:
    void testClockTick(void)
    {
        TS_ASSERT(1 + 1 > 1);
        TS_ASSERT_EQUALS(1 + 1, 2);
    }
};