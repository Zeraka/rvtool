#include <stdio.h>

class External {
protected:
  void prot () { printf ("  External::prot()\n"); }
  virtual void vprot () { printf ("  External::vprot()\n");}
};

extern "C" {
  struct DontInsertBypassTemplate {};
}

// ---------------------------------

class InternalPrivateBase {
public:
  void x() { printf ("--- x ---\n"); }
};

class Internal : private External, private InternalPrivateBase {
  friend void friend_priv (Internal *obj);
  friend void friend_prot (Internal *a);
  friend void friend_static_stuff ();
  void priv () { printf ("  Internal::priv()\n");}
  static void static_priv () { printf ("  Internal::static_priv()\n");}
  void vprot () { printf ("  Internal::vprot()\n");}
public:
  static void static_pub () { printf ("  Internal::static_pub()\n");}
  void f ();
private:
  void g () {
    printf ("--- g ---\n");
    External::vprot (); vprot (); priv ();
  }
};

void Internal::f () {
  printf ("--- f ---\n");
  External::prot (); priv (); g(); x();
}

void friend_prot (Internal *obj) { obj->prot (); }
void friend_priv (Internal *obj) { obj->priv (); }
void friend_static_stuff () { Internal::static_priv (); Internal i; i.static_priv (); i.static_pub (); }

class Internal2 : public Internal {
protected:
  virtual void vprot () {}
};

int main () {
  printf ("CallAccess: call advice for private or protected member fcts\n");
  printf ("============================================================\n");
  Internal i;
  i.f ();
  printf ("--- friends ---\n");
  friend_prot (&i);
  friend_priv (&i);
  printf ("--- problem with static private functions (bug 485)\n");
  friend_static_stuff ();
  printf ("============================================================\n");
}

aspect X {
  advice call("% External::prot()") : around () { tjp->proceed (); }
  advice call("% External::vprot()") : around () { tjp->proceed (); }
  advice call("% Internal::priv()") : around () { tjp->proceed (); }
  advice call("% Internal::static_p%()") : before () { }
  advice call("% InternalPrivateBase::x()") : around () { tjp->proceed (); }
};

/*

Possible cases:

1. A member function calls a PROTECTED member function of a base class
   => put friend declaration into the calling (derived) class
2. A friend class or function of a class A calls a PROTECTED base class
   function of A
   => put friend declaration in A
3. Within a class A a private member is called
   => put friend declaration into A
4. A function or class is a friend and calls a PRIVATE member of class A
   => put friend declaration into target class A

The cases 3. and 4. will not be supported for external target classes
=> error message! 

*/
