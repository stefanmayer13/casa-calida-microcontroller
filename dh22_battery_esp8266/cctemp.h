//
//    FILE: cctemp.h
//  AUTHOR: Stefan Mayer
// VERSION: 0.1.1
// PURPOSE: Retreives humidy, temperature in °C and battery level via Serial from Attiny85
//

#ifndef cctemp_h
#define cctemp_h

#include <string>

class cctemp
{
public:
    cctemp() {};

	bool parseData(std::string data);
	float humidity, temp_c, battery;
};
#endif