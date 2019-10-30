#include <stdio.h>

aspect anotherBaseAspect {
  pointcut methods() = execution("% anotherClass::%(...)");

  advice methods() : before() {
    printf("ENTRY: %s\n",JoinPoint::signature());
  }
  advice methods() : around() {
    printf("ENTRY: %s\n",JoinPoint::signature());
    tjp->proceed ();
  }
};

class anotherClass {
public:
  anotherClass();

private:
  void methodA();
  void methodB();
};

void anotherClass::methodB () {}

int main () {
  printf ("Bug207: ac++ mixes up private and public\n");
  printf ("==============================================================\n");
  printf ("Check if methodA and B are still private after weaving\n");
  printf ("==============================================================\n");
  return 0;
}
