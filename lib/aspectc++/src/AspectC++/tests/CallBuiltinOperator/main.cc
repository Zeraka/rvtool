#include <iostream>

using namespace std;

int global;

void basicTests() {
  int a = 8, b = 23;
  bool n = true, m = false;
  int x = 1;
  int q[] = { 13, 23, 33, 43, 53 };
  const unsigned int i = 1;
  int *p = q;

  cout << "a = " << a << endl;
  cout << "b = " << b << endl;
  cout << "m = " << m << endl;
  cout << "n = " << n << endl;
  cout << "x = " << x << endl;

  cout << ( +a ) << endl;
  cout << ( -a ) << endl;
  cout << ( a + b ) << endl;
  cout << ( a - b ) << endl;
  cout << ( a * b ) << endl;
  cout << ( a / b ) << endl;
  cout << ( a % b ) << endl;

  cout << ( ~a ) << endl;
  cout << ( a & b ) << endl;
  cout << ( a | b ) << endl;
  cout << ( a ^ b ) << endl;
  cout << ( a << b ) << endl;
  cout << ( a >> b ) << endl;

  cout << ( a == b ) << endl;
  cout << ( a != b ) << endl;
  cout << ( a >= b ) << endl;
  cout << ( a <= b ) << endl;
  cout << ( a < b ) << endl;
  cout << ( a > b ) << endl;

  cout << ( !m ) << endl;
  cout << ( m && n ) << endl;
  cout << ( m || n ) << endl;

  cout << ( x = a ) << endl;
  cout << "Var: " << x << endl;

  cout << ( x += b ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x -= a ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x *= b ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x /= a ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x %= b ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x &= b ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x |= a ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x ^= b ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x <<= a ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x >>= b ) << endl;
  cout << "Var: " << x << endl;

  cout << ( x++ ) << endl;
  cout << "Var: " << x << endl;
  cout << ( x-- ) << endl;
  cout << "Var: " << x << endl;
  cout << ( --x ) << endl;
  cout << "Var: " << x << endl;
  cout << ( ++x ) << endl;
  cout << "Var: " << x << endl;

//  cout << ( &global ) << endl;
  cout << ( *p ) << endl;
  cout << ( q[3] ) << endl;
  cout << ( q[i] ) << endl;

  cout << ( "dummy", a ) << endl;
  cout << ( m ? a : b ) << endl;
  cout << ( n ? a : b ) << endl;
};

void nothing_to_do() {

}

void func(int arg) {
  int h = 0;
  h = arg != 0 ? (nothing_to_do(), 3) : 2;
}

class TestClass {
    // Members:
    int privatemember;
    static const int constmember = 5 * 2;
  public:
    unsigned int bitfield1 : 4;
    unsigned int bitfield2 : constmember - 5;
    static int depth;
    int intmember;
    TestClass* member;
    void (*mfunc)(int);

    // Constructor:
    TestClass(void (*pfunc)(int)) : intmember(424242), privatemember(212121), bitfield2(0x15), member(0), mfunc(0) {
      intmember = 3;
      int bla = 422142;
      bla = (intmember && depth) ? 4 : 5;
      const int drei = 3;
      const int& dreir = drei;
      const int* dreip = &dreir;
      privatemember = 1;
      bitfield1 = 2;
      ++bitfield1;
      bitfield1++;
      int k = 214221;
      k = bitfield1;
      k = bitfield1 + bitfield2;
      if(depth++ < 10) {
        --depth;
      }
      if(10 > depth++) {
        depth--;
      }
      member = 0;
      mfunc = pfunc;
    }
    TestClass() {}

    // Member-functions:
    void callmemfuncpointer() {
      mfunc(1);
      (*mfunc)(1);
      int* arr = new int[10];
      int k = arr[5];

      // Short-circuit-evaluation:
      bool b1 = true, b2 = false;
      bool shortcut = b1 && b2;
      shortcut = b2 && b1;
      shortcut = b1 || b2;
      shortcut = b2 || b1;
      int i1 = -1, i2 = 3;
      int shortcut_int = shortcut ? i1 : i2;
      shortcut_int = !shortcut ? i1 : i2;
      TestClass shortcut_tc = shortcut ? TestClass() : TestClass();
      TestClass tc1, tc2;
      shortcut_tc = shortcut ? tc1 : tc2;
      TestClass* shortcut_tcp = shortcut ? new TestClass() : new TestClass();
      delete shortcut_tcp;
      delete[] arr;
    }
};
int TestClass::depth = 0;

