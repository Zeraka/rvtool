#ifndef __MODULES_H__
#define __MODULES_H__

namespace Modules {

void init() {
  /*
    This function is empty and serves as an explicit joinpoint.
    Aspects that belong to modules can bind to this function
    in order to get initialized.
    This way, modules can be omitted without changing or modifying
    other parts of the system. The modules are loosely coupled.
  */
}

} //namespace Modules

#endif
