#include <stdio.h>

class FOOBAR {
protected:
  virtual void fooBarMethod();
  void other () {}
};

class FOO : protected FOOBAR{
public:
  friend int main ();
  void fooBarMethod () { other(); } // virtual function, too
};

class BAR : FOO {
public:
  void barMethod();
  void fooBarMethod(); // virtual, but not declared as virtual
  void fooBarMethod(int); // same name, but not virtual
};

void FOOBAR::fooBarMethod() {}

void BAR::fooBarMethod() {}
void BAR::fooBarMethod(int) {}

void BAR::barMethod() { fooBarMethod(); fooBarMethod (42); }

aspect Trace {
  pointcut excl() = call ("% printf(...)");
  advice !excl() && call("% ...::%(...)") : before () {
    printf ("call to %s\n", JoinPoint::signature ());
  }
  advice !excl() && call("virtual % ...::%(...)") : before () {
    printf ("  VIRTUAL method\n");
  }
  advice !excl() && target("%") && !call ("virtual % ...::%(...)"): before () {
    printf ("  NON-VIRTUAL method\n");
  }
};

int main () {
  printf ("Check the match mechanism for 'virtual' in match expressions\n");
  printf ("============================================================\n");
  BAR bar;
  bar.barMethod();
  FOO *fooptr = new FOO;
  fooptr->fooBarMethod ();
  delete fooptr;
  printf ("============================================================\n");
  return 0;
}
