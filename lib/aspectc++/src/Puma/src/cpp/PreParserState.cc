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

#include "Puma/Unit.h"
#include "Puma/Token.h"
#include "Puma/SysCall.h"
#include "Puma/PreMacro.h"
#include "Puma/PreTree.h"
#include "Puma/TokenStream.h"
#include "Puma/ErrorStream.h"
#include "Puma/PreParserState.h"
#include "Puma/PreMacroManager.h"
#include "Puma/CCommentTokens.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

namespace Puma {


PreParserState::PreParserState () :
  _Line("__LINE__"), _File("__FILE__"),
  _Date("__DATE__"), _Time("__TIME__"),
  _IncLevel("__INCLUDE_LEVEL__"),
  _Defined("defined"), _Defined1("defined("),
  _LastDay(-1) {
  noFILE = noLINE = noDATE     = false;
  noTIME = noINCLUDE_LEVEL     = false;
  syntaxError = errorDirective = false;
  syntaxTree                   = (PreTree*) 0;
  sl_token                     = (Token*) 0;
  lastToken = currToken        = (Token*) 0;
  lastType = currType          = Token::ID_UNKNOWN;
  passOnToken                  = true;
  forcedIncludes               = false;
  parseMacroArgs               = false;
  newline                      = new Token (TOK_WSPACE, "\n");
}


// Check whether the macro name is valid.
bool PreParserState::checkMacroName (PreMacro* macro) {
  // Some predefined macros can be redefined. If so they are not
  // computed anymore by the preprocessor.
  if (macro->getName () == _IncLevel) {
    noINCLUDE_LEVEL = true;
  } else if (macro->getName () == _Line) {
    noLINE = true;
  } else if (macro->getName () == _Date) {
    noDATE = true;
  } else if (macro->getName () == _File) {
    noFILE = true;
  } else if (macro->getName () == _Time) {
    noTIME = true;

  // The special name `defined' can't be used for a macro.
  } else if (macro->getName () == _Defined ||
             macro->getName () == _Defined1) {
    *err << macro->location () << sev_error
         << "`defined' is not a valid macro name"
         << endMessage;
    return false;
  }
  return true;
}


// Calculate the number of files on the input stream.
int PreParserState::getIncludeLevel (TokenStream* stream) {
  int len = stream->length (), level = 0;
  for (int i = 0; i < len; i++)
    if (stream->fetch (i)->unit ()->isFile ())
      level++;
  // don't count the primary input file
  return level > 0 ? level-1 : 0;
}


// Check whether macro is one of the special predefined macros.
PreMacro* PreParserState::checkPredefined (PreMacro* macro, TokenStream* scanner, PreMacroManager& manager) {
  if (! macro)
    return 0;

  // Integer constant representing the depth of nesting in include files.
  if (macro == manager.getIncLevelMacro() && ! noINCLUDE_LEVEL) {
    char value[12];
    snprintf (value, 12, "%i", getIncludeLevel (scanner));
    macro->setBody(value);
  }

  // String constant representing the current input file.
  else if (macro == manager.getFileMacro() && ! noFILE) {
    Unit* unit = 0;
    for (int i = scanner->length () - 1; !unit && i >= 0; i--)
      if (scanner->fetch (i)->unit ()->isFile ())
        unit = scanner->fetch (i)->unit ();
    if (unit) {
      char* buffer = unit->name ();
      char* value  = new char[strlen (buffer) + 3];
      sprintf (value, "\"%s\"", buffer);
      macro->setBody(value);
      delete [] value;
    }
  }

  // Integer constant representing the current input line number.
  else if (macro == manager.getLineMacro() && ! noLINE) {
    char value[12];
    sprintf (value, "%i", currToken->location ().line ());
    macro->setBody(value);
  }

  // String constant representing the current date of the form
  // "Apr 1 1999".
  else if (macro == manager.getDateMacro() && ! noDATE) {
    time_t curtime = SysCall::time (NULL, err);
    tm* loctime = localtime (&curtime);
    if (_LastDay != loctime->tm_mday) {
      char buffer[16];
      char value[16];
      _LastDay = loctime->tm_mday;
      strftime (buffer, 12, "%b %d %Y", loctime);
      sprintf (value, "\"%s\"", buffer);
      macro->setBody(value);
    }
  }

  // String constant representing the current time of the form 
  // "12:30:45".
  else if (macro == manager.getTimeMacro() && ! noTIME) {
    char buffer[16];
    char value[16];
    time_t curtime = SysCall::time (NULL, err);
    tm* loctime = localtime (&curtime);
    strftime (buffer, 9, "%H:%M:%S", loctime);
    sprintf (value, "\"%s\"", buffer);
    macro->setBody(value);
  }

  return macro;
}


// Check whether macro is a "self-referential" macro.
bool PreParserState::isSelfReferentialMacro (PreMacro* macro, Array<PreMacro*>& macroStack) {
  // A special feature of ANSI Standard C is that the self-reference
  // is not considered a macro call, it is passed to the preprocessor
  // output unchanged.

  // Search macro on stack. If found it is a self-referential macro.
  for (int i = macroStack.length () - 1; i >= 0; i--) {
    PreMacro* current = macroStack.fetch (i);
    if (current == macro)
      return true;
  }
  return false;
}


// Put a macro on the macro stack
void PreParserState::putMacroOnStack (Array<PreMacro*> &macroStack, PreMacro *macro, int stream_len) {
  for (int i = macroStack.length (); i < stream_len; i++)
    macroStack[i] = 0;
  macroStack[stream_len-1] = macro;
}


// Remove macro from macro stack.
void PreParserState::removeMacroFromStack (Array<PreMacro*> &macroStack, const char* name) {
  for (int i = macroStack.length () - 1; i >= 0; i--) {
    PreMacro* macro = macroStack.fetch (i);
    if (macro && strcmp(name, macro->getName ()) == 0) {
      macroStack[i] = 0;
      break;
    }
  }
}


// Update the macro stack.
void PreParserState::updateMacroStack (Array<PreMacro*>& macroStack, int stream_len) {
  // Shrink macro stack according to the input stream.
  for (int i = macroStack.length () - 1; i > (stream_len-1); i--)
    macroStack.remove (i);
}


// Check if current parsing code of a macro expansion.
bool PreParserState::inMacroExpansion (Array<PreMacro*>& macroStack) {
  for (int i = macroStack.length () - 1; i >= 0; i--)
    if (macroStack.fetch (i) != 0)
      return true;
  return false;
}


} // namespace Puma
