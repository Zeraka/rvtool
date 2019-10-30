class MyClass {
private:
  void privateWork() {}
public:
  MyClass() {}
  void work() { for (int i=0; i<2; i++) privateWork(); }
  void work (int x) { for (int i=0; i<x; i++) work(); }

};


void work() {}
void nowork(int n, char** args) {}

int main() {
  MyClass obj;
  
  obj.work(3);

  work();
  work();

  return 0;
}
