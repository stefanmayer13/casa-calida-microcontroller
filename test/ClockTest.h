#include <cxxtest/TestSuite.h>
#include "clock.h"
#include <unistd.h>

class ClockTest : public CxxTest::TestSuite
{
public:
	void testClockTickNoMinute(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 10, 30);
  		usleep(30000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 30);
        TS_ASSERT_EQUALS(Clock1.getHour(), 10);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTickMinute(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 10, 30);
  		usleep(60000);
  		Clock1.tick();
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 31);
        TS_ASSERT_EQUALS(Clock1.getHour(), 10);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTick2Minutes(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 10, 30);
  		usleep(120000);
  		Clock1.tick();
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 32);
        TS_ASSERT_EQUALS(Clock1.getHour(), 10);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTickHour(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 10, 59);
  		usleep(60000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 0);
        TS_ASSERT_EQUALS(Clock1.getHour(), 11);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTickDay(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 1, 23, 59);
  		usleep(60000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 0);
        TS_ASSERT_EQUALS(Clock1.getHour(), 0);
        TS_ASSERT_EQUALS(Clock1.getDay(), 2);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTickMonth(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 1, 31, 23, 59);
  		usleep(60000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 0);
        TS_ASSERT_EQUALS(Clock1.getHour(), 0);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 2);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2016);
    }

    void testClockTickYear(void)
    {
    	Clock Clock1;
  		Clock1.setTime(2016, 12, 31, 23, 59);
  		usleep(60000);
  		Clock1.tick();
        TS_ASSERT_EQUALS(Clock1.getMinute(), 0);
        TS_ASSERT_EQUALS(Clock1.getHour(), 0);
        TS_ASSERT_EQUALS(Clock1.getDay(), 1);
        TS_ASSERT_EQUALS(Clock1.getMonth(), 1);
        TS_ASSERT_EQUALS(Clock1.getYear(), 2017);
    }
};