#include <stdio.h>

class myClass {
public:
  void method(void);

  void inlineMethod(void) {
    printf("  in myClass::inlineMethod()\n");
  }
};


void myClass::method(void) {
  printf("  in myClass::method()\n");
}

aspect myClassTracer {

  pointcut jps () =
    call ("% myClass::%(...)") || execution ("% myClass::%(...)");

  advice jps () : before () {
    printf ("  -> before advice 1 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
  }

  advice jps () : around () {
    printf ("  -> around advice 1 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
    printf ("  >>>\n");
    tjp->proceed ();
    printf ("  <<<\n");
  }

  advice jps () : after () {
    printf ("  -> after advice 1 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
  }

  advice jps () : before () {
    printf ("  -> before advice 2 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
  }

  advice jps () : around () {
    printf ("  -> around advice 2 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
    printf ("  >>>\n");
    tjp->proceed ();
    printf ("  <<<\n");
  }

  advice jps () : after () {
    printf ("  -> after advice 2 for joinpoint \"%s\"\n",
	    JoinPoint::signature ());
  }
};

int main () {
  myClass mc;

  printf ("VoidArg: checks advice for functions with void argument (#206)\n");
  printf ("==============================================================\n");
  mc.inlineMethod ();
  mc.method ();
  printf ("==============================================================\n");
  return 0;
}
