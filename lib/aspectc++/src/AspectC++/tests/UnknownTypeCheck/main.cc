#include <stdio.h>

#include "b.h"

class Unrelated {
public:
  void f () {
    printf ("void Unrelated::f()\n");
  }
};

int main () {
  B b;
  A a;
  Unrelated u;
  printf ("UnknownTypeCheck: dynamic check for an unknown type\n");
  printf ("===============================================================\n");
  printf ("with advice (invocation on B object):\n");
  b.g ();
  printf ("---------------------------------------------------------------\n");
  printf ("without advice (invocation on A object):\n");
  a.g ();
  printf ("---------------------------------------------------------------\n");
  printf ("without advice (invocation on Unrelated object):\n");
  u.f ();
  printf ("===============================================================\n");
  return 0;
}
