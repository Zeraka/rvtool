#include "classA.h"
#include "classB.h"

int main() {
  classA classAObject;
  classB classBObject;

  printf ("Bug235: buggy project repository with abstract aspects\n");
  printf ("======================================================\n");

  classAObject.methodA();
  classBObject.methodB();

  printf ("======================================================\n");
  return 0;
}
