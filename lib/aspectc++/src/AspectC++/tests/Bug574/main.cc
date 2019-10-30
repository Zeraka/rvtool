typedef int cyg_priority;
typedef bool cyg_bool;

class Cyg_SchedThread
    //: public Cyg_SchedThread_Implementation
{

    
protected:    
    cyg_priority        original_priority;      // our original priority

    cyg_bool            priority_inherited;     // have we inherited?

};

class Cyg_HardwareThread {};

class Cyg_Thread
    : public Cyg_HardwareThread,       // provides hardware abstractions
      public Cyg_SchedThread           // provides scheduling abstractions
{

public:
    cyg_priority get_priority();

};

inline cyg_priority Cyg_Thread::get_priority()
{
    if( priority_inherited ) return original_priority;
    else return 0;
}



int main() {
  Cyg_Thread t;
  t.get_priority();
  return 0;
}

#include <iostream>
using namespace std;

aspect GetterSetter {
  pointcut all_variables() = "% ...::%";

  advice get(all_variables()) || set(all_variables()) : before() {
    cout << tjp->signature() << endl;
  }

};

