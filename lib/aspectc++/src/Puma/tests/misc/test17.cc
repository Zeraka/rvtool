namespace N {
  template <typename T> int foo1(int);
  template <typename T> int foo2(T);
  template <int I> int foo3();
}
template <typename T> int foo4(int);

int res1 = N::foo1<int>(1);
int res2 = N::foo2<int>(1);
int res3 = N::foo3<1>();
int res4 = N::template foo3<1>();
int res5 = foo4<int>(1);
