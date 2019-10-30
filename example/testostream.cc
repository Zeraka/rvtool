
#include<ostream>
#include<iostream>
#include<strstream>

using namespace std;

void f(ostream& out){
    out << "hello world" << endl;
}


int main(void){
    ostrstream out;

    f(out);

    cout << out.str() << endl; 
}

/*
将stream 转化为string类型
*/