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

#include "Puma/Manipulators.h"
#include "Puma/MacroUnit.h"
#include "Puma/PreParser.h"
#include <assert.h>

namespace Puma {


//////////////////////////////////////////////////////////////////////
// Manipulator identification constants. /////////////////////////////
//////////////////////////////////////////////////////////////////////

const char *Manipulator::sequence_op             = "sequence";
const char *Manipulator::copy_op             = "copy";
const char *Manipulator::cut_op                    = "cut";
const char *Manipulator::paste_op            = "paste";
const char *Manipulator::paste_before_op    = "paste_before";
const char *Manipulator::move_op            = "move";
const char *Manipulator::move_before_op     = "move_before";
const char *Manipulator::kill_op            = "kill";
const char *Manipulator::cpp_copy_op            = "cpp_copy";
const char *Manipulator::cpp_copy_before_op = "cpp_copy_before";
const char *Manipulator::cpp_kill_op            = "cpp_kill";
const char *Manipulator::cpp_move_op            = "cpp_move";
const char *Manipulator::cpp_move_before_op = "cpp_move_before"; 
const char *Manipulator::cpp_replace_op            = "cpp_replace";
const char *Manipulator::cpp_swap_op            = "cpp_swap";


// I hope there is a first arg ...
Location ManipError::location () { 
  if (faulty () && faulty ()->args ().length ())
    if (faulty ()->args ()[0])
      return faulty ()->args ()[0]->location ();
  
  static Location loc;
  return loc; 
}


// Print a manipulation error code
void ManipError::print (std::ostream &os) const {
  switch (errorno ()) {
    case FATAL:
      os << "fatal error: Can't do `" << (faulty () ? faulty ()->op () : "<unknown>")
         << "' manipulation"; break;
    case MACRO_GEN:
      os << "error: Can't `" << (faulty () ? faulty ()->op () : "<unknown>")
         << "' if start/end token is macro generated"; break;
    case BAD_ARG:
      os << "error: Bad argument(s) for the `"
         << (faulty () ? faulty ()->op () : "<unknown>") 
         << "' manipulator"; break;
    case UNBALANCED:
      os << "error: Unbalanced preprocessor directive(s) "
         << "found. Can't do `" << (faulty () ? faulty ()->op () : "<unknown>")
         << "' manipulation"; break;
  }

  if (faulty ()) {
    os << " (Args: ";
    for (int a = 0; a < faulty ()->args ().length (); a++) {
      Token *tok = faulty ()->args ().get (a);
      if (!tok) continue;
      if (a > 0) os << ", ";
      Unit *unit = (Unit*)tok->belonging_to ();
      if (unit->isMacroExp ()) {
//        MacroUnit *munit = (MacroUnit*)unit;
//        tok = munit->ExpansionBegin ((Token*)munit->first ());
        os << "macro at ";
      }
      os << tok->location ();
    }
    os << ").";
  }
}


// Return positiv value if the tokens between start and end are 
// valid, that means:
// - start token is not NULL
// - start and end token belong to the same unit and aren't result
//   of a macro expansion
// - only balanced preprocessor directives inside of the manipulation
//   range

int Manipulator::valid (Token *start, Token *end, ManipMode mode) const {
  if (! start)                            return ManipError::BAD_ARG;
  if (!(_ignore & MIM_MACRO) && start->is_macro_generated ()) return ManipError::MACRO_GEN;
  if (! end)                            return ManipError::OK;
  if (!(_ignore & MIM_MACRO) && end->is_macro_generated ())   return ManipError::MACRO_GEN;
  if (start == end)                    return ManipError::OK;

  if (_ignore & MIM_UNBALANCED || mode == MM_EXPAND_MACROS)
    return ManipError::OK;
    
  int balance = 0;
  Token *token = start;
  Unit *unit   = token->unit ();
  while (token) {
    if (token->is_preprocessor ()) {
      switch (token->type ()) {
        case TOK_PRE_IF:
        case TOK_PRE_IFDEF:
        case TOK_PRE_IFNDEF:
          balance++; 
          break;
        case TOK_PRE_ELIF:
          if (! balance)
            return ManipError::UNBALANCED;
          break;
        case TOK_PRE_ELSE:
          if (! balance)
            return ManipError::UNBALANCED;
          break;    
        case TOK_PRE_ENDIF:
          balance--; 
          break;
      }
    }
                
    if (token == end) {
      if (balance == 0)
         return ManipError::OK;
      else
        return ManipError::UNBALANCED;
    }

    token = (Token*) unit->next (token);
  }
    
  // Should only be reached on fatal errors (maybe start and end 
  // token belong to different units).
  return ManipError::FATAL;
}


//////////////////////////////////////////////////////////////////////
// The manipulator sequence. /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


ListElement *ManipulatorSequence::duplicate () {
  ManipulatorSequence *dup = new ManipulatorSequence ();
  *dup += *this;
  dup->_rules += _rules;
  return dup;
}


// Ask every manipulator of the sequence whether its manipulation
// can be done.

ManipError ManipulatorSequence::valid (ManipMode mode) const {
  ManipError error;
  Manipulator *curr = (Manipulator*) first ();
    
  for (; curr; curr = (Manipulator*) next (curr))
    if (curr->valid (mode).errorno () != ManipError::OK)
      return curr->valid (mode);

  error.errorno (error.OK);
  return error;
}


// Special manipulator class that manages a sequence  of
// manipulator commands.

void ManipulatorSequence::manipulate () {
  Manipulator *curr = (Manipulator*) first ();
  Array<Token*> *arguments;
  ManipError error;
  const char *op;

  for (; curr; curr = (Manipulator*) next (curr)) {
    // Check the arguments of a manipulator.
    checkArgs (error, curr);
    if (error.errorno () != error.OK)
      continue;

    // Do the manipulation.
    curr->manipulate ();

    arguments = &curr->args ();
    op = curr->op ();

    if (! op || ! arguments) continue;

    // Update the rule table.
    if (op == cut_op || op == kill_op || op == cpp_kill_op) {
      _rules.add (arguments->get (0)/*, arguments->get (2)*/);
      _rules.add (arguments->get (1)/*, arguments->get (3)*/);
    //} else if (op == cpp_move_op || op == cpp_move_before_op) {
      //_rules.add (arguments->get (0), arguments->get (3));
      //_rules.add (arguments->get (1), arguments->get (4));
    } else if (op == cpp_replace_op) {
      _rules.add (arguments->get (0), arguments->get (4));
      _rules.add (arguments->get (1), arguments->get (5));
    } else if (op == cpp_swap_op) {
      _rules.add (arguments->get (0), arguments->get (4));
      _rules.add (arguments->get (1), arguments->get (5));
      _rules.add (arguments->get (2), arguments->get (6));
      _rules.add (arguments->get (3), arguments->get (7));
    }
  }
  clear ();
  _rules.clear ();
}


// Check the arguments of a manipulator.

void ManipulatorSequence::checkArgs (ManipError &error, Manipulator *curr) {
  Token *arg, *back;
  Array<Token*> array (4);
  int length = curr->args ().length ();
        
  // Check every argument of the current manipulator.
  for (int i = 0; i < length; i++) {
    // Get the current argument.
    arg = curr->args ()[i];
        
    // Check the current argument.
    back = _rules.get (arg);
    
    // If we've got a NULL pointer, the manipulation would
    // be done on a not existing token => fatal error.
    if (! back) {
      error.faulty (curr);
      error.errorno (ManipError::BAD_ARG);
      return;
    }
        
    // Build the correction argument array.
    array.append (back);
  }
    
  // Correct the arguments.
  curr->args (array);

  // All is right.
  error.errorno (ManipError::OK);
}
 

//////////////////////////////////////////////////////////////////////
// The `copy' manipulator. ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void CopyManipulator::args (Array<Token*> &arguments) {
  _from = arguments[0];
  _to   = arguments[1];
  _arguments[0] = _from;
  _arguments[1] = _to;
}

