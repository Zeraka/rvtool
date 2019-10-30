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

#ifndef __preprocessor_parser__
#define __preprocessor_parser__

// Preprocessor parser class. !!! The preprocessor parser has to 
// be added last to the composite parser of a translation unit !!!

/** \page preprocessing Preprocessing
 */

#include "Puma/Stack.h"
#include "Puma/Config.h"
#include "Puma/CScanner.h"
#include "Puma/CUnit.h"
#include "Puma/TokenSource.h"
#include "Puma/PreMacroExpander.h"
#include <iostream>
#include <set>
#include <list>
#include <string>

namespace Puma {


class PreTree;
class ErrorStream;
class UnitManager;
class PrePredicate;
class PreLevelState;
class PreParserState;
class PreMacroManager;
class PreFileIncluder;
class PrePredicateManager;

class PreprocessorParser : public TokenSource {
public:
  // The current mode for token preprocessing
  typedef enum { 
    INITIAL, 
    DEFINING, 
    MAYBEMACRO, 
    ASSERT,
    TOKENLIST, 
    DIRECTIVE, 
    MACRO 
  } PreMode;

private:
  // Do not print the result of the parse process.
  bool _silentMode;

  // If the preprocessor parser is the last in chain,
  // the result of the parse process will be putted on 
  // the given stream.
  std::ostream *_out;

  // Stack for the macro expansion.
  Array<PreMacro*>    *_macroStack;

  PreLevelState       *_levelState;
  PreParserState      *_parserState;
  PreMacroManager     *_macroManager;
  PreFileIncluder     *_fileIncluder;
  PrePredicateManager *_predicateManager;
  UnitManager         *_unitManager;
  UnitManager         *_locals;

  PreMacroExpander     _expander;
  PreMacroExpander     _macroExpander;

  PreMode              _pre_mode;
  bool                 _support_gnu;

  CScanner             _scanner;
  TokenStream         *_stream;

  const char          *_importHandler;

private:
  // Parse a single token.
  Token* parseToken ();

  // Free the preprocessor syntax tree.
  void freeSyntaxTree (PreTree*);

  // Define GNU macros
  void defineGNUMacros ();

  // Get and parse the next token.
  void parseNextToken (Token*& sl_comment, Unit*& comment_unit, bool& last_in_expansion);

  // Check if the current token is a macro call
  bool isMacroCall (bool specialDefinedMacro);

  // Collect the tokens of the macro call for expansion.
  bool collectMacroCallTokens (PreMacro* macro);

  // Expand a macro call.
  bool expandMacroCall (PreMacroExpander& expander);

  // Parse a generated single line comment.
  bool parseGeneratedSingleLineCommentToken (Token*& sl_comment, Unit*& comment_unit, bool& last_in_expansion);

  // Finish the parse process.
  void finishParsing ();

  // Check if macro name has format "name(args)".
  bool hasArguments(const char *name) const;

  // Extract the name of macro given as "name(args)".
  std::string getMacroName(const char *nameAndArgs) const;

  // Extract the arguments of macro given as "name(args)".
  void getMacroArgs(const char *nameAndArgs, std::list<std::string>& args) const;

public:
  // Get several preprocessor informations.
  Array<PreMacro*>    *macroStack () const       { return _macroStack; }
  PreLevelState       *levelState () const       { return _levelState; }
  PreParserState      *parserState () const      { return _parserState; }
  PreMacroManager     *macroManager () const     { return _macroManager; }
  PreFileIncluder     *fileIncluder () const     { return _fileIncluder; }
  PrePredicateManager *predicateManager () const { return _predicateManager; }
  UnitManager         *unitManager () const      { return _unitManager; }
  UnitManager         *locals () const           { return _locals; }
  PreMode              pre_mode () const         { return _pre_mode; } 
  bool                 supportGNU () const       { return _support_gnu; }
  CScanner            &cscanner () const         { return (CScanner&) _scanner; }
  const char          *importHandler () const    { return _importHandler; }
  void macroManager (PreMacroManager *m)         { _macroManager = m; }
  void fileIncluder (PreFileIncluder *f)         { _fileIncluder = f; }
  void predicateManager (PrePredicateManager *p) { _predicateManager = p; }

  // Return the preprocessor syntax tree.
  PreTree *syntaxTree () const;

public:
  PreprocessorParser (ErrorStream *, UnitManager *, UnitManager *, 
                        std::ostream &out = std::cout, int max_depth = 400);

  ~PreprocessorParser ();

  // Reset the preprocessor parser.
  void reset (ErrorStream *, UnitManager *, UnitManager *, 
              std::ostream &out = std::cout, int max_depth = 400);

  // Free the preprocessor syntax tree.
  void freeSyntaxTree ();

  // Configure the preprocessor.
  void configure (const Config &, bool = true);

  // Get the next token to parse.
  Token *next ();

  TokenStream *scanner () const { return _stream; }
  void stream (TokenStream *s) { _stream = s; }

  // Map scanner token types to parser token types
  int map_token (Token *token, bool &parse);

  // Invoke the parse process.
  void parse ();

  // Switch to silent mode.
  void silentMode (bool mode = true) { _silentMode = mode; }

  // Define a new macro.
  void defMacro (const char *, const char * = (const char*)0, char = 0) const;

  // Define a macro if it is not yet defined.
  void defUndefinedMacro (const char *, const char * = (const char*)0, char = 0) const;

  // Undefine a macro.
  void undefMacro (const char *) const;

  // Define a new predicate.
  void defPredicate (const char *, const char *) const;

  // Undefine a predicate.
  void undefPredicate (const char *) const;

  // Add a new non-system include path.
  void addInclPath (const char *) const;

  // Add a new system include path.
  void addSysInclPath (const char *) const;

  void supportGNU (bool);
};


} // namespace Puma

#endif /* __preprocessor_parser__ */
