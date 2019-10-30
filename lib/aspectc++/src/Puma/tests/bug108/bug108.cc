class X {
public:
  operator const char*();
};

void f() {
  X x;
  if (x != 0)      // <= X::const char*() conversion not recognized
    x[0] = '\0';   // <= equivalent to "*(x + 0)" => X::const char*()
}
