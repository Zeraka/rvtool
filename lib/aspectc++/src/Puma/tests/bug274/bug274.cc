struct A { 
  void f(int) {} 
}; 
     
struct B : public A { 
  using A::f; 
  void f(char*) {} 
}; 
	     
int main() { 
  B b; 
  b.f(4711); 
} 
