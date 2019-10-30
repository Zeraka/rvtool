#ifndef ANALOGCLOCK_H
#define ANALOGCLOCK_H

//=========================================================
// ConcreteObserver class
//
// AnalogClock
//
// It is a ConcreteObserver class and is a child class of
// the Observer class.
//
// It is a analog clock observer. It attaches to a timer
// object of the type Subject at run time. The timer
// object calls AnalogClock member function Update()
// whenever there is a AnalogClock update required.
// AnalogClock obtain current time from the timer object
// by timer object's member functions GetHour(),
// GetMinutes, and GetSecond(). AnalogClock prints out
// the updated time.
//
//=========================================================
class ClockTimer;

class AnalogClock {
public:
  void Draw(const ClockTimer &); // print updated time
};

#endif
