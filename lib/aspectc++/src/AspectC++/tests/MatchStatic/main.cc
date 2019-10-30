#include <stdio.h>

struct Test {
  static void static_func() {}
  void non_static_func () {}
  virtual void virtual_func () {}
};

static void static_func() {}
void non_static_func() {}

aspect Trace {
  pointcut excl() = call ("% printf(...)");
  advice !excl() && call("% ...::%(...)") : before () {
    printf ("call to %s\n", JoinPoint::signature ());
  }
  advice !excl() && call("static % ...::%(...)") : before () {
    printf ("  STATIC function\n");
  }
  advice !excl() && call("% ...::%(...)") && !call ("static % ...::%(...)"): before () {
    printf ("  NON-STATIC function\n");
  }
};

int main () {
  printf ("Check the match mechanism for 'static' in match expressions\n");
  printf ("============================================================\n");
  Test test;
  test.static_func ();
  Test::static_func ();
  test.non_static_func ();
  test.virtual_func ();
  printf ("------------------------------------------------------------\n");
  static_func ();
  non_static_func ();
  printf ("============================================================\n");
  return 0;
}
