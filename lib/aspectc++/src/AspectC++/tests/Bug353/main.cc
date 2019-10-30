void t(){}

#include <iostream>

int main () {
  std::cout << "Bug 353: first token shouldn't be join-point" << std::endl;
  std::cout << "=====================================================" << std::endl;
  std::cout << "should compile without error ..." << std::endl;
  std::cout << "=====================================================" << std::endl;
  return 0;
}
aspect Oops {
  advice execution("void t()") : around() { tjp->proceed(); }
  advice execution("void t()") : around() { tjp->proceed(); }
};
