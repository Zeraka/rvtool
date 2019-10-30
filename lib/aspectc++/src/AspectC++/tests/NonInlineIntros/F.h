#ifndef __F_h__
#define __F_h__

#include <stdio.h>

class F {
  int a;
  int b;
public:
  template <int I> void a_template_function ();

  static void f () { printf ("static void F::f()\n"); }

  static const int ci = 12;
  static int nci;
};

// this should no be considered as the link-once code element
template <int I>
void F::a_template_function () {}

#endif // __F_h__
