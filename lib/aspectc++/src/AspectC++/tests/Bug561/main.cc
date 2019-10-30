//#include <stdio.h>

#define GEN_CLASS(name) class Class_ ## name { \
public: \
 inline void stuff () const; \
  }; \
  inline void Class_ ## name :: stuff () const { intro (); }


GEN_CLASS(C);

#define FOO_GEN(func_name)       \
class func_name ## _Foo {        \
public:                          \
        static void doStuff();       \
};                               \
inline void func_name ## _Foo::doStuff()

FOO_GEN(A) {
        A_Foo instance(5);
        instance.print();
}

// .. and now more complicated:

#define BAR_GEN(func_name, ret_type, ...)       \
class func_name ## _Bar {                       \
public:                                         \
	static ret_type doStuff(__VA_ARGS__);       \
};                                              \
inline ret_type func_name ## _Bar::doStuff(__VA_ARGS__)

#define IFACE(GEN, ...) \
	GEN(__VA_ARGS__)

#define SIGNATURE A, int, int a1, int a2, int a3

IFACE(BAR_GEN, SIGNATURE) {
	int sum = a1 + a2 + a3;
	A_Bar instance(sum);
	instance.print();
	return sum;
}

int main () {
  Class_C c;
  printf ("Bug561: intro into macro-generated class\n");
  printf ("========================================\n");
  c.intro ();
  c.stuff ();
  A_Foo::doStuff ();
  A_Bar::doStuff(1,2,3);
  printf ("========================================\n");
}

aspect A {
  advice "Class_C" : slice class {
  public :
    void intro ( ) const { }
  };
  advice call("% Class_C::%(...)" || "% A_Foo::%(...)" || "% A_Bar::%(...)") : before () {
    printf ("called %s\n", JoinPoint::signature ());
  }
};

#include "slice.ah"

aspect Test_Aspect {
  advice "%_Foo" || "%_Bar" : slice Test_Slice;
};
