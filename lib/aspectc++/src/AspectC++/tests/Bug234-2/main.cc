#include "stdio.h"
#include "introTarget.hxx"

int this_func(int i) {
  printf("int this_func(%d)\n", i);
  return i + 3;
}

int main() {
  printf ("Bug234-2: another test for Bug234\n");
  printf ("=================================\n");
  introTarget bumsbla;

  bumsbla.set_a_func(this_func);
  bumsbla.introTargetMethod(5);
  printf ("=================================\n");
}
