#include <cstdio>
using std::printf;

#define XSTR(s) STR(s)
#define STR(s) #s
#define ARGDECL int i
#define BAR bar
#define INT int

aspect Calls {
  advice call ("% _foo(...)") : before () {
    printf ("before foo() call -> ");
  }
  advice execution ("% " XSTR(BAR) "(...)") : before () {
    printf ("before bar() execution -> ");
  }
  advice call ("% qInitResources_%(...)") : before () {
    printf ("before qInitResources_%% call -> ");
  }
};

#define VOID void

#define FOODEF \
VOID _foo (int, float) { \
  printf ("in foo()\n"); \
}

FOODEF

VOID BAR (INT i) {
  printf ("in bar()\n"); \
}

#define ARG 3+3

#define foo _foo

#define QT_MANGLE_NAMESPACE(name) name

#define Q_INIT_RESOURCE(name) \
   do { extern int QT_MANGLE_NAMESPACE(qInitResources_ ## name) ();       \
       QT_MANGLE_NAMESPACE(qInitResources_ ## name) (); } while (0)

int qInitResources_bar () {
  printf ("in qInitResources_bar\n");
  return 0;
}

#define EXPECT_FALSE(exp)        __builtin_expect((exp),false)

class A {
public:
  int test() { return 0; }
  static A* getInstance() { static A inst; return &inst; }
};

int test() {
  if (EXPECT_FALSE(A::getInstance()->test())) { return 1; }
  return 0;
}

aspect A_call {
  advice call("% A::%(...)") : before() {}
};

int main () {
  printf ("MacroCall:\n");
  printf ("=============================================================\n");
  printf ("call to foo() -> ");
  foo (ARG,ARG);
  printf ("call to bar() -> ");
  bar (ARG);
  printf ("=============================================================\n");
  printf ("call to qInitResources_foo() -> ");
  Q_INIT_RESOURCE(foo); // should not match.
  printf ("=============================================================\n");
  printf ("call to qInitResources_bar() -> ");
  Q_INIT_RESOURCE(bar); // should match.
  printf ("=============================================================\n");
  return 0;
}
