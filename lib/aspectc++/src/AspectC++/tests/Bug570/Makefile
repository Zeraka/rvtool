WEAVE_ERROR := 1
# make sure the error message, which depends on the frontend, is hidden as well as the column
AC_PATTERN_REPLACEMENTS := | grep "error: " | sed -e 's/cc:\([0-9][0-9]*\):[0-9][0-9]*:/cc:\1:/' | sed -e "s/error: .*/error: <<<replaced>>>/"
include ../Makefile.generic
