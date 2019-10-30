#include <stdio.h>

// This code is from bug report 548 -- it showed up in Fiaco

class Foo {
public:
  int bar() { printf ("In Foo's bar.\n"); return 42; }
};

template<typename T> 
class Iterator {
  typedef T *Value_type;
  Value_type _c;

public:
  Iterator() { _c = new Foo(); }

  Value_type operator * () { return static_cast<Value_type>(_c); }
  Value_type operator -> () { return static_cast<Value_type>(_c); }

  void bar() {};
};

// These classes are needed to create an even longer operator-> chain

struct C4 {
  int thats_it() const { printf("That's it!\n"); return 42; }
} c4;

struct C3 {
  C4 *operator -> () const { return &c4; }
} c3;

struct C2 {
  C3 operator -> () const { return c3; }
} c2;

struct C1 {
  C2 operator -> () const { return c2; }
} c1;

// Call both chains
int main() {
  printf ("Bug548: operator->() chains not resolved properly\n");
  printf ("=================================================\n");

  Iterator<Foo> i;
  int x = i->bar();
  
  c1->thats_it();

  printf ("=================================================\n");
}

// Weave call advice for it
aspect MyAspect {
  //advice call("% ...::%(...)") : around() { // works, though
  advice call("% Foo::%(...)") : around() {
    tjp->target()->bar();
  }
  advice call("% C2::%(...)"||"% C4::%(...)") : around() {
    tjp->proceed();
  }
};
