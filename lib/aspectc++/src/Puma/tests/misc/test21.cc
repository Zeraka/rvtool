// Test built-in operators generated 
// during overload resolution.

struct String {
  operator char*& () { 
    static char *p; 
    return p; 
  }
};

void f () {
  String s;
  s[0];      // built-in: char*& operator[] (char*, ptrdiff_t)
  *(s + 0);  // built-in: char*  operator+  (char*, ptrdiff_t)
}
