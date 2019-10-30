/************************************************
 ** multi text tokens and empty expressions in **
 ** preprocessor conditionals                  **
 ************************************************/

#define OR or

int
#if
a
#elif
b
#else
c
#endif
= 1 OR 1;   // shall be expanded to "or" not "||"
