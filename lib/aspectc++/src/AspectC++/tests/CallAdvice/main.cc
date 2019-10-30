#include <stdio.h>

int f1 (int a) { return a + 1; }
int f2 (int a, int) { return a + 2; }
int f3 (int a, int, int) { return a + 2; }
void var_arg_fct (const char *fmt, ...) { }

class Foo {
public:
  void bar() {}
};

class Cyg_Thread {
  Foo foo;
public:
  static Cyg_Thread *self() { return 0; }
  void set_timer() {
    self()->foo.bar(); // THIS FAILS with Bug 513
  }

};

struct Data {
  int i;
};

struct DataProxy {
  Data d;
  Data *operator ->() { return &d; }
};

struct QString {
  const char *_s;
  QString (const char*s) : _s(s) {}
};

struct QStream {
  QStream &operator << (const QString &qs) {
    static QStream stream;
    printf ("%s", qs._s);
    return stream;
  }
};

class C {
  int _val;
  DataProxy _dp;
public:
  C () : _val (4711) {}
  operator int () { return _val; }
  int val () const { return _val; }
  C &operator = (const C &obj) {
    return *this;
  }
  C dup () { 
    C duplicate;
    duplicate = *this;
    return duplicate;
  }
  C operator ! () { return *this; }
  int operator + (int) { return 0; }
  DataProxy &operator -> () { return _dp; }
  void cnst () const { printf ("the const function\n"); }
  void cnst () { printf ("the non-const version\n"); };
  static void stat (int = 0) { printf ("in a static member\n"); }
  int stuff (C *pClass) { printf ("in function with class-ptr arg\n"); return 0; }
  bool operator != (int) { return true; }
  int operator * () { return 0; }
  C& operator ++ (int) { return *this; }
  C& operator ++ () { return *this; }
  int operator [] (int) const { return 42; } 
  C& operator %= (int i) { return *this; }
};

C operator % (const C& c1, const C& c2) { return C(); }
bool operator + (C, C) { return false; }
int operator ~ (C) { return 0; }

class B1 {
public:
  void f (int) {}
  virtual void g () { printf ("in the base function - OK\n"); }
};

class B2 : public B1 {
public:
  void f () { B1::f (0); }
  void g () { printf ("in the derived function - ERROR\n"); }
};

struct ABase {
  C c1;
  mutable C c2;
};

class A : public ABase {
public:
  void f () const { c1.cnst (); c2.cnst (); }
  void g () { c1.cnst (); c2.cnst (); }
};

class OpTester {
public:
  void run () {
    C c1, c2;
    c1 + c2;
    ~c1;
    *c1;
    c1 % c2;
    c1 %= 99;
  }
};

struct GlobalPrinter {
  GlobalPrinter () {
    printf ("CallAdvice: code generation for all kinds of functions\n");
    printf ("=============================================================\n");
    printf ("global initialization:\n");
  }
  ~GlobalPrinter () {
    printf ("=============================================================\n");
  }
} gp;

int global () { printf ("in a global function\n"); return 42; }

int g = global ();
namespace XX {}
namespace XX { int g = global (); }
class G {
  static int g2;
public:
  static void f (int i = global ()) {}
};

int G::g2 = global ();

#define SOME_MACRO(x) x

void assert_fct() {}
#define MY_ASSERT(x) assert_fct()

class testfoo {
public:
  void ldebug(const char *, const char *) { printf ("in ldebug\n"); }
};

// --- bug 557:
class ABC {
    typedef struct { 
        int a;
    } newtype;

public:
    int b;

};
// --- end bug 557

// --- bug 588
class CallOp {
public:
  void operator() () {}
  void operator() (int) {}
} call_op;
// -- end bug 588

