//=========================================================
// main.cpp
//
// Program main function that demonstrates the Observer
// Design pattern.
//
// In this program:
//
// ConcreteSubject class: ClockTimer
// ConcreteObserver class: DigitalClock
//         AnalogClock
//
// The ClockTimer object changes state by memeber function
// Tick(). This will trigger Update() function for both
// ConcreteObserver DigitalClock and AnalogClock. For demo,
// purpose, the observer objects will print out the current
// time in respond.
//
//=========================================================
#include "ClockTimer.h"
#include "DigitalClock.h"
#include "AnalogClock.h"

int main(void)
{
  ClockTimer timer;

  DigitalClock digitalClock;
  ClockObserver::aspectof ()->addObserver (&timer, &digitalClock);

  AnalogClock analogClock;
  ClockObserver::aspectof ()->addObserver (&timer, &analogClock);
       
  timer.Tick(); // subject state change and update all it's
                // observers which are digitalClock and
                // analogClock in this program
  timer.Tick(); // and again

  return 0;
}
