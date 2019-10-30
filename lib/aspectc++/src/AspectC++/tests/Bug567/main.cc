#include <stdio.h>

struct FooBar {
  int z;
  static int y;
  void f() { y = 6; z = y; }
  static void g() { y = 5; }
};

int FooBar::y;

struct BarFoos : public FooBar {};

int main() {
  printf ("Bug567: target() pointcut function defect for get/set advice\n");
  printf ("============================================================\n");
  FooBar b;
  b.z = 3;
  b.f();

  FooBar::y = 4;
  FooBar::g();

  BarFoos f;
  f.z = 7;
  printf ("============================================================\n");
}

aspect MemberTrace {
  advice call("% %::...::%(...)") && target(!"void") : before() {
    printf ("call/target %s\n", JoinPoint::signature());
  }

  advice set("% %::...::%") && target(bf) : before(BarFoos* bf) {
    printf ("set/target %s\n", JoinPoint::signature());
  }

  advice set("% %::...::%") && that("%") : before() {
    printf ("set/that %s\n", JoinPoint::signature());
  }

  advice get("% %::...::%") && that("%") : before() {
    printf ("get/that %s\n", JoinPoint::signature());
  }

  advice set("% %::...::%") && args(i) : before(int i) {
    printf ("set/args %s %d\n", JoinPoint::signature(), i);
  }

  advice get("% %::...::%") && result(i) : after(int i) {
    printf ("get/result %s %d\n", JoinPoint::signature(), i);
  }
};
