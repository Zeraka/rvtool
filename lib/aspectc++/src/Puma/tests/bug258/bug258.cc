template <class T>
void kill (T *ptr) { 
  ptr->~T();  // should work for class type as well as for POD types
} 

class X {};

void foo() {
  X* x1;
  kill(x1);   // OK
  int* x2;
  kill(x2);   // not OK for Puma
}
