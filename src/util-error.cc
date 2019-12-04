#include "util-error.hh"
#include <cstring>

#define CASE_CODE(E) \
    case E:          \
        return #E;
const char *AMErrorToString(AMError err)
{
    switch (err)
    {
        CASE_CODE(SUCCESS);
        CASE_CODE(ERROR);
        CASE_CODE(WORD_ACCEPTANCE_WRONG);
        CASE_CODE(NOMATCH);
        CASE_CODE(HOA_FORMAT_ERROR);
        CASE_CODE(HOA_PARSE_ABORT_ERROR);
        CASE_CODE(LTL_FILE_NOT_EXIT);
        CASE_CODE(LTL_FILE_FORMAT_WRONG);
        CASE_CODE(LTL_FILE_NOT_JSON);
        CASE_CODE(LTL_EXPRESSION_FORMAT_ERROR);
        CASE_CODE(YAML_FILE_IS_NULL);
        CASE_CODE(YAML_NODE_PARSE_ERROR);
        CASE_CODE(SOCKET_SEND_ERROR);

        CASE_CODE(PARSE_LABEL_TO_RPN_ERROR);
        CASE_CODE(PARSE_ACCEPTEORD_TO_WORDSET_ERROR);
        CASE_CODE(ACCEPT_WORD_FORMAT_WRONG);
        //CASE_CODE();
    }

    return "UNKNOWN_ERROR";
}
