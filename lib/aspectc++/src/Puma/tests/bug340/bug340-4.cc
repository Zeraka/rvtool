template <class T,class C> 
struct B { 
  void foo(T,C);
};

struct E : B<int,char> {
}; 

struct F : B<float,char> {
}; 

struct D : B<double,double>, E { 
};

struct A : B<double,double>, E, F { 
};

template <class T> 
void f(B<T,char>) {
}

int main()
{
  D d;
  f(d);  // calls f(B<int,char>)
  
  A a;   
  f(a);  // ambiguous: f(B<int,char>) or f(B<float,char>)
}
