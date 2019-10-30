#include <stdio.h>

template < int I > class mySimpleTemplateClass {
 private:
  int myArray[I];
 public:
  int getArraySize() { return I; }
};

aspect my_trace_call {
  advice call("mySimpleTemplateClass<...>") : before () {
    printf("my_trace_call CALLING: %s\n",JoinPoint::signature());
  }  
};

int main () {
  mySimpleTemplateClass<10> t;
  t.getArraySize ();
}
