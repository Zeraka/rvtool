struct X {
  static const int x = 0;
  struct Y;
};

struct X::Y {
  Y() {x;}       // <= x not found
};
