#include <stdio.h>

struct Complex {
  double r;
  double i;
};

class Test {
  const char *_name;
public:
  Test (const char *n) : _name (n) {}
  const char *name () const { return _name; }
  void f (int i, Complex c) {}
  void run () {
    Complex c = { 2.0, .5 };
    f (10, c);
  }
};

aspect ContextTest {
  pointcut _args(Complex _c, int _i) = args (_i, _c);
  advice _args (c, i) && that (obj) : before (int i, Test *obj, Complex c) {
    printf ("that: %s, int %d, Complex(%lf,%lf)\n", obj->name (), i, c.r, c.i);
  }
  advice args (i, c) && that (obj) : before (Test *obj, Complex c, int i) {
    printf ("that: %s, int %d, Complex(%lf,%lf)\n", obj->name (), i, c.r, c.i);
  }
};


int main () {
  Test t ("main()::t");
  printf ("Context information is requested by advice in arbitrary order:\n");
  printf ("==============================================================\n");
  t.run ();
  printf ("==============================================================\n");
}
