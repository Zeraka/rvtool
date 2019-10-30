#include "Puma/CCParser.h"
#include "Puma/ErrorStream.h"
#include "Puma/UnitIterator.h"
#include "Puma/CTokens.h"
#include "Puma/PreParser.h"
#include <iostream>
#include <map>
#include <set>

typedef std::set<int> LineNumbers;
typedef std::map<const char*,LineNumbers> Macros;


int main (int argc, char **argv) {
  if (argc < 2) {
    std::cout << "Extract C preprocessor macro names from a file" << std::endl;
    std::cout << "usage: macronames <filename>" << std::endl;
    return 1;
  }

  // scan input file
  Puma::ErrorStream err;
  Puma::CProject project(err, argc, argv);
  Puma::Unit *file = project.scanFile(argv[argc-1]);
  if (! file) {
    std::cerr << argv[argc-1] << ": unable to scan input file" << std::endl;
    return 1;
  }

  // find macro names in the token lists of #if, #elseif, 
  // #ifdef, and #ifndef directives
  Macros macros;
  Puma::UnitIterator token(*file);
  for (; *token; ++token) {
    if (token->is_preprocessor()) { 
      int type = token->type();
      if (type == TOK_PRE_IF || type == TOK_PRE_ELIF) {
        // search for identifiers, ignore special identifier "defined"
        for (++token; *token; ++token) {
          if (token->is_whitespace()) {
            // white space, let's see if it is the end of the directive
            const char *c = token->text();
            for (; *c && *c != '\n'; c++);
            if (*c == '\n') {
              // end of directive, stop searching for macro names
              break;
            }
          } else if (token->is_identifier() && strcmp(token->text(), "defined") != 0) {
            // macro name found
            macros[token->text()].insert(token->location().line());
          }
        }
      } else if (type == TOK_PRE_IFDEF || type == TOK_PRE_IFNDEF) {
        // skip comments and white spaces
        for (++token; *token && (token->is_comment() || token->is_whitespace()); ++token);
        if (*token && token->is_identifier()) {
          // macro name found
          macros[token->text()].insert(token->location().line());
        }
      }
    }
  }
  
  // print the macro names and the line numbers where they are called (XML)
  std::cout << "<?xml version=\"1.0\"?>\n<macros>" << std::endl;
  Macros::const_iterator macro = macros.begin();
  for (; macro != macros.end(); ++macro) {
    std::cout << "  <macro name=\"" << macro->first << "\">" << std::endl;
    LineNumbers::const_iterator number = macro->second.begin();
    for (; number != macro->second.end(); ++number) {
      std::cout << "    <line>" << *number << "</line>" << std::endl;
    }
    std::cout << "  </macro>" << std::endl;
  }
  std::cout << "</macros>" << std::endl;
  
  return 0;
}
