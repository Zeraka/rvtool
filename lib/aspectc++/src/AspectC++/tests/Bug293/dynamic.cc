#include "Derived.h"
#include "Callee.h"

Base *base_ptr;
CalleeBase *callee_ptr;

void measurePoint() {
  base_ptr->f ();
}

int main () {
  Derived d;
  base_ptr = &d;
  Callee c;
  callee_ptr = &c;
  measurePoint ();
}
