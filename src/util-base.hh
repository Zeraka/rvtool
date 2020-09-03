#ifndef UTIL_BASE_HH
#define UTIL_BASE_HH

#include "util-error.hh"
#include "util-debug.hh"
#include <vector>
#include <string>
using namespace std;

typedef unsigned int uint4;
typedef std::vector<std::string> stringList;

/*
功能： 字符串分割函数
*/
stringList splitstr(const std::string &str, char tag);

/*
    Split the string by many chars.
    tag[] can be {' ', '(',')','&','|'};
*/


/*
Format the received log.
*/
void Check_json_log_format(char *event);

/*Free the memory*/
#define AMFree(x)                            \
    {                                        \
        if (x)                               \
        {                                    \
            free(x);                         \
            x = nullptr;                     \
            INFOPrint(#x " has been freed"); \
        }                                    \
    }

/*Free the memory*/
#define AMDelete(x)                          \
    {                                        \
        if (x)                               \
        {                                    \
            delete x;                        \
            x = nullptr;                     \
            INFOPrint(#x " has been freed"); \
        }                                    \
    }

//===========================================
//Test Unit==================================
//===========================================

//void TestsplitstrBycharArray(void);


#endif

