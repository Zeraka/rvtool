#include <iostream>
using namespace std;

template<typename T>
struct is_char {
  static void print() { cout << "wrong type" << endl; }
};
template<>
struct is_char<char> {
  static void print() { cout << "char type" << endl; }
};

const char arr[2] = {'A', 'B'};

int main() {
  char x = arr[1];
  return 0;
}

aspect Bar {
  advice get("% ...::%") : before() {
    cout << "Get: " << JoinPoint::signature() << endl;
    cout << "Entity type: ";
    is_char<JoinPoint::Entity>::print(); // should be: char type
    cout << "Result type: ";
    is_char<JoinPoint::Result>::print(); // should be: char type
    cout << "Res::Type: ";
    is_char<typename JoinPoint::Res::Type>::print();
    cout << "Res::ReferredType: ";
    is_char<typename JoinPoint::Res::ReferredType>::print();
  }
};
