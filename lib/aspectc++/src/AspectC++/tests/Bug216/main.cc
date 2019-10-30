#include <stdio.h>

aspect introductions {
  pointcut pTarget() = "IntroductionTarget";

 public: 

  advice pTarget() : slice struct {
    struct myStruct {
      int member1;
      long member2;
      float member3;
    };
  };

  advice pTarget() : slice struct { int myArray1[10]; };
  advice pTarget() : slice struct { myStruct myArray2[10]; };

};

class IntroductionTarget {
};

aspect abstract_trace_construction {
  pointcut virtual pClasses() = 0;

 private:
  advice construction(pClasses()) : before () {
    printf("in %s\n",JoinPoint::signature());
  }  
};

aspect trace : public abstract_trace_construction {
  pointcut pClasses() = "IntroductionTarget";
};

int main () {
  printf ("Bug 216: copy constructor generation\n");
  printf ("====================================\n");
  IntroductionTarget it;
  printf ("====================================\n");
  return 0;
}
