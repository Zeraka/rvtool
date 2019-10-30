/****************************************
 ** overload resolution with arguments **
 ** of type "template instance"        **
 ****************************************/

template <class T>
struct basic_string {
  basic_string(const char*){}
};

typedef basic_string<char> string;

void foo(const string &s){}

int main() {
  foo("Hallo");       // <= has to be resolved
  return 0;
}
