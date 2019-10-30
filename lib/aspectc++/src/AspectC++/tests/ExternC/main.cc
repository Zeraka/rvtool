#include <stdio.h>

extern "C" void b(char c, const char *str);

void b(char c, const char *str) {
  printf("b(%c, %s)\n", c, str);
}

extern "C" void d(char c, const char *str) {
  printf("d(%c, %s)\n", c, str);
  b (c, str);
}

extern "C" {
  void b2(char c, const char *str);

  void b2(char c, const char *str) {
    printf("b2(%c, %s)\n", c, str);
  }

  void d2(char c, const char *str) {
    printf("d2(%c, %s)\n", c, str);
    b2 (c, str);
  }
}

extern "C" {
  extern "C" {
    extern "C" void b3(char c, const char *str);

    extern "C" void b3(char c, const char *str) {
      printf("b3(%c, %s)\n", c, str);
    }

    extern "C" void d3(char c, const char *str) {
      printf("d3(%c, %s)\n", c, str);
      b3 (c, str);
    }
  }
}

aspect Trace {
  pointcut fcts() = "% b(...)" || "% d(...)" || "% b2(...)" || "% d2(...)" || "% b3(...)" || "% d3(...)";
  advice execution (fcts ()) : before () {
    printf ("exec \"%s\"\n", JoinPoint::signature ());
  }
  advice call (fcts ()) : before () {
    printf ("call \"%s\"\n", JoinPoint::signature ());
  }
};

int main() {
  printf ("ExternC: advice for call/exec of extern \"C\" functions\n");
  printf ("=======================================================\n");
  b('H', "ello World");
  printf ("-------------------------------------------------------\n");
  d('H', "ello World");
  printf ("-------------------------------------------------------\n");
  b2('H', "ello World");
  printf ("-------------------------------------------------------\n");
  d2('H', "ello World");
  printf ("-------------------------------------------------------\n");
  b3('H', "ello World");
  printf ("-------------------------------------------------------\n");
  d3('H', "ello World");
  printf ("=======================================================\n");
}
