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

#ifndef __pre_macro_expander__
#define __pre_macro_expander__

// The macro expander provides methods to expand a single macro or
// all macros of a given string or unit.

#include <set>

#include "Puma/Location.h"
#include "Puma/Array.h"
#include "Puma/PreArgDesc.h"

namespace Puma {


class PreprocessorParser;
class PreMacroExpander;
class PreMacroManager;
class UnitManager;
class TokenStream;
class ErrorStream;
class PreMacro;

class PreMacroExpander {
  TokenStream        *_scanner;
  PreMacroManager    *_macroManager;
  UnitManager        *_unitManager;
  ErrorStream        *_err;
  Array<PreMacro*>   *_macroStack;

  PreprocessorParser *_parser;

  PreMacroExpander   *_preexpander;

  bool               _in_include; // true if an include directive is scanned

  // Check whether there is really a macro.
  bool checkForGoOn (bool mode = true) const;

  // Process the macro operator `##'.
  void processConcatOp (Unit *) const;

  // Process the macro operator `#'.
  void processStringOp (PreMacro *, Unit *, PreArgDesc &) const;

  // Stringify and replace an argument.
  void replaceStringifiedArg (int, Unit *, PreArgDesc &) const;

  // Replace an argument unchanged.
  void replaceUnchangedArg (int, Unit *, PreArgDesc &, Unit *, Token *, bool) const;

  // Test whether a string is an argument of a macro.
  int isArg (const char *, PreMacro *) const;

  // Compare two macro argument identifier.
  bool equal (const char *, const char *) const;

  // Build an unit from a string.
  Unit *buildUnit (const char *) const;

  // Collect the arguments of a function-like macro.
  void collectArgs (PreMacro *, PreArgDesc &) const;

  // Expand a function-like macro.
  void substituteMacro (PreMacro *, PreArgDesc &) const;

  // Process the special keyword `defined'.
  void processDefined (PreMacro *, PreArgDesc &) const;

  // Prescan an argument of a macro call.
  Unit& prescanArgument (Unit *) const;

  // Expand the given macro.
  bool expandMacro (PreMacro *, Token *) const;

  // Reset the expander to a special initial state.
  void reset (PreprocessorParser *, Array<PreMacro*> *);

  // Expand all macros of the given unit.
  Unit *expandMacros (Unit *);

  // Push unit on token stream.
  void pushUnit (TokenStream &, Unit *) const;

  // Get the next non-whitespace non-comment token
  Token* nextPreprocessingToken (Token*, Unit*) const;

  // Get the previous non-whitespace non-comment token
  Token* prevPreprocessingToken (Token*, Unit*) const;

  // Check if the given unit contains only spaces and comments.
  bool onlySpacesAndComments (MacroUnit&) const;

public:
  // Special constructor. Call reset() before using the expander.
  PreMacroExpander ();

  PreMacroExpander (PreprocessorParser *);
  ~PreMacroExpander () { if (_preexpander) delete _preexpander; }

  // Reset the expander to the initial state.
  void reset (PreprocessorParser *);

  // Set a flag that is used to check whether a macro is expanded in a
  // #include directive
  void inInclude (bool ii) { _in_include = ii; }

  // Expand the given macro.
  bool expandMacro (PreMacro *);

  // Expand all macros of the given string.
  char *expandMacros (const char *);
};


} // namespace Puma

#endif /* __pre_macro_expander__ */
