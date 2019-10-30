#include <stdio.h>

class A {
  int a;
public:
  virtual void foo() {};
};

class A0 : public A {};

template<typename T>
class B : private A {
  struct Inner {};
};

B<double> bbb; // create two instances of B<T>!

template <typename T>
class C : private B<int> {
public:
  void foo() {}
  void bar() {}
};

class D : public C<char> {
public:
  void moo() {}
};

class E : public D {
public:
  void bla() {}
};


int main(){
  E e;
  printf ("Bug477: Templates muddle base() and derived() pointcut functions\n");
  printf ("================================================================\n");
  e.foo(); // not matched by derived(A)
  e.bar(); // not matched by derived(A)
  
  ((A*)&e)->foo(); // not matched by base(E)
  
  e.moo(); // not matched by derived(A)
  e.bla(); // not matched by derived(A)
  printf ("================================================================\n");
  
  return 0;
}

aspect DerivedTest {

  advice call(derived("A")) : before() {
    printf ("derived(\"A\"): %s\n", JoinPoint::signature());
  }
  
  advice call(base("E")) : before() {
    printf ("base(\"E\"): %s\n", JoinPoint::signature());
  }
};
