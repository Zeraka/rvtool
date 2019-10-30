#ifndef __E_h__
#define __E_h__

#include <stdio.h>

class E {
  int a;
  int b;
public:
  template <int I> void a_template_function ();

  void f () { printf ("void E::f()\n"); }
};

// this should no be considered as the link-once code element
template <int I>
void E::a_template_function () {}

#endif // __E_h__
