#include <iostream>
#include <inttypes.h>
using namespace std;

// =====================================================================
// some preliminary functions for generic printing ... to be extended
template <typename T, bool AVAILABLE=(bool)AC::TypeInfo<T>::AVAILABLE>
struct PrinterSelector;
template <typename T, int BASES_LEFT = AC::TypeInfo<T>::BASECLASSES>
struct PrintBaseClasses;

void print (bool c) { cout << (c ? "true" : "false"); }
void print (unsigned char c) { cout << (int)c; }
void print (int c) { cout << c; }
void print (float f) { cout << f; }
void print (const char *c) { if (c) cout << "\"" << c << "\""; else cout << "<null>"; }
template<typename T> void print (T &c) {
  cout << "{";
  PrinterSelector<T>::print (c);
  cout << " }";
}
template<typename T> void print (T *c) {
  cout << "-> ";
  print (*c);
}

void print (AC::Protection prot) {
  switch (prot) {
  case AC::PROT_PRIVATE: cout << " private"; return;
  case AC::PROT_PROTECTED: cout << " protected"; return;
  case AC::PROT_PUBLIC: cout << " public"; return;
  default: break;
  }
}

void print (AC::Specifiers spec) {
  if (spec & AC::SPEC_STATIC) cout << " static";
  if (spec & AC::SPEC_MUTABLE) cout << " mutable";
  if (spec & AC::SPEC_VIRTUAL) cout << " virtual";
}


template <typename T>
struct AP {
    static void print (T c) { ::print (c); }
};
template<typename T, unsigned int N> struct AP<T[N]> {
    template <typename T2>
    static void print (T2 *c) {
	cout << "{ ";
	for (unsigned int i = 0; i < N; ++i) {
	    cout << "[" << i << "]=";
	    ::print (c[i]);
	    cout << " ";
	}
	cout << "}";
    }
};

template<typename T, int I = AC::TypeInfo<T>::MEMBERS> struct _Printer {
  static void print (T &c) {
      _Printer<T,I-1>::print (c);
      typedef typename AC::TypeInfo<T>::template Member<I-1> Member;
      ::print (Member::prot);
      ::print (Member::spec);
      cout << " " << Member::name () << "=";
      AP<typename Member::ReferredType>::print(*Member::pointer(&c));
    }
};
template<typename T> struct _Printer<T,0> {
  static void print (T &c) {}
};

template <typename T, bool AVAILABLE>
struct PrinterSelector {
  static void print (T &obj) {
    PrintBaseClasses<T>::print (obj);
    _Printer<T>::print (obj);
  }
};
template <typename T> struct PrinterSelector<T, false> {
  static void print (T &) {
    cout << " no type info";
  }
};

template <typename T, int BASES_LEFT>
struct PrintBaseClasses {
  static void print (T &obj) {
    PrintBaseClasses<T, BASES_LEFT - 1>::print (obj);
    cout << "{ baseclass";
    typedef typename AC::TypeInfo<T>::template BaseClass<BASES_LEFT - 1> Base;
    ::print (Base::prot);
    ::print (Base::spec);
    cout << ":";
    _Printer< typename Base::Type >::print (obj);
    cout << "}";
  }
};

template <typename T>
struct PrintBaseClasses<T, 0> {
  static void print (T &obj) {}
};


// =====================================================================

// The test code itself ...
// Led.h
typedef unsigned char uint8_t;

class LED0 {
public:
        volatile uint8_t _pad[0x2];

        // PINA
        volatile uint8_t
                LEDPIN  : 1,
                                : 7;

        // DDRA
        volatile uint8_t
                LEDDDR  : 1,
                                : 7;

        // PORTA
        volatile uint8_t
                LEDPORT : 1,
                                : 7;
};

aspect Led {
        pointcut ledfunc() = "LED%";

        advice ledfunc() : slice class S {
        public:
                S() {
                        LEDPORT = 0;
                        LEDDDR  = 1;
                }

                void on() {
                        LEDPORT = 1;
                }

                void off() {
                        LEDPORT = 0;
                }
        };
};

struct C {
    int i;
    float j;
    const char *k;
};

C inner_c = { 43, 2.1, 0 };
C *cptr = &inner_c;

// inheritance ...
struct Base1 {
  int i;
};

struct Base2 {
  float j;
};

struct Derived : public Base1, public virtual Base2 {
  const char *k; // non-const
  //int* const c; // const
  const int c2; // const
  void cee() const {} // const
  const int zee() { return 0; } // non-const
  Derived() : /*c((int*)this),*/ c2(0xCAFEBABE) {}
};

// arbitrary members ...
struct D {
    mutable bool foo;
    int id;
    float size;
    const char *name;
    C **c;
    struct Inner {
      int inner;
    } i;
    int array[5];
    static float dd;
    void f() {}
} d = {true, 42, 1.8, "Olaf", &cptr, { 42 }, { 1,2,3 }};