void caller(void (*fnc)(int)) {
  cout << endl << "Begin: Obj-Copy-Test";
  // Copy-assignment und copy-constructor:
  TestClass* tcp2 = new TestClass(fnc);
  tcp2->member = new TestClass(*tcp2);
  TestClass tc = TestClass(fnc);
  TestClass* tcp = new TestClass(fnc);
  TestClass tc2 = tc;
  TestClass tc3 = TestClass(tc);
  tc = TestClass(fnc);
  tc = tc2;
  cout << endl << "End: Obj-Copy-Test";

  tc.callmemfuncpointer();
  delete tcp;
  delete tcp2->member;
  delete tcp2;
}

// Constant, static, global:
const int const_two = 3 - 1;
static char char_array[const_two + 5];
int g1 = 0;
int g2 = g1 + 5;
enum ExampleEnum {
  ENUM_VALUE = const_two + 1
};

// Extern C:
extern "C"
{
  extern "C"
  {
    extern "C" void extern_c_func(int arg)
    {
      int e = 32767, e2 = 1;
      e = e2 + 4;
    }
    extern "C" int i;
  }
}

void complexTests() {
  extern_c_func(0);
  void* voidp = 0;
  const int const_22 = 3 - 1;
  void (*sfp)() = &nothing_to_do;
  void (**dfp)() = &sfp;
  dfp[0];

  //Member-pointer:
  ((1 == 2 ? (*dfp, &func) : &func))(1);
  TestClass tc = TestClass(&func);
  TestClass* (TestClass::*member_pointer) = 0;
  member_pointer = &TestClass::member;
  TestClass* value = tc.*member_pointer;
  value = (&tc)->*member_pointer;

  TestClass tc2 = TestClass(&func);
  int (TestClass::*int_member_pointer) = 0;
  int_member_pointer = &TestClass::intmember;
  int int_value = tc2.*int_member_pointer;
  int_value = (&tc2)->*int_member_pointer;

  caller( &func );

  // Miscellaneous:

  // ?: required to break the const_expr
  if( static_cast<void *>( &int_value ) != ( voidp ? static_cast<void *>( &tc2 ) : static_cast<void *>( &tc ) ) ) {}

  bool bool_value = true && int_value && &tc;
  int lala = dfp ? cout<< "hello" << endl, 4 : 5;
  int* lalap = 0;
  int la = (lalap = &lala, *lalap);
  int i = 1;
  int* ip = 0;
  int i2 = (ip = &i, *ip);

  int m1 = 3, m2 = 5;

  int* int_pointer1 = new int(7);
  int* int_pointer2 = new int(9);
  m1 = *int_pointer1 + *(&(((m1))));
  m2 = *int_pointer2 + m2;

  // FIXME: Due to pointers and result-type not reliable:
  // std::ptrdiff_t d = int_pointer2 - int_pointer1;

  cout << m1 << ", " << m2 << endl;
  delete int_pointer1;
  delete int_pointer2;
}

int main () {
  cout << endl << endl << endl;
  cout << "  ~~~ Simple checks ~~~" << endl << endl;
  basicTests();

  cout << endl << endl << endl;
  cout << "  ~~~ Complex checks ~~~" << endl << endl;
  complexTests();
}

template <typename T> inline void print_type() { cout << "<unknown>"; }
template <> inline void print_type<int>() { cout << "int"; };
template <> inline void print_type<int *>() { cout << "int *"; };
template <> inline void print_type<int * const>() { cout << "int * const"; };
template <> inline void print_type<int &>() { cout << "int &"; };
template <> inline void print_type<int *&>() { cout << "int *&"; };
template <> inline void print_type<int **>() { cout << "int **"; };
template <> inline void print_type<int ** const>() { cout << "int ** const"; };
template <> inline void print_type<const int>() { cout << "const int"; };
template <> inline void print_type<const int *>() { cout << "const int *"; };
template <> inline void print_type<const int &>() { cout << "const int &"; };
template <> inline void print_type<unsigned int>() { cout << "unsigned int"; };
template <> inline void print_type<unsigned long int>() { cout << "unsigned long int"; };
template <> inline void print_type<long int>() { cout << "long int"; };
template <> inline void print_type<bool>() { cout << "bool"; };
template <> inline void print_type<bool *>() { cout << "bool *"; };
template <> inline void print_type<bool &>() { cout << "bool &"; };
template <> inline void print_type<const bool>() { cout << "const bool"; };
template <> inline void print_type<bool * const>() { cout << "bool * const"; };
template <> inline void print_type<void>() { cout << "void"; };
template <> inline void print_type<void *>() { cout << "void *"; };

