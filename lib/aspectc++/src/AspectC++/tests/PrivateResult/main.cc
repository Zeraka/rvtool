#include <stdio.h>
#include <iostream>
using namespace std;

#include "Safe.h"

Safe f () {
  Safe safe (4711);
  return safe;
};

Safe g () {
  return f ();
}

AUnion h () {
  AUnion u;
  return u;
}

int main () {
  printf ("PrivateResult: advice for functions returning object with\n");
  printf ("               non-public default constructor/assignment op.\n");
  printf ("============================================================\n");
  Safe s(f ());
  g ();
  Safe s2 (1);
  printf ("------------------------------------------------------------\n");
  printf ("problem with private inner types:\n");
  Safe::Inner i = s2.uses_private_inner1 ();
  const Safe::Inner &r = s2.uses_private_inner2 ();
  printf ("------------------------------------------------------------\n");
  printf ("return a union:\n");
  AUnion u = h ();
  printf ("============================================================\n");
}
