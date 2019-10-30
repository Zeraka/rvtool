#include <iostream>

using namespace std;

static const int MaxBufferSize = 1024000;

const int getMBS() { return MaxBufferSize; }

class Connection
{
public:
    int readDataIntoBuffer(int maxSize = getMBS() );
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

  x = con.readDataIntoBuffer( 2048000 );
  cout << x << endl;

  return 0;
}

aspect TestExecution {
 advice call("const int getMBS()") : before() {
   cout << "  call " << JoinPoint::signature() << endl;
 }
};
