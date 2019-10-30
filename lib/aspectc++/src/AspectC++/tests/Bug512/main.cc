#include <stdio.h>

#define __B__ b
struct C { void b() {} };
C a() { return C (); }
#define CALL a().__B__()
 
void hal_thread_switch_context() {}

#define HAL_THREAD_SWITCH_CONTEXT()                    \
  hal_thread_switch_context();
        
int main() {
  printf ("Bug512: advice for macro-generated call not woven\n");
  printf ("=================================================\n");
  HAL_THREAD_SWITCH_CONTEXT(); // no call advice with Bug 512 :(
  hal_thread_switch_context(); // call advice is ordinarily woven
  CALL;
  printf ("=================================================\n");
}

aspect CallAdvisor {
  advice call("% ...::%(...)" && !"% printf(...)") : before() {
    printf ("BEFORE: %s\n", JoinPoint::signature());
  }
};