CopyManipulator::CopyManipulator (Unit *buffer, Token *from, Token *to) {
  _buffer = buffer;
  _from   = from;
  _to     = to;
  _arguments.append (_from);
  _arguments.append (_to);
}

ManipError CopyManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CopyManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }

  error.errorno (Manipulator::valid (_from, _to, mode));
  return error;
}

void CopyManipulator::manipulate () {
  assert (_buffer);
  assert (_from);
  assert (_to);
    
  List *l = _from->belonging_to ();
  if (! l) l = _buffer;
  List *res = l->copy (_from, _to);
  ((Unit*) l)->state ().modified ();

  assert (res);
    
  *_buffer += *res;
  delete res;
}


//////////////////////////////////////////////////////////////////////
// The `cut' manipulator. ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void CutManipulator::args (Array<Token*> &arguments) {
  _from = arguments[0];
  _to   = arguments[1];
  _arguments[0] = _from;
  _arguments[1] = _to;
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

CutManipulator::CutManipulator (Unit *buffer, Token *from, Token *to) {
  _buffer = buffer;
  _from   = from;
  _to     = to;
  _arguments.append (_from);
  _arguments.append (_to);
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

ManipError CutManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CutManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }

  error.errorno (Manipulator::valid (_from, _to, mode));
  return error;
}

