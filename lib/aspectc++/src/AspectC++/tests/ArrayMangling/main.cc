#include <stdio.h>

class C {};

void f (int a1[],
        int a2[2],
        int *a3[3],
	int (*a4)[4],
	int a5[5][10],
	int (*(*a6)())[3],
	int (*a7[7])[14],
	double (C::*a8)(long *(a[32])[64]) = 0) {
}

aspect Nonsense {
  advice execution ("% f(...)") : before () {
    printf ("%s\n", tjp->signature ());
  }
};

int main () {
  int a1[] = { 1, 2 };
  int a2[2] = { 3, 4 };
  int *a3[3];
  int (*a4)[4] = 0;
  int a5[5][10];
  int (*(*a6)())[3] = 0;
  int (*a7[7])[14];
  printf ("ArrayMangling\n");
  printf ("=============================================================\n");
  printf ("this is a quite complicated signature with array types\n");
  f (a1, a2, a3, a4, a5, a6, a7);
  printf ("=============================================================\n");
  return 0;
}
