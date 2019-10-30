#include <stdio.h>

aspect Calls {
  advice call ("% %::%(...)") : before () {
    printf ("call to %s\n", tjp->signature ());
  }
  advice call ("% %::%(...)") : around () {
    printf ("around1: call to %s\n", tjp->signature ());
    tjp->proceed ();
  }
  advice call ("% %::%(...)") : around () {
    printf ("around2: call to %s\n", tjp->signature ());
    tjp->proceed ();
  }
};

aspect Calls2 {
  advice call ("% HasFriend::h(...)") : around () {
    printf ("Calls2: call to %s\n", tjp->signature ());
    tjp->proceed ();
  }
};

aspect Calls3 {
  advice call ("% HasFriend::h(...)") : around () {
    printf ("Calls3: call to %s\n", tjp->signature ());
    tjp->proceed ();
  }
};

class Base {
protected:
  Base () {}
  void f ();
public:
  void x ();
};

class HasFriend {
  friend void Base::x ();
  static void h () {
    printf (" --> access for friend Base::x() only\n");
  }
};
  
void Base::f () {
  printf (" --> protected in base\n");
  x ();
}

// need because starting with g++ 3.4.1 a friend has to be accessible
void Base::x () {
  HasFriend::h ();
}

class Derived : private Base {
  int _attr;
  void p () {
    printf (" --> private in same class\n");
  }
  int val () { return 4711; }
public:
  Derived ();
  void g1 () {
    printf (" --> public\n");
    p ();
    f ();
  }
  void g2 ();
};

Derived::Derived () : _attr (val ()) {
  printf (" --> call to private member from constructor init list\n");
}

void Derived::g2 () {
  printf (" --> public\n");
  p ();
  f ();
}

int main () {
  printf ("AccessControl: advice for calls with special privileges\n");
  printf ("=======================================================\n");

  Derived d;
  d.g1 ();
  d.g2 ();
  printf ("=======================================================\n");
}
