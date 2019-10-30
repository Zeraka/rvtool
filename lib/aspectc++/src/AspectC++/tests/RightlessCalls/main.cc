#include <stdio.h>

class Map {
  class Iter { 
  public:
    int value () { // object type inaccessible for main()
      printf ("inaccessible target object type\n");
    }
  };
  friend class MapIter;
};

class MapIter {
  typedef Map::Iter Item;
  friend Item friendFunc();
public:
  Item prev () { // result type inaccessible for main()
    printf ("inaccessible result type\n");
    return Item ();
  }
  static void check (const Map::Iter &) {}
};

MapIter::Item friendFunc () {
    printf ("inaccessible result, no object type\n");
}
 
class Outer {
  class C {};
  class D : public C {};
  friend int main();
  friend C& func(C);
};

Outer::C& func(Outer::C) {}
void func(int) {}

int main() {
  printf ("RightlessCalls: Calls using inaccesible types\n");
  printf ("=============================================================\n");
  MapIter i;
  i.prev ().value ();
  //  MapIter::check (i.prev ());
  friendFunc ();
  // TODO: inaccesible argument types are not supported yet!
  // printf ("-------------------------------------------------------------\n");
  // func (Outer::D()); // here type deduction is tricky
  printf ("=============================================================\n");
}

aspect Trace {
  advice call("% Map%::...::%(...)" || "% friendFunc()") : before () {
    printf ("calling %s: ", JoinPoint::signature ());
  }
};
