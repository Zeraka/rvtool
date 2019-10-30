#include <stdio.h>

#include "Mem.h"

int main() {
  printf ("Bug552: repository-merge crashes ...\n");
  printf ("====================================\n");
  printf ("no output, fine if it compiles without crashing\n");
  Mem::memset_mwords(0, 42, 4711);
  return 0;
}
