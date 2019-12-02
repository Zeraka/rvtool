#ifndef UTIL_BASE_HH
#define UTIL_BASE_HH

#include "util-error.hh"
#include "util-debug.hh"

typedef unsigned int uint4;
typedef std::vector<std::string> stringList;

/*
功能： 字符串分割函数
*/
stringList splitstr(const std::string &str, char tag)
{
    stringList li;
    std::string subStr;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (tag == str[i])
        {
            if (!subStr.empty())
            {
                li.push_back(subStr);
                subStr.clear();
            }
        }
        else
        {
            subStr.push_back(str[i]);
        }
    }

    if (!subStr.empty())
    {
        li.push_back(subStr);
    }

    return li;
}
/*
Intercepts the string and prints from scratch to the first occureence
of the character n (including n).
*/
static int interceptCString(char *str, char n)
{
    char *p = str;

    while (*p != '\0')
    {
        if (*p == n)
        {
            break;
        }
        else
        {
            p = p + 1;
        }
    }
}


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
#endif