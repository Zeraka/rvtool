#include <stdio.h>


void func () {

   printf ("in func()\n");
}


class Class1
{
public:
  class Class2
 {
  public:
    void func1()
   {
      printf("in Class1::Class2::func1()\n");
   }
  } class2Instance;
};


int main (int argc, char * argv[]) {
  printf ("NestedClassFuncAdvice\n");
  printf ("=============================================================\n");
  func ();
  Class1 class1;
  class1.class2Instance.func1();
  printf ("=============================================================\n");
  return 0;
}