template <> inline void print_type<TestClass>() { cout << "TestClass"; };
template <> inline void print_type<TestClass *>() { cout << "TestClass *"; };
template <> inline void print_type<TestClass &>() { cout << "TestClass &"; };
template <> inline void print_type<TestClass *&>() { cout << "TestClass *&"; };
template <> inline void print_type<TestClass **>() { cout << "TestClass **"; };
template <> inline void print_type<TestClass ** const>() { cout << "TestClass ** const"; };
template <> inline void print_type<TestClass *TestClass::*>() { cout << "TestClass * TestClass::*"; };
template <> inline void print_type<TestClass * TestClass::*&>() { cout << "TestClass * TestClass::*&"; };
template <> inline void print_type<TestClass * TestClass::**>() { cout << "TestClass * TestClass::**"; };
template <> inline void print_type<TestClass * TestClass::** const>() { cout << "TestClass * TestClass::** const"; };
template <> inline void print_type<int TestClass::*>() { cout << "int TestClass::*"; };
template <> inline void print_type<int TestClass::*&>() { cout << "int TestClass::*&"; };
template <> inline void print_type<int TestClass::**>() { cout << "int TestClass::**"; };
template <> inline void print_type<int TestClass::** const>() { cout << "int TestClass::** const"; };

template <> inline void print_type<void (*)(int)>() { cout << "void (*)(int)"; };
template <> inline void print_type<void (*&)(int)>() { cout << "void (*&)(int)"; };
template <> inline void print_type<void (**)(int)>() { cout << "void (**)(int)"; };
template <> inline void print_type<void (** const)(int)>() { cout << "void (** const)(int)"; };
template <> inline void print_type<void (*)()>() { cout << "void (*)()"; };
template <> inline void print_type<void (*&)()>() { cout << "void (*&)()"; };
template <> inline void print_type<void (**)()>() { cout << "void (**)()"; };
template <> inline void print_type<void (** const)()>() { cout << "void (** const)()"; };

template <typename T> inline void print_value( T v ) { cout << v; }
template <typename T> inline void print_value( T *p ) {
  if( p ) {
    cout << "ptr<";
    print_value( *p );
    cout << ">";
  }
  else
    cout << "nullptr";
}
template <> inline void print_value( bool b ) { cout << ( b ? "true" : "false" ); }
template <> inline void print_value( void *p ) {
  if( p )
    cout << "ptr<void>";
  else
    cout << "nullptr";
}
template <> inline void print_value( void (* p)(int) ) {
  if( p == &func )
    cout << "fptr<func>";
  else if( p == 0 )
    cout << "nullfptr";
  else
    cout << "fptr<unknown>";
}
template <> inline void print_value( void (* p)() ) {
  if( p == &nothing_to_do )
    cout << "fptr<nothing_to_do>";
  else if( p == 0 )
    cout << "nullfptr";
  else
    cout << "fptr<unknown>";
}
inline void print_value( TestClass & ) { cout << "<obj>"; }
template <> inline void print_value<TestClass>( TestClass *p ) {
  if( p )
    cout << "objptr<{TestClass}>";
  else
    cout << "nullptr";
}
template <> inline void print_value( TestClass * TestClass::*p ) {
  if( p == &TestClass::member )
    cout << "memptr<TestClass::member>";
  else if( p )
    cout << "memptr<unknown>";
  else
    cout << "memptr<invalid>";
}
template <> inline void print_value( int TestClass::*p ) {
  if( p == &TestClass::intmember )
    cout << "memptr<TestClass::intmember>";
  else if( p )
    cout << "memptr<unknown>";
  else
    cout << "memptr<invalid>";
}

template <int I> struct ARGPrinter {
  template <class JoinPoint> static inline void print( JoinPoint &tjp ) {
    ARGPrinter<I - 1>::print( tjp );
    cout << "  ";
    print_type< typename JoinPoint::template Arg<I - 1>::Type >();
    cout << " Arg" << I << " = ";
    if(tjp.template arg<I - 1>() > 0) {
      print_value( *tjp.template arg<I - 1>() );
    }
    else {
      cout << "<not available>";
    }
    cout << endl;
  }
};
template <> struct ARGPrinter<0> {
  template <class JoinPoint> static inline void print( JoinPoint &tjp ) {}
};

