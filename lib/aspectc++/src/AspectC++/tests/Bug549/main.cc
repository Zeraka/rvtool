#include <stdio.h>

void void_int_func(int arg) {}

void (&get_func_ref())(int) {
  return void_int_func;
}

void (*get_func_pointer())(int) {
  return &void_int_func;
}

int (&get_array_ref())[10] {
  static int an_array[10];
  return an_array;
}

int main() {
  printf ("Bug549: exec joinpoint with function-reference-return-type\n");
  printf ("==========================================================\n");
  get_func_ref();
  get_func_pointer();
  get_array_ref();
}

aspect TestAspect {  
  advice execution("% get_%()") : before() {
    printf ("executing %s\n", JoinPoint::signature ());
  }  
};
