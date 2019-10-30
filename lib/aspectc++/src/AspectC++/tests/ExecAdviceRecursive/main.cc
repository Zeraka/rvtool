#include <stdio.h>

int fak2 (int n);

namespace N {
    int fak3 (int n);
}

#ifdef XXXXXXXXXXXX
XYZ
#else
int fak1 (int n)
#endif
{
    return (n <= 1) ? 1 : n * fak1 (n - 1);
}
	
int fak2 (int n) {
    return (n <= 1) ? 1 : n * fak2 (n - 1);
}

int N::fak3 (int n) {
    return (n <= 1) ? 1 : n * fak3 (n - 1);
}

aspect Tracer {
    advice execution ("% ...::fak1(...)") : before () {
	printf ("before %s\n", JoinPoint::signature ());
    }
    advice execution ("% ...::fak2(...)") : around () {
	printf ("around-before %s\n", JoinPoint::signature ());
	tjp->proceed ();
	printf ("around-after  %s\n", JoinPoint::signature ());
    }
    advice execution ("% ...::fak3(...)") : after () {
	printf ("after  %s\n", JoinPoint::signature ());
    }
};

int main() {
  printf ("ExecAdviceRecursive: checks exec advice for recursive fcts\n");
  printf ("==========================================================\n");
  fak1 (2);
  fak2 (3);
  N::fak3 (4);
  printf ("==========================================================\n");
}
