#include <stdio.h>

class Base {
public:
  virtual int a(int i, float b) {
    printf("inside Base::a(%d, %f)\n", i, b);
    return i;
  }
  virtual int b(int p, int q);
  virtual void c () = 0;
};

int Base::b (int p, int q) {
  printf("inside Base::b(%d, %d)\n", p, q);
  return p + q;
}

class Derived : public Base {
public:
  virtual int a(int i, float b) {
    printf("inside Derived::a(%d, %f)\n", i, b);
    return i;
  }
  virtual int b(int p, int q);
  void c () {
    printf("inside Derived::c()\n");
  }
};

int Derived::b (int p, int q) {
  printf("inside Derived::b(%d, %d)\n", p, q);
  return p + q;
}


int main() {
  Base *obj = new Derived ();
  printf ("AdviceVirtualFct\n");
  printf ("=============================================================\n");
  printf ("first we cast 'obj' to 'Derived': here should be *no* advice\n");
  ((Derived*)obj)->a (1, 2);
  printf ("-------------------------------------------------------------\n");
  printf ("now we call 'b()' on 'obj': *only call* advice should be run\n");
  obj->b (3, 4);
  printf ("-------------------------------------------------------------\n");
  printf ("a call 'Base::b()' fully qualified: *both* call and exec\n");
  ((Derived*)obj)->Base::b (5, 6);
  printf ("-------------------------------------------------------------\n");
  printf ("a call to a pure virtual function: *only call* advice\n");
  obj->c ();
  printf ("=============================================================\n");
  delete obj;
}
