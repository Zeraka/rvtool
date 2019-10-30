#include <iostream>

#define FAILING

struct test {
    int field0;
};

#ifndef FAILING
    test teststruct = {0, 1, 2, 3};
#else
    test teststruct = {0, 1, 3};
#endif

int main () {

  std::cout << "Bug 336: wrong ordering if order advice has no match:" << std::endl;
  std::cout << "=====================================================" << std::endl;
  std::cout << "the result should be 031 ..." << std::endl;
#ifndef FAILING
    std::cout << teststruct.field0 << teststruct.field1 << teststruct.field2 << 
teststruct.field3 << std::endl;
    // returns 0321
#else
    std::cout << teststruct.field0 << teststruct.field1 << teststruct.field3 << 
std::endl;
    // should return 031,
    // but returns 013 (as if order advice was not given)
#endif
  std::cout << "=====================================================" << std::endl;

    return 0;
}

aspect orderaspect {
    advice "test" : order ("slice3", "slice2", "slice1");
};

aspect slice1 {
    advice "test" : slice struct {
        int field1;
    };
};

#ifndef FAILING
aspect slice2 {
    advice "test" : slice struct {
        int field2;
    };
};
#endif

aspect slice3 {
    advice "test" : slice struct {
        int field3;
    };
};
