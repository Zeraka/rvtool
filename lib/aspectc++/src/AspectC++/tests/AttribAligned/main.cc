#include <stdio.h>

#ifdef __GNUC__
typedef char more_aligned_char __attribute__ ((aligned (8)));
#else
typedef struct X { char x[8]; X(){} X(int){} } more_aligned_char;
#endif

void foo(more_aligned_char x) {}

int main() {
  foo(0);
  return 0;
}

class T1 { more_aligned_char t1, t2; }; // for reference

class T3 { more_aligned_char t1; };

aspect AlignAspect {
  advice "T3" : slice class { JoinPoint::Member<0>::Type t2; }; // test Introspection

  advice execution("void foo(%)") : before() {
    *tjp->arg<0>(); // needed somehow ...

#ifdef __GNUC__
    class T2 { JoinPoint::template Arg<0>::Type t1, t2; }; // test JoinPoint API
    // ... ignoring attributes on template argument ...
#else
    class T2 { char t1, t2 };
#endif
    
    printf("sizeof T1(%u), T2(%u), T3(%u)\n", sizeof(T1), sizeof(T2), sizeof(T3));
  }
};

