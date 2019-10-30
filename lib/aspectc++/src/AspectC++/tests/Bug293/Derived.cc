#include "Derived.h"
#include "Callee.h"

extern CalleeBase *callee_ptr;

void Base::f () {
  callee_ptr->callee();
}
