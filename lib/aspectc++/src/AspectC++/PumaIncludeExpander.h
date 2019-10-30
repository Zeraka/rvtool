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

#ifndef __puma_include_expander_h__
#define __puma_include_expander_h__

// Visitor, which expands all project-local includes 

#include "Puma/PreVisitor.h"
#include "Puma/BCStack.h"
#include "Puma/Token.h"
#include "Puma/Unit.h"
#include "ACUnit.h"
#include "ACToken.h"
#include "ACErrorStream.h"

namespace Puma {
  class CProject;
  class Token;
} // namespace Puma

class LineDirectiveMgr;

class PumaIncludeExpander : public Puma::PreVisitor {

  struct File {
    Puma::Unit *_unit;
    ACToken _next;
    File (Puma::Unit *u = 0) : _unit (u), _next (0) {
      if (_unit)
        _next = ACToken ((Puma::Token*)_unit->first ());
    }
  };

  ACErrorStream &_err;
  Puma::CProject &_project;
  LineDirectiveMgr &_line_mgr;
  ACUnit _unit;
  Puma::BCStack<File> _files;
  int _depth;
  int _ext_depth;
  Puma::Unit *_root;

  // Go through the nodes of the syntax tree.
  void iterateNodes (Puma::PreTree*);
  
  // expand tokens from a unit (write them into the result unit)
  void write (Puma::Unit *unit, ACToken first, ACToken last);

  // write a string into the unit
  void write (const char *str);

  // print the rest of the topmost unit
  void finish ();

  // pop elements from the include stack until depth 'up_to'
  void rewind (int up_to);

  // Visiting the parts of the preprocessor syntax tree.
  
  void visitPreIncludeDirective_Pre (Puma::PreIncludeDirective*);

public:
    
  PumaIncludeExpander (ACErrorStream &err, Puma::CProject &project,
		   LineDirectiveMgr &ldm) :
    _err (err), _project (project), _line_mgr (ldm), _unit (err), _root (0) {}
 
  virtual ~PumaIncludeExpander () {}

  void expand (const char *file);
  ACUnit &unit () { return _unit; }
};  	    

#endif // __puma_include_expander_h__
