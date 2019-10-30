#ifndef __MODULE_0_H__
#define __MODULE_0_H__

#include <iostream>

namespace Modules {

class Module0 {
private:
  static void init() {
    std::cout << "Module 0 initialized" << std::endl;
  }

};

} //namespace Modules

#endif
