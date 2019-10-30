#include <stdio.h>

class SomeBase1 {};
class SomeBase2 {};
class SomeBase3 {};

namespace Puma {

  class XXX {
  };

  slice class ACSemanticForC : public SomeBase2 {
    bool _in_advice;
    class Nested;
    struct Nested3 {
      static void nf() {}
    };
public:
    static int anon;
    ACSemanticForC () : _in_advice (false) {}
    explicit ACSemanticForC (bool a);
    virtual ~ACSemanticForC ();
    void enter_advice () {
      _in_advice = true;
      Nested3::nf ();
      Nested3::here_I_am();
    }
    void enter_advice2 () {
      _in_advice = true;
    }
    bool in_advice () const { return _in_advice; }
    void leave_advice ();
};

  slice class ACBuilder;
}

slice class IntoNested3 {
  struct Nested4 {};
public:
  static void here_I_am() {}
};

slice class IntoNested4 {
  struct Nested3 {};
  static void here_I_am() {}
};

slice class Puma::ACBuilder;
slice class Puma::ACBuilder {};

slice struct InGlobal {
  int _in_global;
  void global ();
};

class WithBase : public SomeBase1 {};

namespace Puma {
  slice class Bla;
}

int main () {
  printf ("Slice: of few simple slice implementations\n");
  printf ("==========================================\n");
  Puma::XXX x(true);
  printf ("in: %d\n", x.in_advice ());
  x.enter_advice ();
  printf ("in: %d\n", x.in_advice ());
  x.leave_advice ();
  printf ("in: %d\n", x.in_advice ());
  x.g (); // from DefinedLater
  printf ("==========================================\n");
};

slice class DefinedLater;

aspect Foo {
  // advice "Puma::XXX" : baseclass (SomeBase1);
  advice "Puma::XXX" : slice class U : public SomeBase3;
  advice "WithBase"  : slice struct : SomeBase2;

  advice "Puma::XXX" : slice Puma::Bla;
  advice "...::Nested3" : slice IntoNested3;
  //  advice "...::Nested4" : slice IntoNested4; // doesn't work! Infinite intro recursion.

  slice class Local {
      typedef Local THIS;
  };
/*   advice "XXX" : slice class Bla; // wrong */
/*   advice "XXX" : slice struct ::Foo; // wrong */
  advice "Puma::XXX" : slice Puma::ACSemanticForC;
  advice "Puma::XXX" : slice InGlobal;

/*   advice "XXX" : { */
/*     void *blob; */
/*   }; */
  advice "Puma::XXX" : slice DefinedLater;
  slice struct Later2;
  advice "Puma::XXX" : slice Later2;
  advice "Puma::XXX" : slice class {}; // useless :-)
  advice "Puma::XXX" : slice class S {
    double murks;
    long long murks2;
  public:
//     S () {}
    explicit S(int) {}
    explicit S(double i) : murks(i) {}
    S(unsigned long long ll) : murks2(ll) {}
    int bar2;
    class Nested2;
  };
};

slice class Puma::Bla {
  void func ();
};

slice void Puma::Bla::func () {
}

slice struct Foo::Later2 { int _later2; };

slice class DefinedLater { int _def_later; public: void g (); };

slice void DefinedLater::g () {}

aspect Trace {
  advice execution (member ("Puma::XXX")) ||
         construction ("Puma::XXX") || destruction ("Puma::XXX") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
  advice call ("% ...::here_I_am()") : before () {
    printf ("call advice for introduced code works.\n");
  }
};

slice class Puma::ACSemanticForC::Nested {
  int i;
};

slice
void Puma::ACSemanticForC::leave_advice () { _in_advice = false; }


slice
int Puma::ACSemanticForC::anon = 0;

slice
class ::Foo::S::Nested2 {};

slice Puma::ACSemanticForC::~ACSemanticForC () {}

slice Puma::ACSemanticForC::ACSemanticForC (bool a) : _in_advice (a) {}

slice void InGlobal::global () {}
