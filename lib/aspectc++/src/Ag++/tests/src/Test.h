#ifndef __Test_h__
#define __Test_h__
class Test{

private:
	int number;
public:
	Test();
	int get();
	void set(int i);
  template <class T, class U> T GetMin (T a, U b);
};
#endif //__Test_h__

