#include "Test.h"
#include "../additional_includes/test_include.h"
Test::Test(){
	number=0;
}

int Test::get(){
	return number;
}

void Test::set(int i){
	number=i;
}

template <class T, class U>
T GetMin (T a, U b) {
  return (a<b?a:b);
}
