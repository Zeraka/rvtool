#ifndef __callee_h__
#define __callee_h__

class CalleeBase {
public:
  virtual void callee() {}
};

class Callee : public CalleeBase {
};

#endif // __callee_h__
