// Virtual inheritance structure:
//
//        A::f
//       /  /
//    B::f /
//    / \ /
//  C::f D
//    \ /
//     E


struct A {
  void f();
};

struct B : virtual public A {
  void f();
};

struct C : virtual public B {
  void f();
};

struct D : virtual public A, virtual public B {};

struct E : virtual public D, virtual public C {};

void g() {
  E *e;
  e->f();  // => C::f()
}
