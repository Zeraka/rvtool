#ifndef __ClangIntroParser_h__
#define __ClangIntroParser_h__

#include "clang/Parse/Parser.h"

class ClangIntroParser : public clang::Parser {

public:

  ClangIntroParser(clang::Preprocessor &PP, clang::Sema &Actions,
      bool SkipFunctionBodies) :
    clang::Parser(PP, Actions, SkipFunctionBodies) {}

  // New parser function: Clang parses the whole class definition in one
  // function. For the introduction mechanism we need to parse a class body
  // separately.
  void ParseCXXClassBody(clang::Decl *TagDecl);

  // Hack: We make this parser method 'public'
  void ParseBaseClause(clang::Decl *TagDecl);
};

#endif // __ClangIntroParser_h__
