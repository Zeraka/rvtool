// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#include "LineDirectiveMgr.h"

#ifdef FRONTEND_PUMA
#include "Puma/Token.h"
#include "Puma/CUnit.h"
#include "Puma/MacroUnit.h"
using namespace Puma;
#else
#include "clang/Basic/SourceLocation.h"
#endif

#ifdef FRONTEND_PUMA
bool LineDirectiveMgr::insert (Unit *unit, PumaToken token, bool after) {
  if (_config.noline ())
    return false;
  if (unit && token && unit->isMacroExp ()) {
    if (after)
      token = PumaToken (((MacroUnit*)unit)->ExpansionEnd (token.get()));
    else
      token = PumaToken (((MacroUnit*)unit)->ExpansionBegin (token.get()));
    unit  = token ? token.unit () : (Unit*)0;
  }
  if (!unit || unit->isMacroExp ()) {
    return false; // no chance :-(
  }
  CUnit line_directive (_err);
  directive (line_directive, unit, token);
  line_directive << endu;
  Unit *ins_unit = token.unit ();
  if (after)
    ins_unit->move (token.get(), line_directive);
  else
    ins_unit->move_before (token.get(), line_directive);
  return true;
}

bool LineDirectiveMgr::directive (ostream &out, Unit *unit, PumaToken token) {
  if (_config.noline ())
    return false;
  if (unit && token && unit->isMacroExp ()) {
    token = PumaToken (((MacroUnit*)unit)->ExpansionBegin (token.get()));
    unit  = token ? token.unit () : 0;
  }
  if (unit && unit->isMacroExp ()) {
    return false; // no chance :-(
  }
  // work around broken debug information resulting from #line directives with
  // an empty filename, e.g., #line 42 ""
  if (unit && (unit->name() == 0 || *(unit->name()) == 0)) {
    return false;
  }
  out << endl << "#line " << (token ? token.location ().line () : 1)
      << " \"";
  if (unit) {
    // path names in line directives always contain '/' and not '\'
    const char *curr = unit->name ();
    while (curr && *curr != '\0') {
      if (*curr == '\\')
        out << '/';
      else
        out << *curr;
      curr++;
    }
  }
  else {
    out << "<ac>";
  }
  out << "\"" << endl;
  return true;
}

#else

bool LineDirectiveMgr::directive (ostream &out, const clang::PresumedLoc &loc) {
  // clang needs #line directives to get the token locations right! (even with --no_line)
//  if (_config.noline ())
//    return false;
  if (!loc.isValid ())
    out << "\n#line 1 \"<ac>\"\n";
  else {
    out << "\n#line " << loc.getLine() << " \"";
    const char *curr = loc.getFilename();
    while (curr && *curr != '\0') {
      if (*curr == '\\')
        out << '/';
      else
        out << *curr;
      curr++;
    }
    out << "\"\n";
  }
  return true;
}

#endif
