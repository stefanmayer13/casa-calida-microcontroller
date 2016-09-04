#include <cxxtest/TestSuite.h>
#include "clock.h"
#include <unistd.h>

class ClockTest : public CxxTest::TestSuite
{
public:
    void testClockTick(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 10, 30);
  		usleep(60000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 31);
    }
};