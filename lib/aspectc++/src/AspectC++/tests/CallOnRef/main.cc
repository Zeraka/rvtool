#include <stdio.h>

aspect CallTracer {
  advice call ("% ...::%(...)") && !call ("% printf(...)") : around () {
    printf ("before call to %s\n", tjp->signature ());
    tjp->proceed ();
  }
};

class C {
public:
  void func () {}
};

void f (C& obj) {
  obj.func ();
}

int main () {
  printf ("CallOnRef:\n");
  printf ("=============================================================\n");
  C c;
  f (c);
  printf ("=============================================================\n");
  return 0;
}

