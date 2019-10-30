#include <stdio.h>

// --------------------------- Ordering code ---------------------------

aspect Order_A1_A2_and_A3 {
  advice execution("% C1::run2(...)") : order(base ("A3")||"A3","A1"||"A2");
};

aspect Order_A1_and_A3 {
  advice execution("% C1::run2(...)") : order("A3", "A1");
  advice "C1" : order("A3", "A1"); // order for intros into "C1"
};

aspect Order_A2_first {
  advice execution("% C1::run3(...)") : order("A2", !"A2");
};

// --------------------------- Aspect code -----------------------------

aspect A1 {
  advice execution("void %::run%()") : after() {
    printf("A1::after\n");
  }
  advice execution("void %::run%()") : before() {
    printf("A1::before\n");
  }
  // type INT comes from A3 -> A3 needs higher precedence
  advice "C1" : slice struct { INT i; };
};

aspect A2 {
  advice execution("void %::run%()") : before() {
    printf("A2::before\n");
  }
  advice execution("void %::run%()") : after() {
    printf("A2::after\n");
  }
};

aspect A3Base {
  pointcut virtual jps () = 0;
  advice execution("void %::run%()") : after() {
    printf("A3Base::after\n");
  }
  advice execution("void %::run%()") : around() {
    printf("A3Base::around 1\n");
    tjp->action ().trigger ();
  }
  advice execution("void %::run%()") : around() {
    printf("A3Base::around 2\n");
    tjp->proceed ();
  }
};

aspect A3 : public A3Base {
  pointcut jps () = execution("void %::run%()");
  advice jps (): after() {
    printf("A3::after last\n");
  }
  advice jps () : after() {
    printf("A3::after first\n");
  }
  advice "C1" : slice struct { typedef int INT; };
};

// ------------------------------- normal C++ ---------------------------- 
class C1 {
public:
  void run()  { printf("run\n"); }
  void run2() { printf("run2\n"); }
  void run3() { printf("run3\n"); }
};

int main () {
  C1 c1;
	 
  c1.run();
  printf ("\n\n");
  c1.run2();
  printf ("\n\n");
  c1.run3();
}
