#include <stdio.h>
//Extracted from memcached:
//------------------------

#define bool bool
#define true true

class C {
  void f();
};
void C::f() {}


slice class Foo {
public:
  bool enter();
};
slice bool Foo::enter() { return true; }


aspect Bar {
  advice "C" : slice Foo;
  advice execution("% C::%(...)") : before() {}
};


int main() {
  C c;
  printf ("enter: %s\n", c.enter() ? "true":"false");
  return 0;
}
