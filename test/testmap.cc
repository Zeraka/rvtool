#include<map>
#include<iostream>
#include<string>
using namespace std;
/*
统计每个单词在输入中的次数。

*/
int main(void){
    map<string, size_t> word_count;//word_count是一个map类型的。
    string word;
    int i=0;
    while(cin >> word){
        ++word_count[word];
        i++;

        if(i > 6) break;
    } 

    for(const auto& w:word_count)
    {
        cout<<w.first<<" occurs "<<w.second<<((w.second > 1)? " times ":" time ")<<endl;
    }
}