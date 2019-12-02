#pragma once

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
    SOCKET_SEND_ERROR
} AMError;

const char *AMErrorToString(AMError err);

#define ErrorPrintNReturn(AMError)                           \
    {                                                        \
        std::cout << AMErrorToString(AMError) << "in line: " \
                  << __LINE__ << " in file:" << __FILE__     \
                  << std::endl;                              \
        return AMError;                                      \
    }
