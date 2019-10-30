//=========================================================
// AnalogClock.cpp
//
// Implement member functions in AnalogClock class which
// is in file AnalogClock.h
//=========================================================
#include "AnalogClock.h"
#include "ClockTimer.h"
#include <iostream>

void AnalogClock::Draw (const ClockTimer &clock)
{
  int hour   = clock.GetHour();
  int minute = clock.GetMinute();
  int second = clock.GetSecond();

  std::cout << "Analog Clock time is " << hour << ":"
	    << minute << ":"
	    << second << std::endl;
}
