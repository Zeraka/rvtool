#pragma once

typedef enum
{
    SUCCESS = 0,
    ERROR,
    WORD_ACCEPTANCE_WRONG,
    NOMATCH,
    LTL_FILE_NOT_EXIT,
    LTL_FILE_FORMAT_WRONG,
} AMError;

const char *AMErrorToString(AMError err);

#define ErrorPrintNReturn(AMError)                         \
    {                                                      \
        std::cout << AMErrorToString(AMError) << "in line" \
                  << __LINE__ << "in file" << __FILE__     \
                  << std::endl;                            \
        return AMError;                                    \
    }