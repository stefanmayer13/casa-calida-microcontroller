#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <string>
#include "mock_arduino.h"
#include "../cctemp.h"

void millis_test() {
  unsigned long start = millis();
  std::cout << "millis() test start: " << start << std::endl;
  while( millis() - start < 10000 ) {
    //std::cout << millis() << std::endl;
    usleep(1);
  }
  unsigned long end = millis();
  std::cout << "End of test - duration: " << end - start << "ms" << std::endl;
}

void delay_test() {
  unsigned long start = millis();
  std::cout << "delay() test start: " << start << std::endl;
  while( millis() - start < 10000 ) {
    //std::cout << millis() << std::endl;
    delay(250);
  }
  unsigned long end = millis();
  std::cout << "End of test - duration: " << end - start << "ms" << std::endl;
}

void cctemp_parseData_test() {
  std::cout << "Testing parse data...";
  cctemp temp;
  
  std::string test = "a";
  bool received = temp.parseData(test);
  assert(received==false);
  std::cout << "succesful" << std::endl;
}

void run_tests();

int main(int argc, char **argv){
  initialize_mock_arduino();
  std::cout << "Starting..." << std::endl;
  run_tests();
}

void run_tests() {
  //millis_test();
  //delay_test();
  cctemp_parseData_test();
}