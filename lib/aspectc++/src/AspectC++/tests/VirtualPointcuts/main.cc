#include <stdio.h>

void a() {}
void b() {}

int main() {
  printf ("VirtualPointcuts: checks virtual pointcut lookup\n");
  printf ("================================================\n");
  a();
  b();
  printf ("================================================\n");
}

aspect Base {
  pointcut virtual f() = "void a()";
  pointcut virtual g() = 0;

  advice execution(f()) : before() {
    printf ("Advice on virtual pct 'f()': %s\n", JoinPoint::signature());
  }
  advice execution(g()) : before() {
    printf ("Advice on pure virtual pct 'g()': %s\n", JoinPoint::signature());
  }
};

aspect Derived : public Base {
  // should override, but doesn't:
  pointcut virtual f() = "void b()";

  // works
  pointcut virtual g() = "void b()";
};
