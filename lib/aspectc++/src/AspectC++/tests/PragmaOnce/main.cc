#include <stdio.h>

// this should not result in an error
#include "a.h"
#include "b.h"

int main () {
  C c;
  c._var = 0; // to avoid warnings
  printf ("PragmaOnce: check how the include expander handles pragma once\n");
  printf ("==============================================================\n");
  printf ("Everything is fine if it compiles without warnings or errors\n");
  printf ("==============================================================\n");
  return 0;
}
