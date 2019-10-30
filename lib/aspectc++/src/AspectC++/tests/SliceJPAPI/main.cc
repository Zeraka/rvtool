#include <stdio.h>

template <typename JP, int N = JP::MEMBERS>
struct Introspector {
  static void print_names() {
    Introspector<JP, N-1>::print_names();
    printf ("member %d: %s\n", N, JP::template Member<N-1>::name());
  }
};

template <typename JP>
struct Introspector<JP, 0> {
  static void print_names() {}
};

struct Target {
  int i;
};

slice class Slice1 {
  int j;
public:
  static const int MEMBERS1 = JoinPoint::MEMBERS;
  static int get_elements();
};

slice int Slice1::get_elements() { return JoinPoint::MEMBERS; }
 
slice class Slice2 {
public:
  static const int MEMBERS2 = JoinPoint::MEMBERS;
  static const int BASECLASSES = JoinPoint::BASECLASSES;
  static void print_names ();
  static int get_delta() {
    return AC::TypeInfo<Slice1>::MEMBERS -JoinPoint::MEMBERS;
  }   
};

slice void Slice2::print_names() { Introspector<JoinPoint>::print_names (); }

int main () {
  printf ("SliceJPAPI: The static joinpoint API\n");
  printf ("====================================\n");
  printf ("Introspecting class/struct '%s'\n",
	  AC::TypeInfo<Target>::signature ());
  printf ("------------------------------------\n");
  printf ("MEMBERS 1: %d\n", Target::MEMBERS1);
  printf ("------------------------------------\n");
  printf ("BASECLASSES 2: %d\n", Target::BASECLASSES);
  printf ("MEMBERS 2: %d\n", Target::MEMBERS2);
  Target::print_names ();
  printf ("------------------------------------\n");
  printf ("increased no of members by %d\n", Target::get_delta ());
  Introspector<AC::TypeInfo<Target> >::print_names ();
  printf ("====================================\n");
};

aspect Foo {
  advice "Target" : slice Slice1;
  advice "Target" : slice Slice2;
};
