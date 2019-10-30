#include <cstdio>
using std::printf;

class C {
public:
  C() { foo(); }
private:
  class D { };
  enum E { E1 };

  E returner () { return E1; }
  void foo ();
  void foo (D *pD);
};

void C::foo () {
  D d;
  foo (&d);
  returner ();
}

void C::foo (D *pD) { }

aspect AspC {
 public:
  pointcut methods () = call ("% C::%(...)") || execution ("% C::%(...)");
  
  advice methods () : before () {
    printf ("before call to %s\n", JoinPoint::signature());
  }
};


int main () {
  printf ("Bug368: access control problem with private class\n");
  printf ("=================================================\n");
  C c;
  printf ("=================================================\n");
  return 0;
}
