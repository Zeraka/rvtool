// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
//                                                                
// This program is free software;  you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as 
// published by the Free Software Foundation; either version 2 of 
// the License, or (at your option) any later version.            
//                                                                
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.                   
//                                                                
// You should have received a copy of the GNU General Public      
// License along with this program; if not, write to the Free     
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA  02111-1307  USA                                            

#ifndef __pre_parser_state__
#define __pre_parser_state__

// The state of a preprocessor parser. It contains all variable 
// parts of the preprocessor parser belonging not to the generated 
// lemon parser.

#include <set>

#include "Puma/Array.h"
#include "Puma/Token.h"
#include "Puma/PreMacroManager.h"

namespace Puma {


class ErrorStream;
class PreTree;
class PreMacro;
class TokenStream;

class PreParserState {
  // some constant strings
  DString _Line, _File, _Date, _Time, _IncLevel;
  DString _Defined, _Defined1;

  int _LastDay;

public:
  PreParserState ();
  ~PreParserState () { delete newline; }

  // Check whether a macro name is valid.
  bool checkMacroName (PreMacro *);

  // Check whether macro is one of the special `predefined' macros.
  PreMacro *checkPredefined (PreMacro *, TokenStream *, PreMacroManager &);

  // Check whether macro is a `self-referential' macro.
  bool isSelfReferentialMacro (PreMacro *, Array<PreMacro*> &);

  // Put a macro on the macro stack.
  void putMacroOnStack (Array<PreMacro*> &, PreMacro *, int stream_len);

  // Remove macro from macro stack.
  void removeMacroFromStack (Array<PreMacro*> &macroStack, const char* name);

  // Update the macro stack position.
  void updateMacroStack (Array<PreMacro*> &macroStack, int stream_len);

  // Calculate the number of files on the input stream.
  int getIncludeLevel (TokenStream* stream);

  // Check if current parsing code of a macro expansion.
  bool inMacroExpansion (Array<PreMacro*>& macroStack);

public:
  int maxIncludeDepth;  // Maximal include depth.

  // Are true if the special macros FILE, LINE ... has been redefined.
  bool noFILE, noLINE, noDATE, noTIME, noINCLUDE_LEVEL;

  bool syntaxError;     // True if a syntax error occured.
  bool errorDirective;  // True if a #error directive has been parsed.
  bool passOnToken;     // If false, following C++ code will be ignored.
  bool forcedIncludes;  // True as long as we are parsing forced include files.
  bool parseMacroArgs;  // True if parsing macro arguments.

  Token *sl_token;      // Single line comment start token.
  Token *currToken;     // The current token to parse.
  Token *lastToken;     // Last parsed token.
  int currType;         // The type of the current token to parse.
  int lastType;         // The type of the last parsed token.

  void *pParser;        // Pointer to the Lemon generated parser.
  ErrorStream *err;     // Error stream.
  PreTree *syntaxTree;  // Preprocessor syntax tree.

  Token *newline;       // Token to trick the parser.
};


} // namespace Puma

#endif /* __pre_parser_state__ */
