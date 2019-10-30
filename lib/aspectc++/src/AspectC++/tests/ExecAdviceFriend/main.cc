#include <stdio.h>

class something {
public:
  friend int myfunc(void *, int);
};

int myfunc(void *, int) {
  printf("myfunc() running\n");
  return 0;
}

aspect Trace {
public:
  advice execution("% ...::%(...)") : before() {
    printf ("before exec %s\n", JoinPoint::signature());
  }
};

int main() {
  printf ("ExecAdviceFriend: advice for func declared as a friend only\n");
  printf ("===========================================================\n");
  myfunc (0, 0);
  printf ("===========================================================\n");
  // problem: advice for main does not auto-generate return 0!
  return 0;
}
