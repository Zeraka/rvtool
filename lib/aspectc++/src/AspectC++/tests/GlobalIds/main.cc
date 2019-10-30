#include <stdio.h>
#include "h1.h"

extern void another_function ();

int main () {
  f ();
  another_function ();
  printf ("===================================================\n");
}
