#include <stdio.h>

class Server
{
public:
 void serve() {}
};

class Client_A
{
public:
 void call()
  {
    Server s;
    s.serve();
 }
};

class Client_B : public Client_A {
public:
  void foo() {
    printf ("in Client_B::foo()\n");
  }
};

aspect Log {
  pointcut log(Client_B &c) = call("% Server::serve(...)") && that(c);
  
  advice log(c) : after(Client_B &c) {
    c.foo();
  }

  advice call("% Server::serve(...)") && that("Client_B") : after() {
    Client_B &c = *(Client_B*)tjp->that ();
    c.foo();
  }

  advice execution("% Client_A::call(...)") && that("Client_B") : around() {
    printf ("before serve()\n");
    tjp->proceed ();
    printf ("after serve()\n");
  }

};

int main() {
  printf ("ThatContext: checks 'that' with context variables\n");
  printf ("=================================================\n");
  printf ("a.call() should yield nothing:\n");
  Client_A a;
  a.call();
  printf ("-------------------------------------------------\n");
  printf ("c.call() should yield two invocations of foo() within serve():\n");
  Client_B c;
  c.call();
  printf ("=================================================\n");
}
