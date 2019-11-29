#include "ltl-parse.hh"
#include "util-error.hh"
#include <string>
#include <iostream>
#include <fstream>
#include <regex>

#include "CJsonObject.hpp"

static AMError parse_ltl_json_file(const std::string &filename);

static AMError parse_ltl_json_file(const std::string &filename)
{
    if (filename.empty())
    {
        ErrorPrintNReturn(LTL_FILE_NOT_EXIT);
    }

    //读取文件, 读取直到第一次碰到LTL公式。
    std::ifstream file;
    file.open(filename, std::ios::in);

    std::string line;
    std::string re = "/[^,:{}\\[\\]0-9.\-+Eaeflnr-u \n\r\t]/"; //Regex represents json format.

    if (file.is_open() == 0)
    {
        ErrorPrintNReturn(LTL_FILE_NOT_EXIT);
    }

    while (!file.eof())
    {
        getline(file, line);

        if (line.compare("\n"))
        {
            continue;
        }
        else if (std::regex_match(line, re) == 1) //use the regex to match LTL expression
        {

        }
        else
        {
            ErrorPrintNReturn(LTL_FILE_NOT_JSON);
        }
    }

    neb::CJsonObject ojson(filename);

    //解析后生成一个自动机格式
}

/*Parse ltl file*/
AMError parse_ltl_file(const std::string &filename, const std::string &fileFormat)
{
    if (filename.empty())
    {
        ErrorPrintNReturn(LTL_FILE_NOT_EXIT);
    }
    if (fileFormat.compare("json"))
    {
        parse_ltl_json_file(filename);
    }
    else
    {
        ErrorPrintNReturn(LTL_FILE_FORMAT_WRONG);
    }
}

AMError parse_ltl_by_ltl2tgba()
{
}

AMError 