#include <stdio.h>

int main () {
  void *array[] = { &&l1, &&l2, &&l3 };

#ifndef __cplusplus
  const int array2[] = { &&l1 - &&l1, &&l2 - &&l1, &&l3 - &&l1 };
#endif
  
  void *ptr = &&l2;
  goto *ptr;

 l1:
  printf ("l1\n");
#ifndef __cplusplus
  goto *(&&l1 + array2[2]);
#else
  goto end;
#endif
 l2:
  printf ("l2\n");
  goto *array[0];
 l3:
  printf ("l3\n");
  goto end;

 end:
  printf ("end reached\n");
}
