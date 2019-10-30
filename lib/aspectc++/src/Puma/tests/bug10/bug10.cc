template <int i>
class X {};

template <template <int j> class T, int i>
void foo( T<i> ) {}

template <>
void foo( X<1> ) {}

void bar() {
  X<1> x;
  foo(x);
}