void CutManipulator::manipulate () {
  assert (_buffer);
  assert (_from);
  assert (_to);
    
  List *l = _from->belonging_to ();
  if (! l) l = _buffer;

//  _arguments[2] = (Token*) l->prev (_from);
//  if (! _arguments[2]) 
//    _arguments[2] = (Token*) l->next (_from);
//  _arguments[3] = (Token*) l->next (_to);
//  if (! _arguments[3]) 
//    _arguments[3] = (Token*) l->prev (_to);

  l->cut (*_buffer, _from, _to);
  ((Unit*) l)->state ().modified ();
}


//////////////////////////////////////////////////////////////////////
// The `kill' manipulator. ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void KillManipulator::args (Array<Token*> &arguments) {
  _from = arguments[0];
  _to   = arguments[1];
  _arguments[0] = _from;
  _arguments[1] = _to;
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

KillManipulator::KillManipulator (Token *from, Token *to) {
  _from   = from;
  _to     = to;
  _arguments.append (_from);
  _arguments.append (_to);
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

ManipError KillManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((KillManipulator*) this);

  error.errorno (Manipulator::valid (_from, _to, mode));
  return error;
}

void KillManipulator::manipulate () {
  assert (_from);
  assert (_to);
  
  List *l = _from->belonging_to ();
  if (l) {
    //_arguments[2] = (Token*) l->prev (_from);
    //if (! _arguments[2]) 
    //  _arguments[2] = (Token*) l->next (_from);
    //_arguments[3] = (Token*) l->next (_to);
    //if (! _arguments[3]) 
    //  _arguments[3] = (Token*) l->prev (_to);

    l->kill (_from, _to);
    ((Unit*) l)->state ().modified ();
  } else {
    //_arguments[2] = (Token*) 0;
    //_arguments[3] = (Token*) 0;

    Unit unit;
    unit.kill (_from, _to);
  }
}


//////////////////////////////////////////////////////////////////////
// The `paste' manipulator. //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void PasteManipulator::args (Array<Token*> &arguments) {
  _at = arguments[0];
  _arguments[0] = _at;
}

PasteManipulator::PasteManipulator (Unit *buffer, Token *at) {
  _buffer = buffer;
  _at   = at;
  _arguments.append (_at);
}

ManipError PasteManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((PasteManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }
  
  error.errorno (Manipulator::valid (_at, (Token*) 0, mode));
  return error;
}

void PasteManipulator::manipulate () {
  assert (_buffer);
  assert (_at);
  
  List *l = _at->belonging_to ();
  if (! l) l = _buffer;

  l->paste (_at, (const Unit&) *_buffer);
  ((Unit*) l)->state ().modified ();
}


//////////////////////////////////////////////////////////////////////
// The `paste_before' manipulator. ///////////////////////////////////
//////////////////////////////////////////////////////////////////////


