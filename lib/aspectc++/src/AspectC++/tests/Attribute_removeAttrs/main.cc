#include <iostream>

using namespace std;

namespace myAttr {
  attribute attr();
}

//test if Attributes are removed
//this would cause a warning if it is not removed
class [[myAttr::attr]] myClass {};


int a = 0;

//if attribute is found by following compiler
//this function is called before entering
//main()-function.
void constructA [[gnu::constructor]] () {
	a = 10;
}

int main() {
	//if attribute not found: 0
	//if attribute found: 10
	cout << a << endl;

	return 0;
}

