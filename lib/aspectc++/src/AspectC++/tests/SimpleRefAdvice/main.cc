#include <iostream>

using namespace std;

class T1 {
private:
  int p;
public:
  int m;
  static int x;

  T1() : m( 5 ), p( 13 ) {}

  static void testrefS() {
    int *r = &x;
  }

  void testrefM() {
    int *r = &m;
  }

  void testrefP();
};

void paramtest1( int &r ) {
  cout << "1: " << r << endl;
}

void paramtest2( int &r = T1::x ) {
  cout << "2: " << r << endl;
}

int &returntest() {
  return T1::x;
}

int T1::x = 23;

void T1::testrefP() {
  int *r = &p;
};

int a = 42;

int *i = &a;
int &j = T1::x;

int main() {
  int *b;
  T1 o;

  cout << endl << "REF (explitcit):" << endl;

  cout << endl << "A:" << endl; 
  b = &a;
  
  cout << endl << "X1:" << endl; 
  b = &T1::x;
  
  cout << endl << "X2:" << endl; 
  T1::testrefS();
  
  cout << endl << "X3:" << endl; 
  b = &o.x;

  cout << endl << "M1:" << endl; 
  b = &o.m;

  cout << endl << "M2:" << endl;
  o.testrefM();

  cout << endl << "P1:" << endl;
  o.testrefP();

  cout << endl << "REF (implicit):" << endl;

  cout << endl << "local ref:" << endl;
  int &c = a;

  cout << endl << "param ref:" << endl;
  paramtest1( a );

  cout << endl << "default param ref (no default):" << endl;
  paramtest2( a );

  cout << endl << "default param ref (default):" << endl;
  paramtest2();

  cout << endl << "return ref:" << endl;
  cout << returntest() << endl;

  return 0;
}

aspect RefTest1 {
  advice ref("int ...::%") : before() {
    cout << "Before: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
  }

  advice ref("int ...::%") : after() {
    cout << "After: " << tjp->signature() << endl;
    cout << "res: " << **tjp->result() << endl;
  }
};

aspect RefTest2 {
  advice ref("int ...::%") : around() {
    cout << "Around: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;

    tjp->proceed();

    int &tmp = **tjp->result();
    cout << "tmp: " << tmp << endl;
  }
};

aspect RefTest3 {
  advice ref("int ...::%") : around() {
    cout << "Around (inner): " << tjp->signature() << endl;
    cout << "ent (inner): " << *tjp->entity() << endl;

    tjp->proceed();

    cout << "res (inner): " << **tjp->result() << endl;
  }
};

aspect MemberTest {
  advice ref("int T1::%" && ! "static % T1::%") : before() {
    cout << "ent ( via memberptr() ): " << tjp->target()->*(tjp->memberptr()) << endl;
  }
};
