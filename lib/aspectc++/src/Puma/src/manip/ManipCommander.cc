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

#include "Puma/ManipCommander.h"
#include "Puma/MacroUnit.h"
#include "Puma/CTree.h"
#include <string.h>

namespace Puma {


ManipCommander::~ManipCommander () { 
  clear (); 
}


ManipError ManipCommander::valid () const { 
  return _commands.valid (_manip_mode);
}
 

void ManipCommander::clear () {
  for (std::list<Unit*>::iterator iter = _buffers.begin (); iter != _buffers.end ();
       ++iter)
    delete *iter;
  _buffers.clear ();
  _commands.clear ();
  _ignore = MIM_NONE;
  _expanded_macro_units.clear ();
}


Unit *ManipCommander::newBuffer () {
  Unit *buffer = new Unit ();
  addBuffer (buffer);
  return buffer;
}


void ManipCommander::addBuffer (Unit *buffer) {
  assert (buffer);
  _buffers.push_back (buffer);
}


Token *ManipCommander::getStartToken (Token *token) {
  if (token && token->is_macro_generated ()) {
    MacroUnit *macro = (MacroUnit*)token->belonging_to ();
    if (macro) {
      if (_manip_mode != MM_EXPAND_MACROS) {
        Token *macro_call_start = macro->ExpansionBegin (token);
        if (macro_call_start)
          token = macro_call_start;
      }
      else if (token->is_macro_generated ())
        auto_expand_macro (macro);
    }
  }
  return token;
}


Token *ManipCommander::getEndToken (Token *token) {
  if (token && token->is_macro_generated ()) {
    MacroUnit *macro = (MacroUnit*)token->belonging_to ();
    if (macro) {
      if (_manip_mode != MM_EXPAND_MACROS) {
        Token *macro_call_end = macro->ExpansionEnd (token);
        if (macro_call_end)
          token = macro_call_end;
      }
      else if (token->is_macro_generated ())
        auto_expand_macro (macro);
    }
  }
  return token;
}


void ManipCommander::auto_expand_macro (MacroUnit *macro) {
  std::pair<std::set<MacroUnit*>::iterator,bool> ret = _expanded_macro_units.insert (macro);
  if (!ret.second) return; // macro has already been expanded

  // expand enclosing macros recursively
  Token *macro_call_begin = macro->MacroBegin ();
  Token *macro_call_end = macro->MacroEnd ();
  if (macro_call_begin->is_macro_generated ()) {
    MacroUnit *macro_call_unit = (MacroUnit*)macro_call_begin->belonging_to ();
    auto_expand_macro (macro_call_unit);
  }

  // issue tranformation commands that replace the macro call by the macro expansion unit
  add (new CMoveBeforeManipulator ((Token*)macro->first (),
    (Token*)macro->last (), macro_call_begin));
  if (macro_call_begin->unit () == macro_call_end->unit ())
    add (new KillManipulator (macro_call_begin, macro->MacroEnd ()));
  else {
    // The macro name was generated; arguments are from somewhere else
    Unit *caller = macro_call_begin->unit ();
    // Remove start of macro call
    add (new KillManipulator (macro_call_begin, (Token*)caller->last ()));
    // Remove end of macro call
    while (caller->isMacroExp () &&
        ((MacroUnit*)caller)->MacroEnd ()->unit () != macro_call_end->unit ())
      caller = ((MacroUnit*)caller)->MacroBegin ()->unit ();
    add (new KillManipulator (macro_call_end->unit ()->next (((MacroUnit*)caller)->MacroEnd ()),
        macro_call_end));
  }
}


void ManipCommander::commit () {
  _commands.manipulate ();
  clear ();
}


void ManipCommander::commit (ManipController &mc) {
  Manipulator *curr = (Manipulator*) _commands.first ();
  Array<Token*> *arguments;
  const char *op;
    
  for (; curr; curr = (Manipulator*) _commands.next (curr)) {
    op = curr->op ();
    arguments = &curr->args ();

    if (op && arguments) {
      if (op == Manipulator::paste_op || 
          op == Manipulator::paste_before_op) {
        if (! mc.permitted (arguments->get (0))) continue;
      } else if (op == Manipulator::cpp_copy_op || 
                 op == Manipulator::cpp_copy_before_op) {
        if (! mc.permitted (arguments->get (2))) continue;
      } else if (op == Manipulator::cut_op || 
                 op == Manipulator::kill_op || 
                op == Manipulator::cpp_kill_op) {
        if (! mc.permitted (arguments->get (0))) continue;
        if (! mc.permitted (arguments->get (1))) continue;
      } else if (op == Manipulator::cpp_move_op || 
                 op == Manipulator::cpp_move_before_op) {
        if (! mc.permitted (arguments->get (0))) continue;
        if (! mc.permitted (arguments->get (1))) continue;
        if (! mc.permitted (arguments->get (2))) continue;
      } else if (op == Manipulator::cpp_replace_op || 
                 op == Manipulator::cpp_swap_op) {
        if (! mc.permitted (arguments->get (0))) continue;
        if (! mc.permitted (arguments->get (1))) continue;
        if (! mc.permitted (arguments->get (2))) continue;
        if (! mc.permitted (arguments->get (3))) continue;
      }
    }

    // Do the manipulation.
    curr->manipulate ();
  }
    
  clear ();
}


void ManipCommander::add (Manipulator *manip) {
  assert (manip);
  if (manip_mode () == MM_EXPAND_MACROS)
    manip->ignore_mask ((ManipIgnoreMask)(_ignore | MIM_MACRO));
  else
    manip->ignore_mask (_ignore);
  _commands.append (manip);
}


//////////////////////////////////////////////////////////////////////
// Low level manipulators (may use extern buffers too). //////////////
//////////////////////////////////////////////////////////////////////


void ManipCommander::cut (Unit *buffer, Token *from, Token *to) {
  from = getStartToken (from);
  to = getEndToken (to);
  Manipulator *cut = new CutManipulator (buffer, from, to ? to : from);
  add (cut);
}


void ManipCommander::copy (Unit *buffer, Token *from, Token *to) {
  from = getStartToken (from);
  to = getEndToken (to);
  Manipulator *copy = new CopyManipulator (buffer, from, to ? to : from);
  add (copy);
}


void ManipCommander::kill (Token *from, Token *to) {
  from = getStartToken (from);
  to = getEndToken (to);
  Manipulator *kill = new KillManipulator (from, to ? to : from);
  add (kill);
}


void ManipCommander::paste (Token *at, Unit *buffer) {
  at = getEndToken (at);
  Manipulator *paste = new PasteManipulator (buffer, at);
  add (paste);
}


void ManipCommander::paste_before (Token *at, Unit *buffer) {
  at = getStartToken (at);
  Manipulator *paste_before = new PasteBeforeManipulator (buffer, at);
  add (paste_before);
}
        

//////////////////////////////////////////////////////////////////////
// Composed manipulators. ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void ManipCommander::paste (Token *at, Token *which) {
  copy (which, which, at);
}


