#include <stdio.h>

void f (int *a) { printf ("non const\n"); }
// void f (const int *a) { printf ("const\n"); }

int main () {
  const int x[3] = { 1, 2, 3 };
  f (x);
  return 0;
}