float D::dd = 3.14;

struct F; // an incomplete type
LED0 l0; // constructed type with bit field and pad bytes

aspect TypeExtensions {
  advice "D": slice struct SL1 {
    bool b;
    JoinPoint::Member<0>::ReferredType same_as_member_0;
  };
  advice execution("% D::f()") : before () {
    //    typename JoinPoint::That::Inner ii;
    JoinPoint::That *ptr;
  }
};

// template members as introduction
namespace Test {
template<typename T>
class A {};
};

class B {};

aspect Introducer {
  advice "B" : slice class { Test::A<JoinPoint> a; };
};

// -------
// Bug 469: enums are counted for MEMBERS, but not treated in the list itself

struct Bug469 {
  enum { VAL = 1 };
  int a;
} bug469 = { 123 };

// -------
// Bug 474: segfault; fine if it compiles

// template<typename T >
// class Per_cpu {};

// class Jdb 
// {
//   typedef enum
//     {
//       SS_NONE=0, SS_BRANCH, SS_RETURN
//     } Step_state;

//   static Per_cpu<Step_state> ss_state;
// };

// -------
// Bug 475: introspection code in extern "C" structs
//          -> fine if it compiles

extern "C" {
  struct A {
    int a;
  };
}

// -------
// Bug 525: introspection code for members that are instances of anonymous types
//          -> fine if it compiles

class Bug525_1 {
  struct
  {
    bool     hit  : 1;
    unsigned res  : 6;
  } _flags;
  typedef struct { int i; } IStruct;
  IStruct _is;
};

class Bug525_2 {
union
    {
      struct
	{
	  unsigned long _space:32;
	  unsigned long address:20;
        } __attribute__((packed)) data;
      int *_submap;
    } __attribute__((packed));
};

// don't de-anonymize typedef'd classes
class Bug525_3 {
public:
  Bug525_3 () {}
};

typedef class { Bug525_3 _attr; } Bug525_4;
Bug525_4 global_obj;

struct {
  Bug525_4 _obj;
  int _bla;
} typedef Bug525_5;

void f(const Bug525_5 &p) {}
aspect Bug525_Check {
  advice execution("void f(const %&)") : before () {}
};

//
// Bug reported on problems compiling Gem5
//
// De-Anonymize class with a base class
// Fine if it compiles
struct YYY {};

class BitfieldUnderlyingClassesExtMachInst {
  union {
    class : public YYY {
    } puswl;
  };
};

// -------
// Bug 546:  Type hiding by a non-type declaration
//           -> requires elaborated type specifier
struct settings {
  int hashpower_init;
};

class Memcached {
  static struct settings settings;
  const volatile struct settings hmpf;
  void (Memcached::*wr)(char const *, int, unsigned & );
};

settings Memcached::settings;

// -------
// Bug 524:  handle ambiguous type names
//

// case 1
namespace Addr {

template< int SHIFT >
struct Addr {};

} // namespace Addr

class Virt_addr
: public Addr::Addr<42> {};

// case 2
class Page
{
  typedef int Type;
  Type t;
};

// case 3 (sanity check)
namespace Inner {
  struct T { uint32_t a; };
}

class C2 {
  public:
  class Inner {
    public: struct T { uint8_t a; };
  };
  Inner::T my_t;
  ::Inner::T my_t2;
};

// ======

int main () {
    C c;
    c.i = 42;
    c.j = 3.14;
    c.k = "Hallo";
    cout << "Introspection: Test AC::TypeInfo<typename>" << endl;
    cout << "==================================================" << endl;
    print (c); cout << endl;
    print (&d); cout << endl;
    print ((F*)0); cout << endl;
    print (cout); cout << endl;
    print (42); cout << endl;
    print (l0); cout << endl;
    cout << "--------------------------------------------------" << endl;
    Derived d;
    d.i = 31;
    d.j = 2.71;
    d.k = "Christoph";
    print (d); cout << endl;
    cout << "-Bug-469------------------------------------------" << endl;
    print (bug469); cout << endl;
    cout << "-Bug-525------------------------------------------" << endl;
    cout << "#members of Bug525_1: " << AC::TypeInfo<Bug525_1>::MEMBERS << endl;
    cout << "#members of Bug525_2: " << AC::TypeInfo<Bug525_2>::MEMBERS << endl;
    f (Bug525_5 ());
    cout << "Bug524: must be 1: " << sizeof(AC::TypeInfo<C2>::Member<0>::Type) << endl;
    cout << "Bug524: must be 4: " << sizeof(AC::TypeInfo<C2>::Member<1>::Type) << endl;
    cout << "==================================================" << endl;
}
