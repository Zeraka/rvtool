#include <iostream>

using namespace std;

class T1 {
private:
  int p;
public:
  int m;
  static int x;

  T1() : m( 5 ), p( 13 ) {}

  static void testgetS() {
    int r = x;
  }

  void testgetM() {
    int r = m;
  }

  void testgetP();
};

int T1::x = 23;

void T1::testgetP() {
  int r = p;
};

int a = 42;

int i = a;

int main() {
  int b;
  T1 o;

  cout << endl << "GET:" << endl;

  cout << endl << "A:" << endl; 
  b = a;
  
  cout << endl << "X1:" << endl; 
  b = T1::x;
  
  cout << endl << "X2:" << endl; 
  T1::testgetS();
  
  cout << endl << "X3:" << endl; 
  b = o.x;

  cout << endl << "M1:" << endl; 
  b = o.m;

  cout << endl << "M2:" << endl;
  o.testgetM();

  cout << endl << "P1:" << endl;
  o.testgetP();

  return 0;
}

aspect GetTest1 {
  advice get("int ...::%") : before() {
    cout << "Before: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
  }

  advice get("int ...::%") : after() {
    cout << "After: " << tjp->signature() << endl;
    cout << "res: " << *tjp->result() << endl;
  }
};

aspect GetTest2 {
  advice get("int ...::%") : around() {
    cout << "Around: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
    *tjp->entity() = 64;

    tjp->proceed();

    JoinPoint::Result &tmp = *tjp->result();
    cout << "tmp: " << tmp << endl;
    tmp += 32;
  }
};

aspect GetTest3 {
  advice get("int ...::%") : around() {
    cout << "Around (inner): " << tjp->signature() << endl;
    cout << "ent (inner): " << *tjp->entity() << endl;

    tjp->proceed();

    cout << "res (inner): " << *tjp->result() << endl;
  }
};

aspect MemberTest {
  advice get("int T1::%" && ! "static % T1::%") : before() {
    cout << "ent ( via memberptr() ): " << tjp->target()->*(tjp->memberptr()) << endl;
  }
};
