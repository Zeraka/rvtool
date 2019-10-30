#include <stdio.h>

struct X {
    X () {}
};

struct C1 {
    void x () {
	printf ("  void C1::x()\n");
    }
    void f () {
	printf ("  void C1::f()\n");
    }
    void f (double) {
	printf ("  void C1::f(double)\n");
    }
    void f (const X&) {
	printf ("  void C1::f(X&)\n");
    }
};

struct C2 : C1 {
    void x () {
	printf ("  void C2::x()\n");
    }
    void f () {
	printf ("  void C2::f()\n");
    }
    void f (double) {
	printf ("  void C2::f(double)\n");
    }
    void f (const X&) {
	printf ("  void C2::f(X&)\n");
    }
};

struct C3 : C2 {
    void x () {
	printf ("  void C3::x()\n");
    }
    void f () {
	printf ("  void C3::f()\n");
    }
    void f (double) {
	printf ("  void C3::f(double)\n");
    }
    void f (const X&) {
	printf ("  void C3::f(X&)\n");
    }
};

aspect Tracer {
    advice execution (derived ("C2")) : before () {
	printf ("  before derived(\"C2\")\n");
    }
    advice execution (derived ("% C2::f(double)")) : before () {
	printf ("  before derived(\"%% C2::f(double)\")\n");
    }
    advice execution (derived ("% C2::f(X&)")) : before () {
	printf ("  before derived(\"%% C2::f(X&)\")\n");
    }
};

int main() {
    C3 c;
    printf ("Derived: test the 'derived' pointcut function:\n");
    printf ("==============================================\n");
    c.C1::f (3.0);
    printf ("----------------------------------------------\n");
    c.C1::f ();
    printf ("----------------------------------------------\n");
    c.C2::f (3.0);
    printf ("----------------------------------------------\n");
    c.C3::f ();
    printf ("----------------------------------------------\n");
    c.C3::f (X ());
    printf ("----------------------------------------------\n");
    c.C3::x ();
    printf ("==============================================\n");
}
