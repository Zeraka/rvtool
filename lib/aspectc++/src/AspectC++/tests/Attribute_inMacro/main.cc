#include <iostream>

#define attrDef attribute test()
#define attrName1 [[myAttr::test]]
#define attrName2 myAttr::test

#define className MyClass


namespace myAttr {
attrDef;
}

struct className {
	attrName1 int myFunc() {return 0;}
	[[ attrName2 ]] int mySecondFunc() {return 0;}
};

int main()
{
	MyClass a;
	a.myFunc();
	a.mySecondFunc();
	return 0;
}

aspect output {
	advice execution( attrName2() ) : before() {
		using namespace std;
		cout << "Test successfull - first advice!" << endl;
	}

	advice execution( myAttr::test() ) : before() {
		using namespace std;
		cout << "Test successfull - second advice!" << endl;
	}
};
