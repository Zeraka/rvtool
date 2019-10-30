struct A {};
struct B : A {};

void x (A *);
void x (const A *);

void f ()
{
  x ((B*)0);
}
