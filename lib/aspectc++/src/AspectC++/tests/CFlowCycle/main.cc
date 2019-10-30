#include <stdio.h>

#include "D.h"
#include "A.h"

int main() {
  printf ("CFlowCycle: cflow generation and cycle problem check\n");
  printf ("====================================================\n");
  A::f ();
  printf ("now calling C::f() directly from main():\n");
  C::f ();
  printf ("the execution advice shall print nothing above this line\n");
  printf ("====================================================\n");
}
