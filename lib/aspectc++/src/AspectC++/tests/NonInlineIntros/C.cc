#include <stdio.h>

#include "C.h"
#include "D.h"
#include "E.h"
#include "F.h"

void C::f () { printf ("void C::f()\n"); }

void D::f () { printf ("void D::f()\n"); }

int main () {
  C c;
  D d;
  E e;
  printf ("NonInlineIntros: Checks location of non-inline introductions\n");
  printf ("============================================================\n");
  printf ("calling normal functions:\n");
  c.f ();
  d.f ();
  e.f ();
  F::f ();
  printf ("calling introduced functions:\n");
  c.g ();
  d.g ();
  F f;
  f.g ();
  printf ("accessing introduced static members:\n");
  printf ("C::pi is %f\n", C::pi);
  printf ("D::pi is %f\n", D::pi);
  printf ("F::pi is %f\n", F::pi);
  printf ("============================================================\n");
}

// here the non-inline intros for C and D should be added ...
