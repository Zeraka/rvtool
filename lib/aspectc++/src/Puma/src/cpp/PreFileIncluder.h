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

#ifndef __pre_file_includer__
#define __pre_file_includer__

// Preprocessor class to handle includes.

#include "Puma/PreprocessorParser.h"
#include "Puma/PreMacroExpander.h"
#include "Puma/Filename.h"
#include "Puma/Array.h"

#include <map>
#include <list>
#include <string>

namespace Puma {


class PreParserState;
class TokenStream;
class UnitManager;
class Unit;

class PreFileIncluder {
  int                 _skip;
  PreParserState     *_state;
  ErrorStream        *_err;
  UnitManager        *_unitManager;
  PreprocessorParser *_parser;

  PreMacroExpander _expander;

  // Ordered list of include paths.
  std::list<std::string> _includeDirs;
  std::list<std::string>::iterator _firstIncludeDir;

  // Include paths lookup and system/non-system include path status map.
  std::map<std::string,bool> _includeDirLookup;

  // true if the last included file was protected by an active include guard
  bool _guarded;

  // the set of included units, the Token is 0 or the name of an include guard
  std::map<Unit*, Token*> _includedUnits;

  // Builds a macro expanded unit from a string.
  Unit *buildUnit (const char *);

  // Prepare to include a file from current working directory.
  Unit *includeFromCwd (Unit *);

  // Prepare to include a file from system include directories.
  Unit *includeFromSystem (Unit *);

  // Try to locate the include file.
  Unit *searchFile (const char *, bool);

  // Include the file.
  Unit *doInclude (const char *, const char *, bool);

  // check if this file has an include guard that is already active
  bool hasActiveIncludeGuard (Unit *unit);

  // check if this file has an include guard
  static Token *hasIncludeGuard (Unit *unit);

  // Return true if string is not of format "..." or <...>.
  bool assumeMacroCalls (const char *) const;

public:
  PreFileIncluder (PreprocessorParser &);

  // Push unit on token stack.
  void pushOnStack (Unit *);

  // Set and get the associated preprocessor parser
  void preprocessor (PreprocessorParser *pp);
  PreprocessorParser *preprocessor () const { return _parser; }

  // Include a file.
  Unit *includeFile (const char *name);
  Unit *handleIncludeOption (const char *name);

  // Include the next file of the given name.
  Unit *includeNextFile (const char *name);

  // returns true if the last included file was protected by an active
  // include guard
  bool guarded () const { return _guarded; }

  // Add a new non-system include path to list of include paths.
  void addIncludePath (const char *path);

  // Add a new system include path to list of include paths.
  void addSystemIncludePath (const char *path);

  // Get the system and non-system include directories.
  std::list<std::string> &getIncludePaths () { return _includeDirs; }

  // Check if the given directory is a known include directory.
  bool isIncludePath(const char *path) const;

  // Check if the given include directory is a system include directory.
  bool isSystemIncludePath(const char *path) const;
};


} // namespace Puma

#endif /* __pre_file_includer__ */