void PasteBeforeManipulator::args (Array<Token*> &arguments) {
  _at = arguments[0];
  _arguments[0] = _at;
}

PasteBeforeManipulator::PasteBeforeManipulator (Unit *buffer, Token *at) {
  _buffer = buffer;
  _at   = at;
  _arguments.append (_at);
}

ManipError PasteBeforeManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((PasteBeforeManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }
  
  error.errorno (Manipulator::valid (_at, (Token*) 0, mode));
  return error;
}

void PasteBeforeManipulator::manipulate () {
  assert (_buffer);
  assert (_at);
  
  List *l = _at->belonging_to ();
  if (! l) l = _buffer;

  l->paste_before (_at, (const Unit&) *_buffer);
  ((Unit*) l)->state ().modified ();
}


//////////////////////////////////////////////////////////////////////
// The `move' manipulator. ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void MoveManipulator::args (Array<Token*> &arguments) {
  _at = arguments[0];
  _arguments[0] = _at;
}

MoveManipulator::MoveManipulator (Unit *buffer, Token *at) {
  _buffer = buffer;
  _at   = at;
  _arguments.append (_at);
}

ManipError MoveManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((MoveManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }
  
  error.errorno (Manipulator::valid (_at, (Token*) 0, mode));
  return error;
}

void MoveManipulator::manipulate () {
  assert (_buffer);
  assert (_at);
  
  List *l = _at->belonging_to ();
  if (! l) l = _buffer;

  l->move (_at, (Unit&) *_buffer);
  ((Unit*) l)->state ().modified ();
}


//////////////////////////////////////////////////////////////////////
// The `move_before' manipulator. ////////////////////////////////////
//////////////////////////////////////////////////////////////////////


void MoveBeforeManipulator::args (Array<Token*> &arguments) {
  _at = arguments[0];
  _arguments[0] = _at;
}

MoveBeforeManipulator::MoveBeforeManipulator (Unit *buffer, Token *at) {
  _buffer = buffer;
  _at   = at;
  _arguments.append (_at);
}

ManipError MoveBeforeManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((MoveBeforeManipulator*) this);

  if (! _buffer) {                // Error, illegal buffer.
    error.errorno (error.FATAL); 
    return error;
  }
  
  error.errorno (Manipulator::valid (_at, (Token*) 0, mode));
  return error;
}

void MoveBeforeManipulator::manipulate () {
  assert (_buffer);
  assert (_at);
  
  List *l = _at->belonging_to ();
  if (! l) l = _buffer;

  l->move_before (_at, (Unit&) *_buffer);
  ((Unit*) l)->state ().modified ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `kill' manipulator. ///////////////////////////
//////////////////////////////////////////////////////////////////////


void CKillManipulator::args (Array<Token*> &arguments) {
  _from = arguments[0];
  _to   = arguments[1];
  _arguments[0] = _from;
  _arguments[1] = _to;
  assert (_from);
  assert (_to);
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

CKillManipulator::CKillManipulator (CTree *what) { 
  _from = what ? what->token () : (Token*) 0; 
  _to   = what ? what->end_token () : (Token*) 0; 
  _arguments.append (_from);
  _arguments.append (_to);
  assert (_from);
  assert (_to);
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

CKillManipulator::CKillManipulator (Token *from, Token *to) { 
  _from = from; 
  _to   = to; 
  _arguments.append (_from);
  _arguments.append (_to);
  assert (_from);
  assert (_to);
  assert (_from->belonging_to ());
  assert (_to->belonging_to ());
}

ManipError CKillManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CKillManipulator*) this);

  error.errorno (Manipulator::valid (_from, _to, mode));
  return error;
}

void CKillManipulator::manipulate () {
  assert (_from);
  
  KillManipulator kill (_from, _to);
  kill.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `copy' manipulator. ///////////////////////////
//////////////////////////////////////////////////////////////////////


void CCopyManipulator::args (Array<Token*> &arguments) {
  _from_from = arguments[0];
  _from_to   = arguments[1];
  _to        = arguments[2];
  _arguments[0] = _from_from;
  _arguments[1] = _from_to;
  _arguments[2] = _to;
}

CCopyManipulator::CCopyManipulator (CTree *from, CTree *to) { 
  _from_from = from ? from->token () : (Token*) 0; 
  _from_to   = from ? from->end_token () : (Token*) 0;
  _to        = to ? to->end_token () : (Token*) 0; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

CCopyManipulator::CCopyManipulator (Token *from, Token *to, Token *at) { 
  _from_from = from; 
  _from_to   = to; 
  _to        = at;
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

ManipError CCopyManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CCopyManipulator*) this);

  if (! _to) {                        // Error, illegal token.
    error.errorno (error.BAD_ARG);
    return error;
  }

  error.errorno (Manipulator::valid (_from_from, _from_to, mode));
  return error;
}

