#include <iostream>

using namespace std;

int FUNC_NM( int a, int b ) {
  cout << "self: FUNC_NM" << endl;
  return a * 4 + b;
}

class T1 {
public:
  static int FUNC_SM( int a, int b ) {
    cout << "self: FUNC_SM" << endl;
    return a * 7 - b;
  }

  int FUNC_OM( int a, int b ) {
    cout << "self: FUNC_OM" << endl;
    return a * -1 + 10 * b;
  }

  virtual int FUNC_VM( int a, int b ) {
    cout << "self: FUNC_VM" << endl;
    return a * -5 + 12 * b;
  }
};

int main() {
  int b;
  T1 o;

  cout << endl << "FUNC-ENTITY:" << endl;

  cout << endl << "FUNC_NM:" << endl;
  b = FUNC_NM( 1, 2 );

  cout << endl << "FUNC_SM (1):" << endl;
  b = T1::FUNC_SM( 1, 2 );

  cout << endl << "FUNC_SM (2):" << endl;
  b = o.FUNC_SM( 1, 2 );

  cout << endl << "FUNC_OM:" << endl;
  b = o.FUNC_OM( 1, 2 );

  cout << endl << "FUNC_VM:" << endl;
  b = o.FUNC_VM( 1, 2 );

  return 0;
}

template<typename TARGET, typename MEMBER> int call_member( TARGET *t, MEMBER m ) {
  return (t->*m)( 1, 2 );
}

template<typename TARGET> int call_member( TARGET *t, void * ) { return 0; }

aspect FuncTest {
  advice call("int ...::FUNC%( int, int )") : after() {
    cout << "After call: " << tjp->signature() << endl;
    if( tjp->entity() )
      int r = tjp->entity()( 1, 2 );
    if( tjp->memberptr() )
      int r = call_member( tjp->target(), tjp->memberptr() );
  }

  advice execution("int ...::FUNC%( int, int )") : after() {
    static bool recursion = false;
    if (!recursion) {
      cout << "After execution: " << tjp->signature() << endl;
      recursion = true;
      if( tjp->entity() )
	int r = tjp->entity()( 1, 2 );
      if( tjp->memberptr() )
	int r = call_member( tjp->target(), tjp->memberptr() );
      recursion = false;
    }
  }

  advice construction("T%") : before() {
    cout << "Construction of " << tjp->signature() << endl;
    cout << "  memberptr: " << (void*)tjp->memberptr() << endl;
    cout << "  entity:    " << (void*)tjp->entity() << endl;
  }

  advice destruction("T%") : before() {
    cout << "Destruction of " << tjp->signature() << endl;
    cout << "  memberptr: " << (void*)tjp->memberptr() << endl;
    cout << "  entity:    " << (void*)tjp->entity() << endl;
  }
};
