#include <stdio.h>

// this was only used for checking the match expression parser
// aspect MatchExprTest {
//   // declaration specifier sequences
//   advice call ("% foo(...)") : before () {}
//   advice call ("const % foo(...)") : before () {}
//   advice call ("int foo(...)") : before () {}
//   advice call ("unsigned int foo(...)") : before () {}
//   advice call ("const char *foo(...)") : before () {}
//   advice call ("const char*foo(...)") : before () {}
//   advice call ("Bar::% foo(...)") : before () {}
//   advice call ("...::% foo(...)") : before () {}
//   advice call ("...:: X% ::% foo(...)") : before () {}
//   advice call ("const ... :: Bar &foo(...)") : before () {}
//   // pointer operators
//   advice call ("int *& foo ()") : before () {}
//   advice call ("int Bar::* & foo ()") : before () {}
//   advice call ("int * const volatile foo ()") : before () {}
//   advice call ("bool ... :: * foo ()") : before () {}
//   // nested declarators
//   advice call ("const char *(*foo)(...)") : before () {}
//   advice call ("const char *( * foo )(...)") : before () {}
//   // qualified names
//   advice call ("const char unsigned *(*Bar::foo)(...)") : before () {}
//   advice call ("const char *( * ... :: Foo :: foo )(...)") : before () {}
//   // argument lists
//   advice call ("% foo()") : before () {}
//   advice call ("% foo(int)") : before () {}
//   advice call ("% foo(%,int,...)") : before () {}
//   advice call ("% foo(int, double (*)())") : before () {}
//   advice call ("% %::foo(double (*)() const) const volatile") : before () {}
//   // array stuff
//   advice call ("% a[10]") : before () {}
//   advice call ("% a[10][ 15 ]") : before () {}
//   advice call ("% a[%][%]") : before () {}
//   // classes
//   advice call ("%") : before () {}
//   advice call ("A%") : before () {}
//   advice call ("%B") : before () {}
//   advice call ("A%B") : before () {}  
//   advice call ("A%B%C%") : before () {}  
//   // various types
//   advice call ("bool") : before () {}
//   advice call ("char") : before () {}
//   advice call ("signed char") : before () {}
//   advice call ("char unsigned") : before () {}
//   advice call ("short") : before () {}
//   advice call ("signed short") : before () {}
//   advice call ("unsigned short") : before () {}
//   advice call ("int") : before () {}
//   advice call ("signed int") : before () {}
//   advice call ("unsigned int") : before () {}
//   advice call ("signed") : before () {}
//   advice call ("unsigned") : before () {}
//   advice call ("wchar_t") : before () {}
//   advice call ("long") : before () {}
//   advice call ("signed long") : before () {}
//   advice call ("unsigned long") : before () {}
//   advice call ("long long") : before () {}
//   advice call ("long signed long") : before () {}
//   advice call ("unsigned long long") : before () {}
//   advice call ("float") : before () {}
//   advice call ("double") : before () {}
//   advice call ("long double") : before () {}
//   advice call ("void") : before () {}
//   // types with declarator
//   advice call ("void *") : before () {}
//   advice call ("void (*)(...)") : before () {}
//   // invalid advice call ("void (...) const") : before () {}
// };

class C {
  int _i;
public:
  C (int i) : _i (i) { printf ("CONS C(int) is matched by ...\n"); }
  void f () { printf ("FUNC void C::f () is matched by ...\n"); }
  void f () const { printf ("FUNC void C::f() const is matched by ...\n"); }
  void f () volatile { printf ("FUNC void C::f() volatile is matched by\n"); }
  void f () const volatile { printf ("FUNC void C::f() const volatile is matched by\n"); }
};

const char r1 () { printf ("FUNC const char r1 () is matched by ...\n"); return 0; }
const float r2 () { printf ("FUNC const float r2 () is matched by ...\n"); return 0; }
char r3 () { printf ("FUNC char r3 () is matched by ...\n"); return 0; }
float r4 () { printf ("FUNC float r4 () is matched by ...\n"); return 0; }

char * p1 () { printf ("FUNC char * p1 () is matched by ...\n"); return 0; }
char * const p2 () { printf ("FUNC char * const p2 () is matched by ...\n"); return 0; }
char * volatile p3 () { printf ("FUNC char * volatile p3 () is matched by ...\n"); return 0; }
char * const volatile p4 () { printf ("FUNC char * const volatile p4 () is matched by ...\n"); return 0; }

