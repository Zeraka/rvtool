//=========================================================
// DigitalClock.cpp
//
// Implement member functions in DigitalClock class which
// is in file DigitalClock.h
//=========================================================
#include "DigitalClock.h"
#include "ClockTimer.h"
#include <iostream>

void DigitalClock::Draw (const ClockTimer &clock)
{
  int hour = clock.GetHour();
  int minute = clock.GetMinute();
  int second = clock.GetSecond();

  std::cout << "Digital Clock time is " << hour << ":"
	    << minute << ":"
	    << second << std::endl;          
}
