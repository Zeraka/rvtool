#include <iostream>
#include <cstdio>
#include <string>
#include <chrono>
#include <functional>

#include "EventLoop.hpp"
#include "aspectdemo.ah"
#include <unistd.h>
using namespace std;

class test
{
public:
    test() : rr(0)
    {
        cout << this << endl;
        cout << "construct" << endl;
    }

    test(const test &t) : rr(t.rr)
    {
        cout << "copy construct" << endl;
    }

    test &operator=(const test &t)
    {
        rr = t.rr;
        cout << "copy assignment" << endl;
        return (*this);
    }

    test(test &&t) : rr(t.rr)
    {
        cout << "move construct" << endl;
    }

    test &operator=(test &&t)
    {
        rr = t.rr;
        cout << "move assignment" << endl;
        return (*this);
    }

    int rr;
};

void Event0Func()
{
    sleep(1);

    fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
}

void Event1Func(double value)
{
    sleep(1);

    fprintf(stdout, "L%d::%s - %lf\n", __LINE__, __FUNCTION__, value);
}

void Event2Func(string str, int value)
{
    sleep(1);

    fprintf(stdout, "L%d::%s - %s %d\n", __LINE__, __FUNCTION__, str.c_str(), value);
}

void Event3Func(EventLoop *el)
{
    //fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
    sleep(1);
    el->Post([=] { Event3Func(el); }); //aspect只用于插装日志
}

void Event4Func(EventLoop *el)
{
    //fprintf(stdout, "L%d::%s\n", __LINE__, __FUNCTION__);
    sleep(1);
    el->Post([=] { Event4Func(el); });
}

void Event5Func(test t)
{
    sleep(1);

    fprintf(stdout, "L%d::%s - %d\n", __LINE__, __FUNCTION__, t.rr);
}

test get_test()
{
    return test();
}

void testRef(test t1, const test &t2, test &&t3)
{
    cout << "add t1: " << &t1 << endl;
    cout << "add t2: " << &t2 << endl;
    cout << "add t3: " << &t3 << endl;
}

int main(int argc, char **argv)
{
    cout << "BEGIN" << endl;

    testRef(test(), test(), test());
    test t0;
    test t1(t0);
    test t2 = t0;
    test t3 = test();
    test t4 = get_test();
    test t5 = move(get_test());
    t1 = t0;
    t1 = test();
    t1 = get_test();

    EventLoop gEventLoop(5);

    function<void()> gEvent0Func = Event0Func;
    function<void(double)> gEvent1Func = bind(Event1Func, placeholders::_1);
    function<void(string, int)> gEvent2Func = bind(Event2Func, placeholders::_1, placeholders::_2);
    function<void(string)> gEvent22Func = bind(Event2Func, placeholders::_1, 54321);
    function<void(test)> gEvent5Func = bind(Event5Func, placeholders::_1);

    gEventLoop.Start();

    gEventLoop.Post(bind(gEvent2Func, string("Hello World!"), 1234));
    gEventLoop.Post(bind(gEvent22Func, string("Hello World Again!")));

    gEventLoop.Post(gEvent0Func);
    gEventLoop.Post(Event0Func);

    gEventLoop.Post(bind(Event1Func, 42.31));
    gEventLoop.Post([=] { gEvent1Func(452.361); });
    gEventLoop.Post([=] { Event1Func(452.361e10); });

    gEventLoop.Post(bind(Event3Func, &gEventLoop));
    gEventLoop.Post(bind(Event4Func, &gEventLoop));
    gEventLoop.Post([&] { Event4Func(&gEventLoop); });

    //gEventLoop.Post(bind(Event5Func, test()));
    //gEventLoop.Post(bind(gEvent5Func, test()));
    gEventLoop.Post([=] { gEvent5Func(test()); });

    while (true)
        ;
    //_sleep(50);
    gEventLoop.Stop(true);

    cout << "DONE." << endl;

    return 0;
}
