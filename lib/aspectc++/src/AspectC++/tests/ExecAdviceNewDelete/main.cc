#include <stdio.h>
#include <stdlib.h>
#include <new>

void *operator new (size_t ) throw(std::bad_alloc);

void *operator new (size_t s) throw(std::bad_alloc) {
  return malloc (s);
}

void *operator new[] (size_t s) throw(std::bad_alloc) {
  return malloc (s);
}

void operator delete (void *obj) throw() {
  free (obj);
}

void operator delete[] (void *obj) throw() {
  free (obj);
}

struct A { // bug 517
  /*static*/ void operator delete(void* ptr) throw (); // bug 517 was: fails without 'static'
};

void A::operator delete (void* ptr) throw() {}


class C {
public:
  void f () {}

  // we always use the same C instance
  void *operator new (size_t) {
    static C c;
    return &c;
  }
  void operator delete (void *) {
  }

};

#define STRING(x) #x

aspect HeapSurveillance {
  advice execution ("% ...::operator new(...)" ||
		    "% ...::operator delete(...)" ||
		    "% ...::operator new[](...)" ||
		    "% ...::operator delete[](...)") : before () {
    printf ("executing heap operation \"%s\"\n", JoinPoint::signature ());
    printf ("  tjp->that() is %s (should be 0)\n", (tjp->that () ? "not 0" : "0"));
  }
};

int main () {
  printf ("ExecAdviceNewDelete: execution advice for heap operations\n");
  printf ("=========================================================\n");
  C *c = new C;
  c->f ();
  delete c;
  delete new int;
  delete[] new double[20];
  delete new A;
  printf ("=========================================================\n");
}
