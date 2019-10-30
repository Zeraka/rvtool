void bar( void (__stdcall *)() );     // <= wrong syntax tree
//void bar( void (__stdcall *arg)() );  // <= works, correct syntax tree

void __stdcall foo() {
}

int main() {
  bar( foo );
  return 0;
}
