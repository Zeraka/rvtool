class Int {
public:
  operator int () { return 3; }
} obj;

double array[10];

void f () {
  array[obj] = 3.14;
}
