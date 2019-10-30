class A {
public:
  long offset () {}
};
  
class B : public virtual A {};
  
class C : public virtual A {
public:
  long offset ()  {}
};
    
class D : public virtual B, public C {};
    
int main() {
  D *result;
  result->offset();  // <= calls C::offset()
}
