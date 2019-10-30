#include <iostream>

using namespace std;

int a = 42;
int b = 3;

short s;

struct S {
  short s;
} o;

int main() {
  cout << "COMPOUND:" << endl;

  cout << endl << "<var> += <const>:" << endl;
  cout << ( a += 3 ) << endl;

  cout << endl << "<var> += <var>:" << endl;
  cout << ( a += b ) << endl;

  cout << endl << "<var>++:" << endl;
  cout << ( a++ ) << endl;

  cout << endl << "++<var>:" << endl;
  cout << ( ++a ) << endl;

  cout << endl << "<var>--:" << endl;
  cout << ( a-- ) << endl;

  cout << endl << "--<var>:" << endl;
  cout << ( --a ) << endl;

  cout << endl << "<var> -= <var>:" << endl;
  cout << ( a -= b ) << endl;

  cout << endl << "<var> *= <var>:" << endl;
  cout << ( a *= b ) << endl;

  cout << endl << "<var> /= <var>:" << endl;
  cout << ( a /= b ) << endl;

  cout << endl << "<var> %= <var>:" << endl;
  cout << ( a %= b ) << endl;

  cout << endl << "<var> |= <var>:" << endl;
  cout << ( a |= b ) << endl;

  cout << endl << "<var> ^= <var>:" << endl;
  cout << ( a ^= b ) << endl;

  cout << endl << "<var> &= <var>:" << endl;
  cout << ( a &= b ) << endl;

  cout << endl << "<var> <<= <var>:" << endl;
  cout << ( a <<= b ) << endl;

  cout << endl << "<var> >>= <var>:" << endl;
  cout << ( a >>= b ) << endl;

  // checking forwarding without advice on op
  cout << endl << endl << "FORWARD:" << endl;

  cout << endl << "<var> = <const>:" << endl;
  { short& r = ( s = 5 ); cout << r << endl; }
  { short& r = ( o.s = 16 ); cout << r << endl; }

  cout << endl << "<var> += <const>:" << endl;
  { short& r = ( s += 2 ); cout << r << endl; }
  { short& r = ( o.s += 2 ); cout << r << endl; }

  cout << endl << "<var> -= <const>:" << endl;
  { short& r = ( s -= 4 ); cout << r << endl; }
  { short& r = ( o.s -= 4 ); cout << r << endl; }

  cout << endl << "<var> *= <const>:" << endl;
  { short& r = ( s *= 5 ); cout << r << endl; }
  { short& r = ( o.s *= 5 ); cout << r << endl; }

  cout << endl << "<var> /= <const>:" << endl;
  { short& r = ( s /= 3 ); cout << r << endl; }
  { short& r = ( o.s /= 3 ); cout << r << endl; }

  cout << endl << "<var> %= <const>:" << endl;
  { short& r = ( s %= 7 ); cout << r << endl; }
  { short& r = ( o.s %= 7 ); cout << r << endl; }

  cout << endl << "<var> ^= <const>:" << endl;
  { short& r = ( s ^= 3 ); cout << r << endl; }
  { short& r = ( o.s ^= 3 ); cout << r << endl; }

  cout << endl << "<var> |= <const>:" << endl;
  { short& r = ( s |= 5 ); cout << r << endl; }
  { short& r = ( o.s |= 5 ); cout << r << endl; }

  cout << endl << "<var> &= <const>:" << endl;
  { short& r = ( s &= 3 ); cout << r << endl; }
  { short& r = ( o.s &= 3 ); cout << r << endl; }

  cout << endl << "<var> <<= <const>:" << endl;
  { short& r = ( s <<= 3 ); cout << r << endl; }
  { short& r = ( o.s <<= 3 ); cout << r << endl; }

  cout << endl << "<var> >>= <const>:" << endl;
  { short& r = ( s >>= 2 ); cout << r << endl; }
  { short& r = ( o.s >>= 2 ); cout << r << endl; }

  cout << endl << "++<var>:" << endl;
  { short& r = ( ++s ); cout << r << endl; }
  { short& r = ( ++o.s ); cout << r << endl; }

  cout << endl << "--<var>:" << endl;
  { short& r = ( --s ); cout << r << endl; }
  { short& r = ( --o.s ); cout << r << endl; }

  return 0;
}

aspect SetTest1 {
  advice set("int ...::%") : before() {
    cout << "Before: " << tjp->signature() << endl;
    cout << "ent: " << *tjp->entity() << endl;
    cout << "new: " << *tjp->arg<0>() << endl;
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

aspect GetTest {
  advice get("int ...::%") : after() {
    cout << "Get: " << tjp->signature() << " = " << *tjp->entity() << endl;
  }
};

aspect Call {
  advice builtin("int &operator ++(int &)") : around() {
    cout << "Hello Operator: " << tjp->signature() << endl;
    tjp->proceed();
    cout << "Bye." << endl;
  }
};

aspect RefTest {
  advice ref("short ...::%") : after() {
    cout << "Ref: " << tjp->signature() << " ";
    bool pass = true;
    if( tjp->target() ) {
      pass = pass && ( tjp->target() == &o );
      cout << ( tjp->target() == &o ? "." : "F" );
      pass = pass && ( tjp->entity() == &o.s );
      cout << ( tjp->entity() == &o.s ? "." : "F" );
    }
    else {
      cout << "_";
      pass = pass && ( tjp->entity() == &s );
      cout << ( tjp->entity() == &s ? "." : "F" );
    }
    if( pass )
      cout << " PASSED";
    else
      cout << " FAILED !!!";
    cout << endl;
  }
};
