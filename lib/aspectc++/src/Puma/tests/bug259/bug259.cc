void f_v(volatile int *) {}

void f(int *) {}
void f_c(const int *) {}
void f_cv(const volatile int *) {}
void f_vc(volatile const int *) {}

int main(int, char **) {
  int i;
  
  // examples/ccparser complains: no matching function for call to `f_v'
  f_v(&i);
      
  // no problem
  f(&i);
  f_c(&i);
  f_cv(&i);
  f_vc(&i);
	
  return 0;
}
		  