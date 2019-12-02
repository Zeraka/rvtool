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
Format the received log.
*/
void Check_json_log_format(char *event){
        int position = 0;
        std::string tmp = event;
        position = tmp.find_first_of("}");
        *(event + position + 1) = '\0';
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