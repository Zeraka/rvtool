#include <iostream>

using namespace std;

class T1 {
private:
  int p;
public:
  int m;
  static int x;

  T1() : m( 5 ), p( 13 ) {}

  static void testsetS() {
    x = 99;
  }

  void testsetM() {
    m = 49;
  }

  void testsetP();

  void set_via_this(int m) {
    this->m = m;
  }

  int b:4;
  void set_bitfield() {
    b = 0;
  }
};

void T1::testsetP() {
  p = 15;
};

int T1::x = 23;

int a = 42;

T1 *o_ref; // for comparing in advice

int main() {
  T1 o;
  o_ref = &o; // init for comparison

  cout << "SET:" << endl;

  cout << endl << "A:" << endl;
  a = 5;

  cout << endl << "X1:" << endl;
  T1::x = 7;

  cout << endl << "X2:" << endl;
  T1::testsetS();

  cout << endl << "X3:" << endl;
  o.x = 21;

  cout << endl << "M1:" << endl;
  o.m = -3;

  cout << endl << "M2:" << endl;
  o.testsetM();

  cout << endl << "M3:" << endl;
  o.set_via_this(33);

  cout << endl << "P1:" << endl;
  o.testsetP();

  return 0;
}

void print_obj( void *ptr ) {
  if( ptr == 0 )
    cout << "<null>";
  else if( ptr == o_ref )
    cout << "<obj>";
  else
    cout << "<unknown>";
}

aspect SetTest1 {
  advice set("int ...::%") : before() {
    cout << "Before: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
    cout << "new: " << *tjp->arg<0>() << endl;
    cout << "obj: "; print_obj( tjp->target() ); cout << endl;
    cout << "src: "; print_obj( tjp->that() ); cout << endl;
  }

  advice set("int ...::%") : after() {
    cout << "After: " << tjp->signature() << endl;
    cout << "set: " << *tjp->entity() << endl;
  }
};

aspect SetTest2 {
  advice set("int ...::%") : around() {
    cout << "Around: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
    cout << "new: " << *tjp->arg<0>() << endl;
    *tjp->arg<0>() *= 2;

    tjp->proceed();

    JoinPoint::Entity &tmp = *tjp->entity();
    cout << "set: " << tmp << endl;
    tmp += 32;
  }
};

aspect SetTest3 {
  advice set("int ...::%") : around() {
    cout << "ent (inner): " << *tjp->entity() << endl;
    cout << "new (inner): " << *tjp->arg<0>() << endl;

    tjp->proceed();

    cout << "set (inner): " << *tjp->entity() << endl;
  }
};