int main () {
  struct Local {
    static void f () { global (); } // call in local class => shall not match!
  };
  printf ("-------------------------------------------------------------\n");
  printf ("call to global function from local class => shall not match:\n");
  Local::f ();
  printf ("-------------------------------------------------------------\n");
  printf ("various kinds of functions:\n");
  C inst;
  C inst2 (inst.dup ());
  printf ("result is %d\n", f1 (f2 (3, 4)) + inst.dup ().dup ().val ());
  f3 (f1 (f2 (5, 6)), f1 (f2 (7, 8)), 1);
  !!!!inst;
  inst + 0;
  inst + f1 (2);
  inst + inst;
  ~inst;
  inst.cnst ();
  inst.stuff (&inst);
  const C cc;
  cc.cnst ();
  testfoo *foo = new testfoo;
  const char *s = "Hi";
  ((foo-> ldebug))("something '%s'", s);
  delete foo;
  printf ("-------------------------------------------------------------\n");
  printf ("static member called:\n");
  C::stat ();
  printf ("-------------------------------------------------------------\n");
  printf ("call in default arg:\n");
  G::f ();
  printf ("-------------------------------------------------------------\n");
  printf ("static member called with object:\n");
  inst.stat (3);
  printf ("-------------------------------------------------------------\n");
  printf ("function with variable argument list:\n");
  var_arg_fct ("bla %s %d %d", "a string", 4711, true);
  var_arg_fct ("bla %f", 3.14);
  printf ("-------------------------------------------------------------\n");
  printf ("calls with fully qualified name:\n");
  B2 b2;
  b2.f ();
  b2.B1::g ();
  printf ("-------------------------------------------------------------\n");
  printf ("calls in const member function:\n");
  A a;
  a.f ();
  a.g ();
  printf ("-------------------------------------------------------------\n");
  printf ("operator calls from within non-static methods:\n");
  OpTester ot;
  ot.run ();
  printf ("-------------------------------------------------------------\n");
  printf ("calls to conversion functions:\n");
  C c;
  int i;
  i = c;
  // i = (int)c; <-- this should also be affected by advice
  printf ("-------------------------------------------------------------\n");
  printf ("calls to operator():\n");
  call_op();
  call_op(42);
  printf ("-------------------------------------------------------------\n");
  printf ("chained calls to operator->():\n");
  C c2;
  i = c2->i;
  printf ("other chained call (bug 513):\n");
  Cyg_Thread::self()->set_timer();
  printf ("-------------------------------------------------------------\n");
  printf ("implicit conversion:\n");
  QStream qs;
  qs << "  *hello*\n" << "  *world*\n";
  printf ("-------------------------------------------------------------\n");
  printf ("prefix and postfix operators:\n");
  ++c++++;
  printf ("-------------------------------------------------------------\n");
  printf ("checking index operator []:\n");
  printf ("  The answer is %d!\n", c[99]);
  printf ("-------------------------------------------------------------\n");
  printf ("calls with macros expansions as argument:\n");
  bool b = (c != SOME_MACRO(42));
  printf ("  operator !=\n");
  c + SOME_MACRO(42);
  printf ("  operator +\n");
  f1(SOME_MACRO(0));
  printf ("  ordinary function\n");
  MY_ASSERT(b==true);
  printf ("  call in macro\n");

  return 0;
}

aspect CallTracer {
  pointcut builtin_ops() = call("C &operator*(C *)") || call("int operator+(int,int)")
    || call("int &operator =(int &,int)") || call("C *operator &(C &)") || call("Data *operator &(Data &)");
  advice call ("% ...::%(...)") && !call ("% printf(...)") && ! builtin_ops() : around () {
    printf ("before %s", tjp->signature ());
    tjp->proceed ();
  }
};

aspect CallTracer2 {
  pointcut builtin_ops() = call("C &operator*(C *)") || call("int operator+(int,int)")
    || call("int &operator =(int &,int)") || call("C *operator &(C &)") || call("Data *operator &(Data &)");
  advice call ("% ...::%(...)") && !call ("% printf(...)") && ! builtin_ops() : before () {
    printf ("\n");
  }
};

aspect CallWithClassPtrTest {
  advice call ("% %::%(C *)") : before () {
    printf ("before %s\n", JoinPoint::signature ());
  }
};

// special treatment of operator % and operator %=:
// in a match expression we use %% and %%=, because % is our wildcard
aspect CallModuloTest {
  advice call ("% ...::operator %%(...)" ||
	       "% ...::operator %%=(...)") : before () {
    printf ("before modulo operator %s\n", JoinPoint::signature ());
  }
};
