#ifndef DIGITALCLOCK_H
#define DIGITALCLOCK_H

//=========================================================
// ConcreteObserver class
//
// DigitalClock
//
// It is a ConcreteObserver class and is a child class of
// the Observer class.
//
// It is a digital clock observer. It attaches to a timer
// object of the type Subject at run time. The timer
// object calls DigitalClock member function Update()
// whenever there is a DigitalClock update required.
// DigitalClock obtain current time from the timer object
// by timer object's member functions GetHour(),
// GetMinutes, and GetSecond(). DigitalClock prints out
// the updated time.
//
//=========================================================
class ClockTimer;

class DigitalClock {
public:
  void Draw(const ClockTimer &); // print updated time
};

#endif
