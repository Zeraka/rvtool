#include <stdio.h>

void f () {}

namespace {
}

namespace {
  class C {
  public:
    void cfunc() { intro++; }
    static const C &staticcfunc (C &obj) { obj.cfunc(); return obj; }
  } cobj;
  void g1 () { f (); cobj.cfunc(); }
  namespace {
    void g2 () { f (); g1 (); }
    namespace {
      void g3() {}
    }
  }
};

namespace {
}

namespace FooBar {
	enum MyEnum {};

	namespace FooBar2 {
		class C {};
	}
}

namespace {

class A
{
public:
    class B
    {
    public:
        FooBar::MyEnum f(FooBar::FooBar2::C&)
        {
          return FooBar::MyEnum ();
        }
    };

}; // class

} // local namespace

aspect Test {
  advice call("% ...::%()") : before () {
    printf ("    call (\"%s\")\n", JoinPoint::signature ());
  }
  advice call("% ...::%(...)") && within("% ...::g2(...)") : before () {
    printf ("    call within g2() to (\"%s\")\n", JoinPoint::signature ());
  }
  advice execution ("% ...::%(...)") : before () {
    printf ("  execution (\"%s\")\n", JoinPoint::signature ());
  }
  advice execution ("% ...::<unnamed>::...::%(...)") : before () {
    printf ("  in unnamed namespace\n");
  }
  advice "<unnamed>::C" : slice struct { int intro; };
};

int main() {
  printf ("AnonymousStuff: tests if ac++ can deal with anonymous classes,\n");
  printf ("                namespace, etc.\n");
  printf ("=============================================================\n");
  g1 ();
  g2 ();
  C c;
  C::staticcfunc(c);
  FooBar::FooBar2::C x;
  A::B ab;
  ab.f (x);
  printf ("=============================================================\n");
  return 0; // TODO: it should also work without this return statement
}