aspect QualCheck {
  advice execution ("% C::f()") : after () {
    printf ("'%% C::f()'\n");
  }
  advice execution ("% C::f() const") : after () {
    printf ("'%% C::f() const'\n");
  }
  advice execution ("% C::f() volatile") : after () {
    printf ("'%% C::f() volatile'\n");
  }
  advice execution ("% C::f() const volatile") : after () {
    printf ("'%% C::f() const volatile'\n");
  }
  advice execution ("const % %()") : after () {
    printf ("'const %% %%()'\n");
  }
  advice execution ("float %()") : after () {
    printf ("'float %%()'\n");
  }
  advice execution ("const float %()") : after () {
    printf ("'const float %%()'\n");
  }
  advice execution ("% * %()") : after () {
    printf ("'%% * %%()'\n");
  }
  advice execution ("char * const %()") : after () {
    printf ("'char * const %%()'\n");
  }
  advice execution ("% * volatile %()") : after () {
    printf ("'%% * volatile %%()'\n");
  }
  advice execution ("% * volatile const %()") : after () {
    printf ("'%% * volatile const %%()'\n");
  }
};

namespace N1 {
  namespace N2 {
    class Inside {
    public:
      void o() { printf ("FUNC void N1::N2::Inside::o () is matched by ...\n"); }
      void p() { printf ("FUNC void N1::N2::Inside::p () is matched by ...\n");}
    };
  }
}

namespace N3 {
  void o() { printf ("FUNC void N3::o () is matched by ...\n"); }
  void p() { printf ("FUNC void N3::p () is matched by ...\n"); }
}

void o() { printf ("FUNC void o () is matched by ...\n"); }
void p() { printf ("FUNC void p () is matched by ...\n"); }

aspect DotCheck {
  advice execution ("void N1::N2::Inside::o ()") : after () {
    printf ("'void N1::N2::Inside::o ()'\n");
  }
  advice execution ("void N1::...::o ()") : after () {
    printf ("'void N1::...::o ()'\n");
  }
  advice execution ("void ...::N1::...::o ()") : after () {
    printf ("'void ...::N1::...::o ()'\n");
  }
  advice execution ("void ...::N2::...::% ()") : after () {
    printf ("'void ...::N2::...::%% ()'\n");
  }
  advice execution ("void N1::...::Inside::...::o ()") : after () {
    printf ("'void N1::...::Inside::...::o ()'\n");
  }
  advice execution ("void p ()") : after () {
    printf ("'void p ()'\n");
  }
};

aspect AnyCheck {
  advice execution ("% %(...)") : after () {
    printf ("'%% %%(...)'\n");
  }
  advice execution ("% ...::%(...)") : after () {
    printf ("'%% ...::%%(...)'\n");
  }
};

void conv1 () { printf ("FUNC void conv1 () is matched by ...\n"); }
void conv1 (float) { printf ("FUNC void conv1 (float) is matched by ...\n"); }
void conv1 (void (*)(int)) { printf ("FUNC void conv1 (void (*)(int)) is matched by ...\n"); }
void conv1 (float *) { printf ("FUNC void conv1 (float *) is matched by ...\n"); }
// code generation problem here: void conv2 (void) { printf ("FUNC void conv2 (void) is matched by ...\n"); }
void conv2 (const float) { printf ("FUNC void conv2 (const float) is matched by ...\n"); }
void conv2 (void (int)) { printf ("FUNC void conv2 (void (int)) is matched by ...\n"); }
void conv2 (float [32]) { printf ("FUNC void conv2 (float [32]) is matched by ...\n"); }
aspect ConvWarningCheck {
  // => argument list '(void)' converted to '()'
  advice execution ("void conv%(void)") : after () {
    printf ("'void conv%%(void)'\n");
  }
  // => qualifier removed from argument
  advice execution ("void conv%(const float)") : after () {
    printf ("void conv%%(const float)''\n");
  }
  // => function argument adjusted to pointer to function
  // parser syntax: advice execution ("void conv%(void (int))") : after () {
  //   printf ("'void conv%%(void (int))'\n");
  // }
  // => array argument adjusted to pointer
  advice execution ("void conv%(float[32])") : after () {
    printf ("'void conv%%(float[32])'\n");
  }
};

class Op {
public:
  operator double* () {
    printf ("FUNC Op::operator double* () is matched by ...\n");
    return 0;
  }
  Op &operator = (const Op &op) { 
    printf ("FUNC Op &Op::operator =(const Op &) is matched by ...\n");
    return *this; 
  }
  void operator << (int) {
    printf ("FUNC void Op::operator <<(int) is matched by ...\n");
  }
};

Op& operator + (Op& o1, const Op& o2) {
  printf ("FUNC Op &operator + (Op&, const Op&) is matched by ...\n");
  return o1;
}

