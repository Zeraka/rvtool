#pragma once
#include "util-debug.hh"
typedef enum
{
    SUCCESS = 0,
    ERROR,
    WORD_ACCEPTANCE_WRONG,
    NOMATCH,
    HOA_FORMAT_ERROR,
    HOA_PARSE_ABORT_ERROR,
    LTL_FILE_NOT_EXIT,
    LTL_FILE_FORMAT_WRONG,
    LTL_FILE_NOT_JSON,
    LTL_EXPRESSION_FORMAT_ERROR,
    YAML_FILE_IS_NULL,
    YAML_NODE_PARSE_ERROR,
    SOCKET_SEND_ERROR,
    PARSE_LABEL_TO_RPN_ERROR,
    PARSE_ACCEPTEORD_TO_WORDSET_ERROR,
    ACCEPT_WORD_FORMAT_WRONG
} AMError;

const char *AMErrorToString(AMError err);

#define ErrorPrintNReturn(AMError)                                                             \
    {                                                                                          \
        std::cout << BOLDRED << AMErrorToString(AMError) << BOLDYELLOW << " in line: "         \
                  << BOLDGREEN << __LINE__ << BOLDCYAN << " in file: " << BOLDBLUE << __FILE__ \
                  << RESET << std::endl;                                                       \
        return AMError;                                                                        \
    }
#define ErrorPrintNEXIT_0(AMError)                                                             \
    {                                                                                          \
        std::cout << BOLDRED << AMErrorToString(AMError) << BOLDYELLOW << " in line: "         \
                  << BOLDGREEN << __LINE__ << BOLDCYAN << " in file: " << BOLDBLUE << __FILE__ \
                  << RESET << std::endl;                                                       \
        exit(0);                                                                        \
    }

#define ErrorPrintNEXIT_1(AMError)                                                             \
    {                                                                                          \
        std::cout << BOLDRED << AMErrorToString(AMError) << BOLDYELLOW << " in line: "         \
                  << BOLDGREEN << __LINE__ << BOLDCYAN << " in file: " << BOLDBLUE << __FILE__ \
                  << RESET << std::endl;                                                       \
        exit(1);                                                                        \
    }


    