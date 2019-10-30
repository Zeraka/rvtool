#include <stdio.h>

aspect A {
  int _obj_id;
  static int _obj_count;
  advice "C": slice struct { A _aspect_instance; };
  static A *aspectof () { 
    JoinPoint::That *that = tjp->that ();
    return &that->_aspect_instance;
  }
  advice construction ("C"): before () {
    _obj_id = ++_obj_count;
  }
  advice execution("C") : before () {
    printf ("ID %d: ", _obj_id);
  }
};

class C {
public:
  int a(int i, float b) {
    printf("inside A::a(%d, %f)\n", i, b);
    return i;
  }
  int c(int p, int q);
};

int C::c(int p, int q){
  printf("inside C::c(%d, %d)\n",p,q);
  return p;
}

int main() {
  C c1;
  C c2;
  printf ("AspectOf: non-default aspect instantiation test\n");
  printf ("===============================================\n");
  c1.a(4711, 3.14f);
  printf ("-----------------------------------------------\n");
  c2.c(4,5);
  printf ("-----------------------------------------------\n");
  c1.a(4712, 2.71f);
  printf ("===============================================\n");
}

int A::_obj_count = 0;
