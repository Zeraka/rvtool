template<class T, int I>
class Test {
public:
  static T a;
};

template<int I>
class Test<int,I>{
public:
  static int b;
};

template<class T,int I>
T Test<T,I>::a;

template<int I>
int Test<int,I>::b;