void CCopyManipulator::manipulate () {
  assert (_from_from);
  assert (_to);
  
  Unit buffer; 
  
  CopyManipulator copy (&buffer, _from_from, _from_to);
  copy.manipulate ();
  
  PasteManipulator paste (&buffer, _to);
  paste.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `copy_before' manipulator. ////////////////////
//////////////////////////////////////////////////////////////////////


void CCopyBeforeManipulator::args (Array<Token*> &arguments) {
  _from_from = arguments[0];
  _from_to   = arguments[1];
  _to        = arguments[2];
  _arguments[0] = _from_from;
  _arguments[1] = _from_to;
  _arguments[2] = _to;
}

CCopyBeforeManipulator::CCopyBeforeManipulator (CTree *from, CTree *to) { 
  _from_from = from ? from->token () : (Token*) 0; 
  _from_to   = from ? from->end_token () : (Token*) 0;
  _to        = to ? to->token () : (Token*) 0; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

CCopyBeforeManipulator::CCopyBeforeManipulator (Token *from, Token *to, Token *at) { 
  _from_from = from; 
  _from_to   = to; 
  _to        = at;
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

ManipError CCopyBeforeManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CCopyBeforeManipulator*) this);

  if (! _to) {                        // Error, illegal token.
    error.errorno (error.BAD_ARG);
    return error;
  }

  error.errorno (Manipulator::valid (_from_from, _from_to, mode));
  return error;
}

void CCopyBeforeManipulator::manipulate () {
  assert (_from_from);
  assert (_to);

  Unit buffer; 
  
  CopyManipulator copy (&buffer, _from_from, _from_to);
  copy.manipulate ();
  
  PasteBeforeManipulator paste_before (&buffer, _to);
  paste_before.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `move' manipulator. ///////////////////////////
//////////////////////////////////////////////////////////////////////


void CMoveManipulator::args (Array<Token*> &arguments) {
  _from_from = arguments[0];
  _from_to   = arguments[1];
  _to        = arguments[2];
  _arguments[0] = _from_from;
  _arguments[1] = _from_to;
  _arguments[2] = _to;
}

CMoveManipulator::CMoveManipulator (CTree *from, CTree *to) { 
  _from_from = from ? from->token () : (Token*) 0; 
  _from_to   = from ? from->end_token () : (Token*) 0;
  _to        = to ? to->end_token () : (Token*) 0; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

CMoveManipulator::CMoveManipulator (Token *from, Token *to, Token *at) {  
  _from_from = from; 
  _from_to   = to; 
  _to        = at; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

ManipError CMoveManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CMoveManipulator*) this);

  if (! _to) {                        // Error, illegal token.
    error.errorno (error.BAD_ARG);
    return error;
  }

  error.errorno (Manipulator::valid (_from_from, _from_to, mode));
  return error;
}

