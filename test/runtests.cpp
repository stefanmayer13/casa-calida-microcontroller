#include "mocks/mock_arduino.h"
#include "clock.h"
#include <unistd.h>

using namespace std;

void clock_test() {
  Clock Clock1;
  Clock1.setTime(2016, 1, 1, 10, 30);
  cout << Clock1.getHour() << ":" << Clock1.getMinute() << endl;
  usleep(60000);
  Clock1.tick();
  cout << Clock1.getHour() << ":" << Clock1.getMinute() << endl;
}

int main(int argc, char **argv){
  initialize_mock_arduino();
  clock_test();
}
