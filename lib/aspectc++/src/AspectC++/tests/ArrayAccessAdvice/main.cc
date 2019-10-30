#include <iostream>
#include "namemap.h"

using namespace std;

// forward decl for friend
namespace NameMap { void register_vars(); }

class T {
  double fill;
  friend void NameMap::register_vars();
private:
  int p[3] = { 22, 33, 44 };
public:
  int m[2][3][2] = { { { 3, 4 }, { 1, 0 }, { 20, 25 } }, { { 300, 400 }, { 100, -1 }, { 2000, 2500 } } };
  static int s[2];

  void test() {
    cout << endl;
    cout << "array 3-Dim Member (internal):" << endl;
    int x = m[0][0][0];
    m[0][0][1] = 13;

    cout << endl;
    cout << "array 1-Dim Private (internal):" << endl;
    int y = p[0];
    p[2] = 99;
  }
} t;
int T::s[2] = { 60, 90 };

int a[5] = { 1, 3, 5, 7, 9 };
int b[2][3] = { 2, 4, 6, 1, 3, 5 };

// --- Helper code
namespace NameMap {
  void register_vars() {
    objs[ &t ] = Info( "t", sizeof( t ) );
    arrays[ &T::s ] = Info( "T::s", sizeof( T::s ) );
    arrays[ &t.m ] = Info( "t.m", sizeof( t.m ) );
    arrays[ &t.p ] = Info( "t.p", sizeof( t.p ) );

    arrays[ &a ] = Info( "a", sizeof( a ) );
    arrays[ &b ] = Info( "b", sizeof( b ) );
  }
}
// --- end Helpers

void check_basic() {
  cout << endl;
  cout << "array 1-Dim:" << endl; {
    int x = a[2];
    a[0] = 11;
    int *p1 = &a[1];
    int *p2 = a;
    int &r = a[2];
  }

  cout << endl;
  cout << "array 2-Dim:" << endl; {
    int x = b[1][0];
    b[0][2] = 12;
    int *p1 = &b[1][2];
    int (*p1a)[3] = &b[1];
    int (*p2)[3] = b;
    int *p2a = b[0];
    int &r = b[0][1];
    int (&ra)[3] = b[1];
  }

  cout << endl;
  cout << "array 3-Dim Member:" << endl; {
    int x = t.m[0][1][1];
    t.m[0][2][0] = 42;
  }

  t.test();

  cout << endl;
  cout << "array 1-Dim Static:" << endl; {
    int x = t.s[0];
    t.s[1] = 23;

    int y = T::s[1];
    T::s[0] = 17;
  }
}

void check_dual() {
  cout << endl;
  cout << "array 1-Dim:" << endl; {
    int x = a[4];
    a[1] = 99;
  }

  cout << endl;
  cout << "array 3-Dim Member:" << endl; {
    int x = t.m[1][2][0];
    t.m[1][0][1] = 21;
  }
}

void check_dual_partial1() {
  cout << endl;
  cout << "partial one:" << endl; {
    int x = t.m[1][2][0];
    t.m[1][0][1] = 21;
  }
}

void check_dual_partial2() {
  cout << endl;
  cout << "partial two:" << endl; {
    int x = t.m[1][2][0];
    t.m[1][0][1] = 21;
  }
}

void check_dual_partial3() {
  cout << endl;
  cout << "partial three:" << endl; {
    int x = t.m[1][2][0];
    t.m[1][0][1] = 21;
  }
}

void check_proceed() {
  int x = b[0][1];
  b[1][0] = 32;
}

void check_action() {
  int x = b[1][1];
  b[1][2] = 48;
}

void check_tunnel() {
  T::s[1] = 666;
  t.s[0] += 222;
}

void check_qualified() {
  t.T::m[1][2][1] *= 3;

  unsigned int i = 5; // dummy non const expr
  i /= 3;
  int *p = 0;
  p = &t.T::m[i][i-1][i];
}

#include <map>
map<int,char> TestMap;
int *TestPointer = new int[3];

namespace NameMap {
  char DUMMY = TestMap[3] = '3'; // preinit TestMap element for output
  char *DUMMYPOINTER = &DUMMY; // needed to enable alias matching
}

void check_interaction() {
  TestMap[3] = 'a';

  int x = TestPointer[2];
  TestPointer[0] = 5;

  a[3];
}

struct CTest {
  int m[12];
} cTest;

namespace NameMap {
  void register_vars2() {
    objs[ &cTest ] = Info( "cTest", sizeof( cTest ) );
    arrays[ &cTest.m ] = Info( "cTest.m", sizeof( cTest.m ) );
  }
}

void check_constr() {
  int x = cTest.m[6];
}

struct Incomplete {
  static const int incomp[];
  static int check_incomp() {
    return incomp[0] + incomp[1];
  }
};

const int Incomplete::incomp[2] = { 1, 2 };


int main() {
  NameMap::register_vars();

  cout << "Basic Checks:" << endl;
  check_basic();

  cout << endl << endl << "Checking dual Weaving:" << endl;
  check_dual();

  cout << endl << endl << "Checking partial dual Weaving:" << endl;
  check_dual_partial1();
  check_dual_partial2();
  check_dual_partial3();

  cout << endl << endl << "Checking proceed:" << endl;
  check_proceed();

  cout << endl << endl << "Checking action:" << endl;
  check_action();

  cout << endl << endl << "Checking tunneling:" << endl;
  check_tunnel();

  cout << endl << endl << "Checking qualified accesses:" << endl;
  check_qualified();

  cout << endl << endl << "Checking interaction with other types:" << endl;
  check_interaction();

  NameMap::register_vars2();

  cout << endl << endl << "Checking interaction with construction advice:" << endl;
  check_constr();

  cout << endl << endl << "Checking incomplete array type:" << endl;
  Incomplete::check_incomp();

  return 0;
}