aspect OperatorCheck {
  advice execution ("% operator %(...)") : after () {
    printf ("'%% operator %%(...)'\n");
  }
  advice execution ("operator %(...)") : after () {
    printf ("'operator %%(...)'\n");
  }
  advice execution ("% operator double*()") : after () {
    printf ("'operator double*()'\n");
  }
  advice execution ("Op& operator +(Op&,Op&)") : after () {
    printf ("'Op& operator +(Op&,Op&)'\n");
  }
  advice execution ("% %::operator %(...)") : after () {
    printf ("'%% %%::operator %%(...)'\n");
  }
  advice execution ("% %::operator <<(...)") : after () {
    printf ("'%% %%::operator <<(...)'\n");
  }
};

enum Enum1 { E1, E2, E3 };
void fe1 (Enum1 enum1) {
  printf ("FUNC void fe1 (Enum1) is matched by ...\n");
}
class EC {
public:
  enum Enum2 { E4 = E3 + 1, E5 };
  static void fe2 (Enum2 enum1) {
    printf ("FUNC static void EC::fe2 (EC::Enum2) is matched by ...\n");
}
};

aspect EnumCheck {
  advice execution ("void fe1 (Enum1)") : after () {
    printf ("'void fe1 (Enum1)'\n");
  }
  advice execution ("void EC::fe2 (EC::Enum2)") : after () {
    printf ("'void EC::fe2 (EC::Enum2)'\n");
  }
  advice execution ("void fe2 (Enum2)") : after () {
    printf ("'void fe2 (Enum2)'\n");
  }
  advice execution ("void fe% (Enum%)") : after () {
    printf ("'void fe%% (Enum%%)'\n");
  }
};

struct S { static void s() {} };
void ss() {}

template <void (*FUNC)()> struct T {};

T<&S::s> t1;
T<&ss> t2;

void x1(T<&S::s>) {
  printf ("FUNC void void x1(T<&S::s>) is matched by ...\n");
}
void x2(T<&ss>) {
  printf ("FUNC void void x2(T<&ss>) is matched by ...\n");
}

aspect TemplAddrCheck {
  advice execution("void x%(%)") : after () {
    printf ("void x%%(%%)\n");
  }
  advice execution("void x%(T<%>)") : after () {
    printf ("void x%%(T<%%>)\n");
  }
  advice execution("void x%(T<&%>)") : after () {
    printf ("void x%%(T<&%%>)\n");
  }
  advice execution("void x%(T<&...::%>)") : after () {
    printf ("void x%%(T<&...::%%>)\n");
  }
  advice execution("void x%(T<&%::%>)") : after () {
    printf ("void x%%(T<&%%::%%>)\n");
  }
};

int main () {
  printf ("MatchExpr: match expression semantics implemented correctly?\n");
  printf ("==============================================================\n");
  C c (1);
  const C cc (2);
  volatile C cv (3);
  const volatile C ccv (4);
  printf ("const/volatile function qualifiers:\n");
  printf ("-----------------------------------\n");
  c.f ();
  cc.f ();
  cv.f ();
  ccv.f ();
  printf ("==============================================================\n");
  printf ("qualified names with '...'\n");
  printf ("--------------------------\n");

  N1::N2::Inside i;
  i.o ();
  i.p ();
  N3::o ();
  N3::p ();
  o ();
  p ();

  printf ("==============================================================\n");
  printf ("const / non-const function results\n");
  printf ("----------------------------------\n");
  r1 ();
  r2 ();
  r3 ();
  r4 ();
  printf ("==============================================================\n");
  printf ("const / volatile pointers\n");
  printf ("-------------------------\n");
  p1 ();
  p2 ();
  p3 ();
  p4 ();
  printf ("==============================================================\n");
  printf ("check parameter type adjustments\n");
  printf ("--------------------------------\n");
  conv1 ();
  conv1 (3.14f);
  // NYI: conv1 ((void (*)(int))0);
  conv1 ((float*)0);
  // code generation problem with advice for  conv2 ();
  conv2 (3.14f);
  // NYI: conv2 ((void (*)(int))0);
  conv2 ((float*)0);
  printf ("==============================================================\n");
  printf ("fancy operators\n");
  printf ("---------------\n");
  Op op1, op2, op3;
  double *ptr = (double*)op2;
  op3 = op1 + op2;
  op3 << 2;
  printf ("==============================================================\n");
  printf ("check for enumerations in match expressions\n");
  printf ("-------------------------------------------\n");
  fe1 (E2);
  EC::fe2 (EC::E4);
  printf ("==============================================================\n");
  printf ("check for addresses in template argument lists in match exprs\n");
  printf ("-------------------------------------------------------------\n");
  x1 (t1);
  x2 (t2);
  printf ("==============================================================\n");
  printf ("FUNC main () is matched by ...\n");
  return 0;
}