void ManipCommander::paste_before (Token *at, Token *which) {
  copy_before (which, which, at);
}


void ManipCommander::replace (Token *which, Token *with) {
  paste (which, with);
  kill (which);
}


void ManipCommander::copy (Token *from, Token *to, Token *at) {
  from = getStartToken (from);
  to = getEndToken (to);
  at = getEndToken (at);
  Manipulator *copy = new CCopyManipulator (from, to, at);
  add (copy);
}


void ManipCommander::copy_before (Token *from, Token *to, Token *at) {
  from = getStartToken (from);
  to = getEndToken (to);
  at = getStartToken (at);
  Manipulator *copy_before = new CCopyBeforeManipulator (from, to, at);
  add (copy_before);
}


void ManipCommander::move (Token *from, Token *to, Token *at) {
  from = getStartToken (from);
  to = getEndToken (to);
  at = getEndToken (at);
  Manipulator *move = new CMoveManipulator (from, to, at);
  add (move); 
}


void ManipCommander::move_before (Token *from, Token *to, Token *at) {
  from = getStartToken (from);
  to = getEndToken (to);
  at = getStartToken (at);
  Manipulator *move_before = new CMoveBeforeManipulator (from, to, at);
  add (move_before); 
}


void ManipCommander::replace (Token *wf, Token *wt, Token *tf, Token *tt) {
  wf = getStartToken (wf);
  wt = getEndToken (wt);
  tf = getStartToken (tf);
  tt = getEndToken (tt);
  Manipulator *replace = new CReplaceManipulator (wf, wt, tf, tt);
  add (replace);
}


