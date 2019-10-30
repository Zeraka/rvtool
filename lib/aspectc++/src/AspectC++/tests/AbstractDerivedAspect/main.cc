#include <cstdio>
using std::printf;

aspect AbstractBase {
  pointcut virtual fn () = 0;
  advice execution (fn ()) : after () {
    printf ("=====================================\n");
  }
  virtual void dummy () = 0;
};

// this one is also abstract and is therefore allowed to be a base class
aspect Mediator : public AbstractBase {
  pointcut fn () = "% main(...)";
};

// this aspect should not be considered by Final although it defines fn
aspect Monster {
  pointcut fn () = "% bar(...)";
  virtual void dummyMonster () = 0;
};

aspect Final : public Monster, public Mediator {
  void dummy () {}
  void dummyMonster () {}
};

aspect AbstractDestructorBase {
public:
  // pure virtual destructor!
  virtual ~AbstractDestructorBase() = 0;
  advice execution ("% main(...)") : after () {
    printf ("=====================================\n");
  }
};

// oh what a hell: here is the definition of the pure virtual destructor
AbstractDestructorBase::~AbstractDestructorBase () {}


aspect WithImplicitDestructor : public AbstractDestructorBase {
};

int main () {
    printf ("AbstractDerivedAspect: check whether an aspect is abtract\n");
    printf ("... fine if this is underlined twice!\n");
    return 0;
}
