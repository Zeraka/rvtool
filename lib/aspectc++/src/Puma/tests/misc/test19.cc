// Object structure of the example:
//
//     A::f
//    /    \
//   B      H
//   |      |
//   C      D::f
//   |      |
//   E      F
//    \    /
//     G::g
//
// Test the "domination" rule of the name lookup.
// D::f() dominates A::f() although A::f() can
// be reached on a base class path that does not
// contain D (=> E->C->B-A).

struct A {void f();};
struct B : virtual A {};                        
struct H : virtual A {};
struct C : B {}; 
struct D : H {void f();};
struct E : C {}; 
struct F : D {};
struct G : E, F {
  void g() { f(); /* => D::f() */ }
};
