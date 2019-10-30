#ifndef __A_h__
#define __A_h__

#include "C.h"

class A {
public:
  static void f() { C::f(); }
};


#endif // __A_h__
