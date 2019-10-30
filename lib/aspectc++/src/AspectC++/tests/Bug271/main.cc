#include <stdio.h>

class MyNS {
public:
  class FOO1 {
  public:
    int d();
  };
  
  class FOO2 {
  public:
    int d();
  };
};

int MyNS::FOO1::d() { return 1; }
int MyNS::FOO2::d() { return 2; }

class FOOBAR {
protected:
   void fooBarMethod();
};

class FOO : protected FOOBAR{
private:
  int disc;
  MyNS::FOO1 *obj1;
  MyNS::FOO2 *obj2;
public:
  FOO () {}
  FOO(MyNS::FOO1 *obj);
  FOO(MyNS::FOO2 *obj);
  int d();
};

FOO::FOO(MyNS::FOO1* obj) : obj1(obj), obj2(0), disc(obj->d()) {}
FOO::FOO(MyNS::FOO2* obj) : obj1(0), obj2(obj), disc(obj->d()) {}

class BAR : FOO {
public:
 void barMethod();
};

void FOOBAR::fooBarMethod() {}

void BAR::barMethod() { fooBarMethod(); }

aspect Trace {
  advice target("%") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
};

int main () {
  printf ("Bug271: access control problem with privately inherited func\n");
  printf ("============================================================\n");
  BAR bar;
  bar.barMethod();
  MyNS::FOO1 foo1;
  FOO foo_a (&foo1);
  MyNS::FOO2 foo2;
  FOO foo_b (&foo2);
  printf ("============================================================\n");
  return 0;
}