void CMoveManipulator::manipulate () {
  assert (_from_from);
  assert (_to);

  Unit buffer; 

  CutManipulator cut (&buffer, _from_from, _from_to);
  cut.manipulate ();
  
  MoveManipulator move (&buffer, _to);
  move.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `move_before' manipulator. ////////////////////
//////////////////////////////////////////////////////////////////////


void CMoveBeforeManipulator::args (Array<Token*> &arguments) {
  _from_from = arguments[0];
  _from_to   = arguments[1];
  _to        = arguments[2];
  _arguments[0] = _from_from;
  _arguments[1] = _from_to;
  _arguments[2] = _to;
}

CMoveBeforeManipulator::CMoveBeforeManipulator (CTree *from, CTree *to) { 
  _from_from = from ? from->token () : (Token*) 0; 
  _from_to   = from ? from->end_token () : (Token*) 0;
  _to        = to ? to->token () : (Token*) 0; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

CMoveBeforeManipulator::CMoveBeforeManipulator (Token *from, Token *to, Token *at) {  
  _from_from = from; 
  _from_to   = to; 
  _to        = at; 
  _arguments.append (_from_from);
  _arguments.append (_from_to);
  _arguments.append (_to);
}

ManipError CMoveBeforeManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CMoveBeforeManipulator*) this);

  if (! _to) {                        // Error, illegal token.
    error.errorno (error.BAD_ARG);
    return error;
  }

  error.errorno (Manipulator::valid (_from_from, _from_to, mode));
  return error;
}

void CMoveBeforeManipulator::manipulate () {
  assert (_from_from);
  assert (_to);

  Unit buffer; 
  
  CutManipulator cut (&buffer, _from_from, _from_to);
  cut.manipulate ();
  
  MoveBeforeManipulator move_before (&buffer, _to);
  move_before.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `replace' manipulator. ////////////////////////
//////////////////////////////////////////////////////////////////////


void CReplaceManipulator::args (Array<Token*> &arguments) {
  _what_from = arguments[0];
  _what_to   = arguments[1];
  _with_from = arguments[2];
  _with_to   = arguments[3];
  _arguments[0] = _what_from;
  _arguments[1] = _what_to;
  _arguments[2] = _with_from;
  _arguments[3] = _with_to;
  assert (_with_from);
  assert (_with_from->belonging_to ());
  assert (_what_from);
  assert (_what_from->belonging_to ());
  assert (_what_to);
  assert (_what_to->belonging_to ());
}

CReplaceManipulator::CReplaceManipulator (CTree *what, CTree *with) { 
  _what_from = what ? what->token () : (Token*) 0; 
  _what_to   = what ? what->end_token () : (Token*) 0;
  _with_from = with ? with->token () : (Token*) 0; 
  _with_to   = with ? with->end_token () : (Token*) 0; 
  _arguments.append (_what_from);
  _arguments.append (_what_to);
  _arguments.append (_with_from);
  _arguments.append (_with_to);
  assert (_with_from);
  assert (_with_from->belonging_to ());
  assert (_what_from);
  assert (_what_from->belonging_to ());
  assert (_what_to);
  assert (_what_to->belonging_to ());
}

CReplaceManipulator::CReplaceManipulator (Token *wf, Token *wt, Token *tf, Token *tt) { 
  _what_from = wf; 
  _what_to   = wt; 
  _with_from = tf;
  _with_to   = tt;
  _arguments.append (_what_from);
  _arguments.append (_what_to);
  _arguments.append (_with_from);
  _arguments.append (_with_to);
  assert (_with_from);
  assert (_with_from->belonging_to ());
  assert (_with_to);
  assert (_with_to->belonging_to ());
  assert (_what_from);
  assert (_what_from->belonging_to ());
  assert (_what_to);
  assert (_what_to->belonging_to ());
}

ManipError CReplaceManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CReplaceManipulator*) this);
        
  error.errorno (Manipulator::valid (_what_from, _what_to, mode));
  if (error.errorno () == ManipError::OK)
    error.errorno (Manipulator::valid (_with_from, _with_to, mode));
  return error;
}

