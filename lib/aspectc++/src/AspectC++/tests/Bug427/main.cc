#include <stdio.h>

#ifndef __GNUC__
#include <inttypes.h>
#endif

namespace Bug427 {

#ifdef __GNUC__
// typedefs from avr's <stdint.h>
typedef unsigned int uint8_t __attribute__((__mode__(__QI__)));
typedef unsigned int uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int uint32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int uint64_t __attribute__((__mode__(__DI__)));
#endif

typedef uint64_t UInt64;

// one argument with 8 bits
void foo(uint8_t x) { /* do something */ }

class A {
  uint8_t a;
  uint16_t b;
  uint32_t c;
  uint64_t d;
};

} // namespace Bug427

int main(){
  printf("Bug427: args() keyword ignores __attribute__ on types\n");
  printf("====================================================================\n");
  Bug427::foo(42);
  printf("sizeof a: %u\n", sizeof(AC::TypeInfo<Bug427::A>::Member<0>::Type));
  printf("sizeof b: %u\n", sizeof(AC::TypeInfo<Bug427::A>::Member<1>::Type));
  printf("sizeof c: %u\n", sizeof(AC::TypeInfo<Bug427::A>::Member<2>::Type));
  printf("sizeof d: %u\n", sizeof(AC::TypeInfo<Bug427::A>::Member<3>::Type));
  printf("====================================================================\n");
  
  return 0;
}

aspect ShadowAspect {
  advice execution("% ...::%(...)" && !"% main(...)") : around() {
    tjp->proceed();
    //printf("%s\n", tjp->signature()); // may be not portable
    printf("sizeof arg<0>: %u\n", sizeof(*tjp->arg<0>()) );
    typedef JoinPoint::template Arg<0>::Type ARG0;
    printf("sizeof ARG<0>: %u\n", sizeof( ARG0 ));
  }
};

// aspect that must not match
aspect BuggyAspect {
  advice execution("void Bug427::foo(%)") && args(x) : before(Bug427::UInt64 x) {
    printf("must not match: %s\n", tjp->signature());
  }
};
