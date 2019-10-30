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

#include "Puma/PreTreeToken.h"
#include "Puma/PreTree.h"
#include "Puma/PreMacro.h"
#include "Puma/StrCol.h"
#include "Puma/CScanner.h"


namespace Puma {


// Constructor for function-like macros.
PreMacro::PreMacro (const DString &name, PreTree *args, const char *body, char flags) {
  const char *text;

  // Get the macro name and body.
  _name = name;
  _body = body ? StrCol::dup (body) : (const char*)0;

  // It's a function-like macro.
  _flags = (char)FUNCTION_MACRO | flags;
    
  // Copy macro arguments.
  if (args) { 
    _arguments = new MacroArgs;
    for (int i = 0; i < args->sons (); i++) {
      text = ((PreTreeToken*) args->son (i))->token ()->text ();
      // Because the commas between the arguments are collected too 
      // they have to be filtered out.
      if (*text != ',')
        _arguments->push_back (text);
    } 

  // It's a function-like macro without any arguments.
  } else {
    _arguments = (MacroArgs*)0;
  }
}

        
// Constructor for function-like macros.
PreMacro::PreMacro (const DString &name, MacroArgs *args, const char *body, char flags) {
  // Get the macro name, arguments, and body.
  _name = name;
  _arguments = (args ? args : (MacroArgs*)0);
  _body = body ? StrCol::dup (body) : (const char*)0;

  // It's a function-like macro.
  _flags = (char)FUNCTION_MACRO | flags;
}


// Constructor for simple macros (constants).
PreMacro::PreMacro (const DString &name, const char *body, char flags) {
  _flags = flags;  

  _arguments = (MacroArgs*)0;

  // Get name and body.
  _name = name;
  _body = body ? StrCol::dup (body) : (const char*)0;
}


// Copy-constructor.
PreMacro::PreMacro (const PreMacro &macro) {
  // Copy macro name and body.
  _name = macro._name;
  _body = macro._body ? StrCol::dup (macro._body) : (const char*)0;
    
  // Copy macro attributes.
  _flags    = macro._flags;
  _location = macro._location;
    
  // Copy the arguments.
  _arguments = (macro._arguments ? new MacroArgs (*macro._arguments) : (MacroArgs*)0);
}

PreMacro::~PreMacro () {
  if (_body) 
    delete[] _body;
  if (_arguments) 
    delete _arguments;
}

void PreMacro::addArg (const std::string &arg_name) {
  if (! _arguments) {
    _arguments = new MacroArgs;
    // It's a function-like macro.
    _flags |= (char)FUNCTION_MACRO;
  }
  _arguments->push_back (arg_name);
}

void PreMacro::setBody (const char *s) {
  if (_body) 
    delete[] _body;
  _body = StrCol::dup(s);
  _body_unit.clear ();
}

void PreMacro::buildUnit (CScanner &scanner) {
  int i;
  const char *string = _body;
  
  // Skip leading white spaces.
  for (i = 0; StrCol::isSpace (string[i]); i++);
  string = &string[i];

  // Scan the string and fill the unit.
  scanner.allow_macro_ops ();
  scanner.fill_unit (string, _body_unit);
  scanner.allow_macro_ops (false);
}

} // namespace Puma