template <class JoinPoint> inline void print_args( JoinPoint &tjp ) {
  ARGPrinter<JoinPoint::ARGS>::print( tjp );
}

template <typename T> inline void print_objptr( T *p ) {
  if( p )
    cout << "<obj>";
  else
    cout << "nullptr";
}

aspect CallTracer {
  pointcut builtin_ops() = builtin("% operator %(...)");

  advice builtin_ops() : before() {
    cout << endl << tjp->signature() << " (Line: " << tjp->line() << ")" <<  endl;
  }

  advice builtin_ops() : after() {
    cout << "  ";
    print_type< JoinPoint::That >();
    cout << " That = ";
    print_objptr( tjp->that() );
    cout << endl;

    cout << "  ";
    print_type< JoinPoint::Target >();
    cout << " Target = ";
    print_objptr( tjp->target() );
    cout << endl;

    cout << ">>>> ";
  }

  advice builtin_ops() : around() {
    print_args<>( *tjp );

    tjp->proceed();
  }

  advice builtin_ops() : around() {
    tjp->proceed();
    cout << "  ";
    print_type< JoinPoint::Result >();
    cout << " Result = ";
    print_value( *tjp->result() );
    cout << endl;
  }

  // advice to structure the output
  advice construction( "TestClass" ) : before() {
    cout << endl << "~~~TestClass init >>>";
  }
  advice construction( "TestClass" ) : after() {
    cout << endl << "~~~TestClass init <<<";
  }
};

// Test whether advice with args-pointcut-function will be handled 
// correctly on join-points with short-circuit-evaluation:
aspect ShortCircuitArgsPointcutFunctionTester {
  pointcut shortcircuiting_ops() = builtin("% operator &&(...)" || "% operator ||(...)" || "% operator ?:(...)");

  advice shortcircuiting_ops() : order("ShortCircuitArgsPointcutFunctionTester", "CallTracer");

  advice shortcircuiting_ops() : before() {
    cout << endl << "<available ";
  }

  advice shortcircuiting_ops() && args(b1, "%") : before(bool b1) {
    cout << "short-circuit arg: ";
  }

  advice shortcircuiting_ops() && args(b1, "%", "%") : before(bool b1) {
    cout << "short-circuit";
  }

  advice shortcircuiting_ops() && args(b1, b2) : around(bool b1, bool b2) {
    cout << "se";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", b2) : around(bool b2) {
    cout << "co";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%") : around() {
    if(tjp->arg<1>() > 0) {
      cout << "nd" << " (" << (*(tjp->arg<1>()) ? "true" : "false") << ")";
    }
    else {
      cout << "<none>";
    }
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args(b1, i1, i2) : around(bool b1, int i1, int i2) {
    cout << "!!! This should never be displayed !!!" << endl;
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", i1, i2) : around(int i1, int i2) {
    cout << "!!! This should never be displayed !!!" << endl;
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", "%") : before() {
    cout << "  arg: ";
  }

  advice shortcircuiting_ops() && args("%", "%", i2) : around(int i2) {
    cout << "third (" << i2 << ")";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", i1, "%") : around(int i1) {
    cout << "second (" << i1 << ")";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", ir2) : around(int& ir2) {
    cout << "third (" << ir2 << ")";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", ir1, "%") : around(int& ir1) {
    cout << "second (" << ir1 << ")";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", t2) : around(TestClass t2) {
    cout << "third ([TestClass])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", t1, "%") : around(TestClass t1) {
    cout << "second ([TestClass])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", tr2) : around(TestClass& tr2) {
    cout << "third ([TestClass-reference])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", tr1, "%") : around(TestClass& tr1) {
    cout << "second ([TestClass-reference])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", tp2) : around(TestClass* tp2) {
    cout << "third ([TestClass-pointer])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", tp1, "%") : around(TestClass* tp1) {
    cout << "second ([TestClass-pointer])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", "%", vp2) : around(void* vp2) {
    cout << "third ([void-pointer])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() && args("%", vp1, "%") : around(void* vp1) {
    cout << "second ([void pointer])";
    tjp->proceed();
  }

  advice shortcircuiting_ops() : before() {
    cout << ">";
  }
};
