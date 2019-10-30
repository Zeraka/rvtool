#include <stdio.h>

#include "C.h"
#include "A.h"

int main () {
  C c;
  A a;
  printf ("NonInlineSliceInclude: check for include generation in .cc\n");
  printf ("==========================================================\n");
  c.f ();
  a.f1 ();
  a.f2 ();
  printf ("==========================================================\n");
}
