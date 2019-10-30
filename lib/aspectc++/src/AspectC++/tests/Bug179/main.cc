#include <stdio.h>

aspect AroundMain {
  advice execution ("% main(...)") : around () {
    printf ("Bug179: advice for main without return statement\n");
    printf ("================================================\n");
    tjp->proceed ();
    printf ("================================================\n");
  }
};

int main() {
  printf ("in main\n");
  // note: here is no return, which is perfectly valid C/C++.
}
