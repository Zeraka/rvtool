#include <stdio.h>
#include "a.h"

void A::f () {
  printf ("A::f() running\n");
}

void A::g () {
  f ();
}
