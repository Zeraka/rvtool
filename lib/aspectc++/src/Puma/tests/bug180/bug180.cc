struct Y {
  int y;
};

template <typename T> 
class X {
public:
  typedef T evil;
  evil e;

  struct S : public evil {
    int n;
  };
};

int main() {
  X<Y> tc;
  return 0;
}
