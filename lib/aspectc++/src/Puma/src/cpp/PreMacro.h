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

#ifndef __pre_macro__
#define __pre_macro__

// Preprocessor macro class. There are two types of macros. The first
// is called a simple macro or a symbolic constant macro. It consists
// only of a macro name and the macro body. The second type of macros
// is called a function-like macro. In addition it consists of a list 
// of arguments.

#include "Puma/ListElement.h"
#include "Puma/Location.h"
#include "Puma/DString.h"
#include "Puma/Chain.h"
#include "Puma/Unit.h"

#include <list>
#include <string>

namespace Puma {


class PreTree;
class CScanner;

class PreMacro : public Chain {
public:
  typedef std::list<std::string> MacroArgs;

  enum {
    FUNCTION_MACRO = 1,                // Function-like macro.
    VAR_ARG_MACRO  = 2,                // Variable number of arguments.
    LOCK_MACRO     = 4,                // Macro cannot be redefined or undefined.
    INHIBIT_MACRO  = 8                 // Macro cannot be defined.
  };

private:
  const char*   _body;                 // Macro body.
  DString       _name;
  char          _flags;                // Macro flags.
  Location      _location;             // Location of the #define directive.
  MacroArgs    *_arguments;            // The arguments array.
  Unit          _body_unit;            // The unit representation of the body

public:
  // Constructors for function-like macros.
  PreMacro (const DString &, PreTree *, const char *, char = 0);
  PreMacro (const DString &, MacroArgs *, const char *, char = 0);
        
  // Constructor for simple macros (constants).
  PreMacro (const DString &, const char *, char = 0);
        
  PreMacro (const PreMacro &);
  virtual ~PreMacro ();

  // Get macro attributes.
  const DString &getName () const { return _name; }
  const char *getBody () const { return _body; }
  const Unit &getBodyUnit (CScanner &scanner) {
    if (_body_unit.empty ())
      buildUnit (scanner);
    return _body_unit;
  }
  short numArgs () const { return _arguments ? _arguments->size () : 0; }
  const MacroArgs *getArgs () const { return _arguments; }
  // add an argument: the string is still owned by the caller and not copied!
  void addArg (const std::string &arg_name);

  // Set macro attributes
  void setBody (const char *);
   
  // Query macro flags.
  bool varArgs () const     { return _flags & VAR_ARG_MACRO; }
  bool isFunction () const  { return _flags & FUNCTION_MACRO; }
  bool isInhibited () const { return _flags & INHIBIT_MACRO; }
  bool isLocked () const    { return _flags & LOCK_MACRO; }

  // Set/get #define location.
  Location location () const { return _location; }
  void location (Location l) { _location = l; }
  
private:
  // create the unit representation
  void buildUnit (CScanner &scanner);
};


} // namespace Puma

#endif /* __pre_macro__ */
