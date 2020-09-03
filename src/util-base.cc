#include "util-base.hh"
#include "util-error.hh"
#include "util-debug.hh"
#include <vector>
#include <string>
using namespace std;

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

void Check_json_log_format(char *event)
{
    int position = 0;
    std::string tmp = event;
    position = tmp.find_first_of("}");
    *(event + position + 1) = '\0';
}