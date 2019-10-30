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

#include "PumaIncludeExpander.h"
#include "LineDirectiveMgr.h"

#include "Puma/PreTreeNodes.h"
#include "Puma/PreSonIterator.h"
#include "Puma/MacroUnit.h"
#include "Puma/FileUnit.h"
#include "Puma/CProject.h"
#include "Puma/UnitManager.h"
#include "Puma/CCParser.h"
#include "Puma/CTranslationUnit.h"
#include "Puma/Token.h"
#include "Puma/Location.h"
using namespace Puma;

void PumaIncludeExpander::expand (const char *file) {

  // clear the resulting unit
  _unit.List::clear ();

  // preprocess the translation unit
  Unit *unit = _project.unitManager ().get (file, true);

  // return if the unit is empty (strange property of the UnitManager!)
  if (!unit || !unit->first ())
    return;

  // ok, we have a unit to expand
  _root = unit;
  
  // silently run the preprocessor
  CCParser parser;
  CTranslationUnit *tunit = parser.parse (*_root , _project, 2);
  
  _depth = 0;
  _ext_depth = 0;
  _files.push (File (_root));

  // search for include and manipulate them -> fills _tree and _nodes
  iterateNodes (tunit->cpp_tree ());

  // cleanup
  delete tunit; 

  // rewind the include stack until we are back at the input file
  rewind (0);

  // write the rest of the main unit
  finish ();
}

// Go through the nodes.
void PumaIncludeExpander::iterateNodes (PreTree* node) {
  PreSonIterator i (node); // the order is important!

  for (i.first (); ! i.isDone (); i.next ())
    i.currentItem ()->accept (*this);
}

// expand tokens from a unit (write them into the result unit)
void PumaIncludeExpander::write (Unit *unit, ACToken first, ACToken last) {
  ACUnit linedir (_err);
  if (_line_mgr.directive (linedir, unit, first)) {
    linedir << endu;
    linedir.cut (_unit, (Token*)linedir.first (), (Token*)linedir.last ());
  }
  // copy the tokens
  ACToken curr = first;
  while (curr) {
    // filter out #pragma once
    if (curr.is_directive () && strcmp ("#pragma", curr.text ()) == 0) {
      ACToken dir ((Token*)unit->next (curr.get()));
      bool have_pragma_once = false;
      while (dir && dir.is_directive () && strcmp ("#pragma", dir.text ()) != 0) {
        if (strcmp ("once", dir.text ()) == 0) {
          have_pragma_once = true;
          break;
        }
        dir = ACToken ((Token*)unit->next (dir.get()));
      }
      if (have_pragma_once && dir)
        curr = ACToken ((Token*)unit->next (dir.get()));
    }
    if (!curr)
      break;
    _unit.append (*(Token*)curr.get()->duplicate ());
    if (curr == last)
      break;
    curr = ACToken ((Token*)unit->next (curr.get()));
  }
//  List *copy = unit->copy (first, last);
//  copy->cut (_unit, (Token*)copy->first (), (Token*)copy->last ());
//  delete copy;
}

// write a string into the unit
void PumaIncludeExpander::write (const char *str) {
  ACUnit str_unit (_err);
  str_unit << str << endu;
  str_unit.cut (_unit, (Token*)str_unit.first (), (Token*)str_unit.last ());
}

// print the rest of the topmost unit
void PumaIncludeExpander::finish () {
  ACToken first = _files.top ()._next;
  ACToken last  (_files.top ()._unit->last ());
  if (first) {
    write (_files.top ()._unit, first, last);
    if (!last.get ()->is_whitespace ()) {
      const char *text = last.text ();
      if (text[strlen (text) - 1] != '\n' && text[strlen (text) - 1] != '\r')
        write ("\n"); // TODO: use Windows or Mac line endings on these platforms? Or guess?
    }
  }
}

// pop elements from the include stack until 'up_to' is the top
void PumaIncludeExpander::rewind (int up_to) {
  while (_depth > up_to) {
    finish ();
    _files.pop ();
    _depth--;
  }
}

// handle include directive node	
void PumaIncludeExpander::visitPreIncludeDirective_Pre (PreIncludeDirective* node)
{
  // this_unit is the unit where the include directive is located
  ACToken this_token (node->startToken ());
  Unit *this_unit   = this_token.unit ();
//  cout << "in " << this_unit->name () << " " << this_token->location () << " "
//    << _project.isBelow (this_unit) << endl;
        
  // manipulate project files only (and forced includes)
  if (_project.isBelow (this_unit) || this_unit == _root) {

    // include if expanded by preprocessor
    if (node->daughters () == 1) {
      
      Unit *included_unit = ((PreInclSemNode*)node->daughter (0))->unit ();
      bool included_below = _project.isBelow (included_unit);

      // a guarded unit is treaded as if it was not included
      // however, system headers are *always* included!
      // otherwise varying include relations in system headers on different
      // systems could break the ability to compile woven code on another
      // platform (e.g. woven Puma sources)
      if (included_below && ((PreInclSemNode*)node->daughter (0))->guarded ()) {
        included_unit = 0;
      }

//      cout << "included: " << included_unit->name () << " " <<
//        _project.isBelow (included_unit) << endl; 

      if (_ext_depth) {
//        cout << "node depth = " << node->depth () << endl;
//        cout << "ext depth  = " << _ext_depth << endl;
        if (node->depth () < _ext_depth) {
          // we are back!
          _ext_depth = 0;
//          cout << "cleared last" << endl;
        }
        else {
          // problem: we are in a project file, which was included by an external
          return;
        }
      }

      // expand includes of project files only
      if (!included_unit || included_below) {

        if (_depth > node->depth ())
          rewind (node->depth ());

        // advance the current position up the include
        ACToken first = _files.top ()._next;
        if (first && first != this_token) {
          ACToken last ((Token*)this_unit->prev (this_token.get()));
          write (this_unit, first, last);
        }
        ACToken last = ACToken (node->endToken ());
        // this might be a macro-generated include -> find the correct next tok
        if (last.is_macro_generated ()) {
          last = ACToken (((MacroUnit*)last.unit ())->ExpansionEnd (last.get()));
          assert (last && last.unit () == this_unit);
        }
        _files.top ()._next = ACToken ((Token*)this_unit->next (last.get()));
        
        if (included_unit) {
          // enter the next include file
          _files.push (File (included_unit));
          _depth++;
        }
      }
      else {
        // remember that we leave the project now
//          cout << "_last = " << this_unit->name () << endl;
        if (!_ext_depth) {
          _ext_depth = node->depth () + 1; // depth of external includes
//            cout << "ext_depth = " << _ext_depth << endl;
        }
      }
    }
    // include was e.g. in false ifdef branch
    else {
//      cout << "no daughter" << endl;
    }
  }
  else {
    // in external file!
    if (node->daughters () == 1) {
      Unit *included_unit = ((PreInclSemNode*)node->daughter (0))->unit ();
      if (included_unit && _project.isBelow (included_unit)) {
        // oops, an external file includes an internal => error!
        _err << sev_error << this_token.location ()
             << "internal file '" << included_unit->name ()
             << "' is included by external '" << this_unit->name () << "'"
             << endMessage;
        return;
      }
    }
  }
}
