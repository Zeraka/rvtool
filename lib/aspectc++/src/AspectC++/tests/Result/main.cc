#include <stdio.h>

void f () {}
int g () { return 0; }
void *h () { return (void*)1; }
int *i () { return (int*)2; }
long **j () { return (long**)3; }
class C {
public:
  operator int * () const { return (int*)4; }
};

int v1 = 4711;

// this is changed by advice to return something else instead of v1!
const int & select_v () { return v1; }

int main () {
  printf ("Result: checks if the result pointcut function works\n");
  printf ("====================================================\n");
  f ();
  g ();
  h ();
  i ();
  j ();
  C c;
  int *i;
  i = c; // TODO: call to operator int*
  printf ("  changed result reference refers to: %d\n", select_v ());
  printf ("====================================================\n");
  return 0;
}
