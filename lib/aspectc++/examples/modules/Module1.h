#ifndef __MODULE_1_H__
#define __MODULE_1_H__

#include <iostream>

namespace Modules {

class Module1 {
private:
  static void init() {
    std::cout << "Module 1 initialized" << std::endl;
  }

};

} //namespace Modules

#endif
