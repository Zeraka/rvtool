#include "util-error.hh"
#include <cstring>

#define CASE_CODE(E) \
    case E:          \
        return #E
const char *AMErrorToString(AMError err)
{
    switch (err)
    {
        CASE_CODE(SUCCESS);
        CASE_CODE(ERROR);
        CASE_CODE(WORD_ACCEPTANCE_WRONG);
        CASE_CODE(NOMATCH);
        CASE_CODE(LTL_FILE_NOT_EXIT);
        CASE_CODE(LTL_FILE_FORMAT_WRONG);
        CASE_CODE(LTL_FILE_NOT_JSON);
        CASE_CODE(YAML_FILE_IS_NULL);
        CASE_CODE(SOCKET_SEND_ERROR);
        //CASE_CODE();
    }

    return "UNKNOWN_ERROR";
}

