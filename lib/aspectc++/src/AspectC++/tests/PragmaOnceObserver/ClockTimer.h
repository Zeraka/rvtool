#pragma once

// include files used to obtain the current system time
#include <time.h>

//=========================================================
// ConcreteSubject class
//
// ClockTimer
//
// It is a ConcreteSubject class and is a child class of
// the Subject class.
//
// It is a time observer. Whenever member function Tick()
// is called. It obtains the current system
// time and store it as a character string. It called
// the update function for all of its observer objects.
//
//=========================================================

class ClockTimer {
  // store system time
  int _hour;
  int _min;
  int _sec;
public:
  ClockTimer() {
    // normally we would get the real local system time here
    _hour = 12;
    _min  = 58;
    _sec  = 48;
  };
  // get hour (used by observer)
  int GetHour() const { return _hour; }
  // get minute (used by observer)
  int GetMinute() const { return _min; }
  // get second (used by observer)
  int GetSecond() const { return _sec; }

  void Tick();  // obtian system time (change state)
};
