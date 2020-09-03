#include "util-parse.hh"
#include "util-debug.hh"
#include "util-base.hh"
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <stack>

using namespace std;

static std::vector<std::string> Operate_Logic_AND(std::vector<std::string> &strs1, std::vector<std::string> &strs2)
{
    std::vector<std::string> newstrs;

    for (int m = 0; m < strs1.size(); m++)
    {
        for (int n = 0; n < strs2.size(); n++)
        {
            std::string tmp = strs1[m] + " & " + strs2[n];
            newstrs.push_back(tmp);
        }
    }
    return newstrs;
}

static std::vector<std::string> Operate_Logic_OR(std::vector<std::string> &strs1, std::vector<std::string> &strs2)
{
    std::vector<std::string> newstrs;

    for (int n = 0; n < strs2.size(); n++)
    {
        strs1.push_back(strs2[n]);
    }
    newstrs = strs1;

    return newstrs;
}

/*
    RPN means Reverse Polish notation.
*/
std::vector<std::string> Parse_label_exp_to_RPN(std::string &label)
{
    std::stack<std::string> a_stack;
    std::vector<std::string> rpn;
    int startPosition = 0;
    int endPosition = 0;

    /*The label example: "!green & (press | red)"*/
    char *labelstr = (char *)label.c_str();
    VePrint(label.c_str());
    //VePrint(label.length());
    while (endPosition != label.length())
    {
        char ch = *(labelstr + endPosition);
        //VePrint(ch);
        if (ch == '(' || ch == '&' || ch == '|')
        {
            //INFOPrint("Enter 1");
            std::string tmp = "";
            tmp.append(1, ch);
            a_stack.push(tmp);
            endPosition++;
            startPosition = endPosition;
        }
        else if (ch == ' ')
        {
            //INFOPrint("Enter 2");
            //VePrint(startPosition)
            //VePrint(endPosition);

            if (startPosition != endPosition)
            {
                string tmp = label.substr(startPosition, endPosition - startPosition);
                //VePrint(tmp);
                rpn.push_back(tmp);
            }
            endPosition++;
            startPosition = endPosition;
        }
        else if (ch == ')')
        {
            //INFOPrint("Enter 3");
            if (startPosition != endPosition)
            {
                std::string tmp = label.substr(startPosition, endPosition - startPosition);
                //VePrint(tmp);
                rpn.push_back(tmp); //Test the value of RPN?
            }
            endPosition++;
            startPosition = endPosition;

            while (a_stack.top() != "(")
            {
                std::string tmp = a_stack.top();
                a_stack.pop();//
                rpn.push_back(tmp);
            }
            a_stack.pop();
        }
        else
        {
            //INFOPrint("Enter 4");

            endPosition++;
        }
    }

    if (endPosition == label.length())
    {
        if (startPosition != endPosition)
        {
            std::string tmp = label.substr(startPosition, endPosition - startPosition);
            VePrint(tmp);
            rpn.push_back(tmp); //Test the value of RPN?
        }
    }
    else
    {
        INFOPrint("PARSE_LABEL_TO_RPN_ERROR");
    }

    while (a_stack.empty() == false)
    {
        std::string tmp = a_stack.top();
        a_stack.pop();
        rpn.push_back(tmp);
    }

    if (rpn.front() == "|" || rpn.front() == "&")
    {
        INFOPrint("PARSE_LABEL_TO_RPN_ERROR");
    }
    return rpn;
}


/*Parse the label string */
//Test exmple "(a | b) | (c | d)"
std::vector<std::string> Parse_label_RPN_to_string_sets(std::vector<string> &rpn)
{
    typedef std::vector<std::string> Strvector;
    typedef std::vector<Strvector> Strvectorbucket;

    int m = -1;

    Strvectorbucket strvbu;
    for (int i = 0; i < rpn.size(); i++)
    {
        //VePrintByArg(rpn, i);
        if (rpn[i] != "|" && rpn[i] != "&")
        {
            //ENTERPrint(1);
            Strvector strv1;
            strv1.push_back(rpn[i]);
            strvbu.push_back(strv1);
            m++;
            //VePrint(m);
        }
        else if (rpn[i] == "|")
        {
            //ENTERPrint(2);

            Strvector strv2 = Operate_Logic_OR(strvbu[m - 1], strvbu[m]);
            //VePrintVector(strv2);
            m--;
            //VePrint(m);
            strvbu.pop_back();
            strvbu.pop_back();
            strvbu.push_back(strv2);
        }
        else if (rpn[i] == "&")
        {
            //ENTERPrint(3);
            //立即进行新字符串的合成。产生 a & b 类型的字符串。
            Strvector strv3 = Operate_Logic_AND(strvbu[m - 1], strvbu[m]);
            //VePrintVector(strv3);
            m--;
            //VePrint(m);
            strvbu.pop_back();
            strvbu.pop_back();
            strvbu.push_back(strv3);
        }
    }
    //Print the a_stack
    VePrintVector(strvbu[0]);
    if (m != 0)
    {
        ERRORPrint("Parse_label_RPN_to_string_sets Wrong");
    }
    FuncEnd();
    return strvbu[0];
}