void ManipCommander::swap (Token *wf, Token *wt, Token *tf, Token *tt) {
  wf = getStartToken (wf);
  wt = getEndToken (wt);
  tf = getStartToken (tf);
  tt = getEndToken (tt);
  Manipulator *swap = new CSwapManipulator (wf, wt, tf, tt);
  add (swap);
}



//////////////////////////////////////////////////////////////////////
// Special C++ syntax tree manipulator interfaces. ///////////////////
//////////////////////////////////////////////////////////////////////


void ManipCommander::kill (CTree *what) {
  Token *f = getStartToken (what ? what->token () : (Token*)0);
  Token *t = getEndToken (what ? what->end_token () : (Token*)0);
  Manipulator *kill = new CKillManipulator (f, t);
  add (kill);
}


void ManipCommander::copy (CTree *from, CTree *to) {
  Token *f = getStartToken (from ? from->token () : (Token*)0); 
  Token *t = getEndToken (from ? from->end_token () : (Token*)0);
  Token *a = getEndToken (to ? to->end_token () : (Token*)0); 
  Manipulator *copy = new CCopyManipulator (f, t, a);
  add (copy);
}


void ManipCommander::copy_before (CTree *from, CTree *to) {
  Token *f = getStartToken (from ? from->token () : (Token*)0); 
  Token *t = getEndToken (from ? from->end_token () : (Token*)0);
  Token *a = getStartToken (to ? to->token () : (Token*)0); 
  Manipulator *copy_before = new CCopyBeforeManipulator (f, t, a);
  add (copy_before);
}


void ManipCommander::move (CTree *from, CTree *to) {
  Token *f = getStartToken (from ? from->token () : (Token*)0); 
  Token *t = getEndToken (from ? from->end_token () : (Token*)0);
  Token *a = getEndToken (to ? to->end_token () : (Token*)0); 
  Manipulator *move = new CMoveManipulator (f, t, a);
  add (move); 
}


void ManipCommander::move_before (CTree *from, CTree *to) {
  Token *f = getStartToken (from ? from->token () : (Token*)0); 
  Token *t = getEndToken (from ? from->end_token () : (Token*)0);
  Token *a = getStartToken (to ? to->token () : (Token*)0); 
  Manipulator *move_before = new CMoveBeforeManipulator (f, t, a);
  add (move_before); 
}


void ManipCommander::replace (CTree *what, CTree *with) {
  Token *wf = getStartToken (what ? what->token () : (Token*)0); 
  Token *wt = getEndToken (what ? what->end_token () : (Token*)0);
  Token *tf = getStartToken (with ? with->token () : (Token*)0); 
  Token *tt = getEndToken (with ? with->end_token () : (Token*)0); 
  Manipulator *replace = new CReplaceManipulator (wf, wt, tf, tt);
  add (replace);
}


void ManipCommander::swap (CTree *what, CTree *with) {
  Token *wf = getStartToken (what ? what->token () : (Token*)0); 
  Token *wt = getEndToken (what ? what->end_token () : (Token*)0);
  Token *tf = getStartToken (with ? with->token () : (Token*)0); 
  Token *tt = getEndToken (with ? with->end_token () : (Token*)0); 
  Manipulator *swap = new CSwapManipulator (wf, wt, tf, tt);
  add (swap);
}


} // namespace Puma
