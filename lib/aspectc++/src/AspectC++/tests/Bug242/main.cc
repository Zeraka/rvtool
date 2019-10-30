#include "OutputStream.h"

OutputStream out;

int main () {
  printf ("Bug242: wrong include guard generation\n");
  printf ("======================================\n");
  printf ("The introduced fill char is '");
  out.print_fill ();
  printf ("'\n");
  printf ("======================================\n");
}
