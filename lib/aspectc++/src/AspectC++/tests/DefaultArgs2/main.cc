#include <iostream>

using namespace std;

static const int MaxBufferSize = 1024000;

const int getMBS() { return MaxBufferSize; }

class Connection
{
public:
    int readDataIntoBuffer(int maxSize = MaxBufferSize + getMBS() );
};

int Connection::readDataIntoBuffer(int maxSize)
{
    if (maxSize > MaxBufferSize)
        return 0;

    return 10;
}

int main() {
  int x = 42;
  Connection con;

  x = con.readDataIntoBuffer();
  cout << x << endl;

  x = con.readDataIntoBuffer( 1024 );
  cout << x << endl;

  return 0;
}

aspect TestExecution {
 advice call("const int getMBS()") : before() {
   cout << "  call " << JoinPoint::signature() << endl;
 }

 advice builtin("int operator +(int,int)") : before() {
   cout << "  builtin " << JoinPoint::signature() << endl;
 }

 advice get("const int MaxBufferSize") : before () {
   cout << "  get " << JoinPoint::signature() << endl;
 }
};
