/*
  Clock implements a simple clock for Arduino depending on the millis function.
  It will need regular synchronisation to stay in sync.
  Copyrigt (C) 2016  Stefan Mayer <stefanmayer13@gmail.com>
*/

#ifndef _TEST_
#include <Arduino.h>
#else
#include "mocks/mock_arduino.h"
#endif

#include "clock.h"

bool Clock::tick() {
  if (millis() - milliseconds >= 60000UL) {
    milliseconds += 60000UL;

    minute++;
    if (minute >= 60) {
      minute = 0;
      hour++;
    }
    if (hour >= 24) {
      hour = 0;
      day++;
    }
    if ((day == 28 && month == 2 && year % 4 != 0) || (day == 29 && month == 2) || 
        (day == 30 && (month == 4 || month == 6 || month == 9 || month == 11)) || day >= 31) {
      month++;
      day = 1;
    }
    if (month >= 13) {
      year++;
      month = 1;
    }
    return true;
  }
  return false;
}

void Clock::setTime(int new_year, int new_month, int new_day, int new_hour, int new_minute) {
  year = new_year;
  month = new_month;
  day = new_day;
  hour = new_hour;
  minute = new_minute;
}

int Clock::getMinute() {
  return minute;
}

int Clock::getHour() {
  return hour;
}

int Clock::getDay() {
  return day;
}

int Clock::getMonth() {
  return month;
}

int Clock::getYear() {
  return year;
}