void CReplaceManipulator::manipulate () {
  assert (_with_from);
  assert (_what_from);
  assert (_what_to);
  
  Unit buffer; 
  List *l = _what_to->belonging_to ();
  assert (l);
  if (! l) l = &buffer;
  Token *new_to = (Token*) l->next (_what_to);
  
  CopyManipulator copy (&buffer, _with_from, _with_to);
  copy.manipulate ();
  
  PasteManipulator paste (&buffer, _what_to);
  paste.manipulate ();
  
  _arguments[4] = (Token*) l->next (_what_to);

  if (! new_to) {
    for (new_to = _what_to; l->next (new_to); 
      new_to = (Token*) l->next (new_to));
    if (new_to == _what_to) 
      _arguments[5] = (Token*) 0;
    else
      _arguments[5] = new_to;
  } else
    _arguments[5] = (Token*) l->prev (new_to);

  KillManipulator kill (_what_from, _what_to);
  kill.manipulate ();
}


//////////////////////////////////////////////////////////////////////
// The C++ syntax tree `swap' manipulator. ///////////////////////////
//////////////////////////////////////////////////////////////////////


void CSwapManipulator::args (Array<Token*> &arguments) {
  _what_from = arguments[0];
  _what_to   = arguments[1];
  _with_from = arguments[2];
  _with_to   = arguments[3];
  _arguments[0] = _what_from;
  _arguments[1] = _what_to;
  _arguments[2] = _with_from;
  _arguments[3] = _with_to;
}

CSwapManipulator::CSwapManipulator (CTree *what, CTree *with) { 
  _what_from = what ? what->token () : (Token*) 0; 
  _what_to   = what ? what->end_token () : (Token*) 0;
  _with_from = with ? with->token () : (Token*) 0; 
  _with_to   = with ? with->end_token () : (Token*) 0; 
  _arguments.append (_what_from);
  _arguments.append (_what_to);
  _arguments.append (_with_from);
  _arguments.append (_with_to);
}

CSwapManipulator::CSwapManipulator (Token *wf, Token *wt, Token *tf, Token *tt) { 
  _what_from = wf; 
  _what_to   = wt; 
  _with_from = tf;
  _with_to   = tt;
  _arguments.append (_what_from);
  _arguments.append (_what_to);
  _arguments.append (_with_from);
  _arguments.append (_with_to);
}

ManipError CSwapManipulator::valid (ManipMode mode) const {
  ManipError error;
  error.faulty ((CSwapManipulator*) this);
        
  error.errorno (Manipulator::valid (_what_from, _what_to, mode));
  if (error.errorno () == ManipError::OK)
    error.errorno (Manipulator::valid (_with_from, _with_to, mode));
  return error;
}

void CSwapManipulator::manipulate () {
  assert (_with_from);
  assert (_with_to);
  assert (_what_from);
  assert (_what_to);
  
  Unit buffer; 
  List *l = _what_to->belonging_to ();
  if (! l) l = &buffer;
  Token *new_to = (Token*) l->next (_what_to);
  
  CopyManipulator copy (&buffer, _with_from, _with_to);
  copy.manipulate ();
  
  PasteManipulator paste (&buffer, _what_to);
  paste.manipulate ();

  _arguments[4] = (Token*) l->next (_what_to);

  if (! new_to) {
    for (new_to = _what_to; l->next (new_to); 
      new_to = (Token*) l->next (new_to));
    if (new_to == _what_to) 
      _arguments[5] = (Token*) 0;
    else
     _arguments[5] = new_to;
  } else
    _arguments[5] = (Token*) l->prev (new_to);

  l = _with_to->belonging_to ();
  if (! l) l = &buffer;
  new_to = (Token*) l->next (_with_to);
  
  buffer.clear ();
  copy.manipulate (&buffer, _what_from, _what_to);
  paste.manipulate (&buffer, _with_to);
  
  _arguments[6] = (Token*) l->next (_with_to);

  if (! new_to) {
    for (new_to = _with_to; l->next (new_to); 
      new_to = (Token*) l->next (new_to));
    if (new_to == _with_to) 
      _arguments[7] = (Token*) 0;
    else
      _arguments[7] = new_to;
  } else
    _arguments[7] = (Token*) l->prev (new_to);

  KillManipulator kill (_what_from, _what_to);
  kill.manipulate ();
  kill.manipulate (_with_from, _with_to);
}


} // namespace Puma
