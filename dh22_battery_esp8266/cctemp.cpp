//
//    FILE: cctemp.cpp
//  AUTHOR: Stefan Mayer
// VERSION: 0.1.1
// PURPOSE: Retreives humidy, temperature in °C and battery level via Serial from Attiny85
//

#include "cctemp.h"
#include <stdlib.h>

bool cctemp::parseData(std::string data)
{
  bool dataRetrieved = false;
  if(data.find("Humidity") == 0) {
    humidity = ::atof(data.substr(9).c_str());
  } else if(data.find("Temperature") == 1) {
    temp_c = ::atof(data.substr(13).c_str());
  } else if(data.find("Battery") == 1) {
    battery = ::atof(data.substr(9).c_str());
    dataRetrieved = true;
  }
  return dataRetrieved;
}