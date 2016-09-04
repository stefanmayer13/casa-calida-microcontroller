/*
  Clock implements a simple clock for Arduino depending on the millis function.
  It will need regular synchronisation to stay in sync.
  Copyrigt (C) 2016  Stefan Mayer <stefanmayer13@gmail.com>
*/

class Clock {
protected:
  unsigned long milliseconds;
  int year = 2016, month = 1, day = 1, hour = 0, minute = 0;
  void write_frame();

public:
  bool tick();
  void setTime(int new_year, int new_month, int new_day, int new_hour, int new_minute);
  int getMinute();
  int getHour();
  int getDay();
  int getMonth();
  int getYear();
};