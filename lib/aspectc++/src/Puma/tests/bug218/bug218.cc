class myClass {
public:
  ~myClass();
};

typedef myClass myClassTypedef;

void destroyMyClassObject(void* toDestroy) {
  ((myClassTypedef*)toDestroy)->~myClassTypedef();
}
