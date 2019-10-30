//=========================================================
// ClockTimer.cpp
//
// Implement member functions in ClockTimer class which
// is in file ClockTimer.h
//=========================================================
#include "ClockTimer.h"

void ClockTimer::Tick() {
  // Obtain operating system-style time. (not portable, therefore faked)
  _sec = (_sec + 1) % 60;
  if (_sec == 0) {
    _min = (_min + 1) % 60;
    if (_min == 0) {
      _hour = (_hour + 1) % 24;
    }
  }
}
