#include <stdio.h>

void f1() {}
void f2();
void f3() {}

int main () {
  printf ("DisjointIds: checks whether JPID are disjoint\n");
  printf ("=============================================\n");
  f1 ();
  f2 ();
  f3 ();
  printf ("=============================================\n");
}
