#include <stdio.h>
#include <string.h>

struct S {
	int i;
	int j;
	S () {}
	void g () const {}
};

void f1 (S a[1]) {}
void f2 (S *a) {}
void f3 (S (&a)[2]) {}

class C {
public:
  C () {}
  S a[2];
  char b[10];
  union {
    short shortVal;
    int intVal;
    char* stringVal;
  };
  union {
    int blibs;
    double blubs;
  };
  char *str () const { return (char*)b; }
};

aspect TestCopyConstructorGeneration {
  advice construction ("C") : around () {
    printf ("before %s\n", JoinPoint::signature ());
    tjp->proceed ();
    printf ("after %s\n", JoinPoint::signature ());
  }
};

int main () {
  printf ("ArrayWrapper: tests if wrapped arrays behave like arrays\n");
  printf ("========================================================\n");
  const C cc;
  C c (cc);
  cc.a[0];
  &cc.a[0];
  c.a[0];
  &c.a[0];
  const S *p = 0;
  p = &c.a[1l];
  c.a[1] = c.a[0];
  c.a[0] = cc.a[0];
  c.a[0].g ();
  // error ...
  // cc.a[0] = c.a[0];
  memcpy (c.a, cc.a, sizeof (c.a));
  f1 (c.a);
  f2 (c.a);
  // another error
  // f3 (cc.a);
  f3 (c.a);
  &c.a;
  printf ("========================================================\n");
  return 0;
}

