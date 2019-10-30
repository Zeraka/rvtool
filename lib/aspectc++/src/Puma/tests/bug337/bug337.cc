template<int N>
void f(const char (&str)[N], int) {}

template<class T>
void f(const char *str, T) {}

int main () {
  const char s[6] = "Hallo";
  f(s,1);
}
