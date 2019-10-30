#include <stdio.h>
#include "introTarget.h"

int meine_funktion(int i) {
  printf("Hallo\n");
  return i + 3;
}

int main() {
  printf ("Bug 234: Types from the aspect-header in introductions\n");
  printf ("======================================================\n");
  introTarget bumsbla;

  bumsbla.set_a_func(meine_funktion);
  bumsbla.introTargetMethod(5);
  printf ("======================================================\n");
}
