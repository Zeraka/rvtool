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

#ifndef __puma_manipulator_classes__
#define __puma_manipulator_classes__

// Puma manipulator classes.
//
// ManipError..................Error reporting class for manipulators.
// Manipulator.................Abstract base class for manipulators.
// ManipulatorSequence.........A sequence of manipulator objects.
//
// CopyManipulator.............Copy one or more tokens into buffer.
// CutManipulator..............Remove one or more tokens and copy in buffer.
// KillManipulator.............Remove one or more tokens permanently.
// PasteManipulator............Insert a list of tokens at given position.
// PasteBeforeManipulator......Insert a list of tokens before given position.
// MoveManipulator.............Shift a list of tokens at given position.
// MoveBeforeManipulator.......Shift a list of tokens before given position.
//
// CKillManipulator..........Remove all tokens of a tree permanently.
// CCopyManipulator..........Copy a tree after the last token of a 2nd tree.
// CCopyBeforeManipulator....Copy a tree before the first token of a 2nd tree.
// CMoveManipulator..........Move a tree after the last token of a 2nd tree.
// CMoveBeforeManipulator....Move a tree before the first token of a 2nd tree.
// CReplaceManipulator.......Replace a tree with an other tree.
// CSwapManipulator..........Swap two trees.

#include "Puma/Unit.h"
#include "Puma/List.h"
#include "Puma/CTree.h"
#include "Puma/Array.h"
#include "Puma/Token.h"
#include "Puma/Printable.h"
#include "Puma/RuleTable.h"

namespace Puma {


// Manipulator error structure. 

class Manipulator;

class ManipError : public Printable {
  int                 _errorno;        // The error number. See below.
  Manipulator *_faulty;                // The faulty manipulator.

public:
  enum {
    UNBALANCED = -3,        // Unbalanced preprocessor directives found.
    MACRO_GEN,                // Wanted to manipulate macro generated tokens.
    BAD_ARG,                // Bad arguments has been given to manipulator.
    FATAL,                // Fatal errors, like NULL pointer buffer.
    OK                        // Everything seems to be correct.
  };
    
  ManipError () : _faulty(0) {}

  // Get ...
  int                 errorno () const  { return _errorno; }
  Manipulator *faulty ()  const  { return _faulty; }
  // An error is true if there really was an error.
  operator bool () const         { return _errorno != OK; }
        
  // Set ...
  void errorno  (int number)       { _errorno = number; }
  void faulty (Manipulator *manip) { _faulty  = manip; }

  // Print ...
  void print (std::ostream &os) const;

  // There has to be a first arg ...
  Location location ();
};
 

// Enum for checks which should NOT be performed on a Manipulator
enum ManipIgnoreMask { MIM_NONE = 0, MIM_UNBALANCED = 1, MIM_MACRO = 2 };


enum ManipMode {
  MM_NO_MACRO_MANIP, // Macro-generated tokens are not transformed unless they are at the end/begin of an
                     // expansion. In this case the token is replace by the macro call end/begin.
  MM_EXPAND_MACROS   // Macros are expanded on demand.
};


// Abstract base class for manipulator classes.

class Manipulator : public ListElement {

  ManipIgnoreMask _ignore;
  
protected:        
  Manipulator () : _ignore (MIM_NONE) {}

  int valid (Token *, Token *, ManipMode) const;

public:
  // Manipulator identifiers used to identify a manipulator typ.
  static const char *copy_op;
  static const char *cut_op;
  static const char *paste_op; 
  static const char *paste_before_op; 
  static const char *move_op; 
  static const char *move_before_op; 
  static const char *kill_op;
  static const char *cpp_copy_op; 
  static const char *cpp_copy_before_op; 
  static const char *cpp_kill_op; 
  static const char *cpp_move_op; 
  static const char *cpp_move_before_op;
  static const char *sequence_op;
  static const char *cpp_replace_op;
  static const char *cpp_swap_op;

  virtual ~Manipulator () {}

  virtual ManipError valid (ManipMode) const = 0;
  virtual void manipulate ()              = 0;
        
  virtual Array<Token*> &args ()     = 0;
  virtual void args (Array<Token*>&) = 0;
        
  virtual const char *op () const   = 0;
  virtual ListElement *duplicate () = 0;
  
  // set/get the ignore mask
  void ignore_mask (ManipIgnoreMask mim) { _ignore = mim; }
  ManipIgnoreMask ignore_mask () const { return _ignore; }
};
 

// Special manipulator class that manages a sequence of
// manipulator commands.

class ManipulatorSequence : public Manipulator, public List {
  RuleTable     _rules;
  Array<Token*> _empty;        // Not really used.
        
  void checkArgs (ManipError&, Manipulator*);

public:
  ManipulatorSequence () {}
        
  Array<Token*> &args () { return _empty; }
  void args (Array<Token*> &empty) {}

  const char *op () const { return sequence_op; }
  ListElement *duplicate ();

  ManipError valid (ManipMode mode) const;
  void manipulate ();
        
  void append (Manipulator *manip)
    { List::append (*manip); }
};

 
// The `copy' manipulator.

class CopyManipulator : public Manipulator {
  Token *_from;
  Token *_to;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  CopyManipulator (Unit *buffer, Token *from, Token *to = (Token*) 0);
  ~CopyManipulator () {}
                
  const char *op () const { return copy_op; }
  ListElement *duplicate () 
    { return new CopyManipulator (_buffer, _from, _to); }

  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);

  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *from, Token *to = (Token*) 0)
    { _buffer = buffer; _from = from; _to = to; manipulate (); }
};


// The `cut' manipulator.

class CutManipulator : public Manipulator {
  Token *_from;
  Token *_to;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  CutManipulator (Unit *buffer, Token *from, Token *to = (Token*) 0);
  ~CutManipulator () {}

  const char *op () const { return cut_op; }
  ListElement *duplicate () 
    { return new CutManipulator (_buffer, _from, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *from, Token *to = (Token*) 0)
    { _buffer = buffer; _from = from; _to = to; manipulate (); }
};


// The `kill' manipulator.

class KillManipulator : public Manipulator {
  Token *_from;
  Token *_to;
  Array<Token*> _arguments;
        
public:
  KillManipulator (Token *from, Token *to = (Token*) 0);
  ~KillManipulator () {}

  const char *op () const { return kill_op; }
  ListElement *duplicate () 
    { return new KillManipulator (_from, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Token *from, Token *to = (Token*) 0)
    { _from = from; _to = to; manipulate (); }
};


// The `paste' manipulator.

class PasteManipulator : public Manipulator {
  Token *_at;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  PasteManipulator (Unit *buffer, Token *at);
  ~PasteManipulator () {}

  const char *op () const { return paste_op; }
  ListElement *duplicate () 
    { return new PasteManipulator (_buffer, _at); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *at)
    { _buffer = buffer; _at = at; manipulate (); }
};


// The `paste_before' manipulator.

class PasteBeforeManipulator : public Manipulator {
  Token *_at;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  PasteBeforeManipulator (Unit *buffer, Token *at);
  ~PasteBeforeManipulator () {}
                
  const char *op () const { return paste_before_op; }
  ListElement *duplicate () 
    { return new PasteBeforeManipulator (_buffer, _at); }

  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *at)
    { _buffer = buffer; _at = at; manipulate (); }
};


// The `move' manipulator.

class MoveManipulator : public Manipulator {
  Token *_at;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  MoveManipulator (Unit *buffer, Token *at);
  ~MoveManipulator () {}

  const char *op () const { return move_op; }
  ListElement *duplicate () 
    { return new MoveManipulator (_buffer, _at); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *at)
    { _buffer = buffer; _at = at; manipulate (); }
};


// The `move_before' manipulator.

class MoveBeforeManipulator : public Manipulator {
  Token *_at;
  Unit * _buffer;
  Array<Token*> _arguments;
        
public:
  MoveBeforeManipulator (Unit *buffer, Token *at);
  ~MoveBeforeManipulator () {}
                
  const char *op () const { return move_before_op; }
  ListElement *duplicate () 
    { return new MoveBeforeManipulator (_buffer, _at); }

  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (Unit *buffer, Token *at)
    { _buffer = buffer; _at = at; manipulate (); }
};


// The C++ syntax tree `kill' manipulator.

class CKillManipulator : public Manipulator {
  Token *_from;
  Token *_to;
  Array<Token*> _arguments;
        
public:
  CKillManipulator (CTree *what);
  CKillManipulator (Token*, Token*);
  ~CKillManipulator () {}

  const char *op () const { return cpp_kill_op; }
  ListElement *duplicate () 
    { return new CKillManipulator (_from, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *what)
    { _from = what ? what->token () : (Token*) 0; 
      _to   = what ? what->end_token () : (Token*) 0; 
      manipulate (); }
};


// The C++ syntax tree `copy' manipulator.

class CCopyManipulator : public Manipulator {
  Token *_from_from;
  Token *_from_to;
  Token *_to;
  Array<Token*> _arguments;

public:
  CCopyManipulator (CTree *from, CTree *to);
  CCopyManipulator (Token*, Token*, Token*);
  ~CCopyManipulator () {}

  const char *op () const { return cpp_copy_op; }
  ListElement *duplicate ()
    { return new CCopyManipulator (_from_from, _from_to, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *from, CTree *to)
    { _from_from = from ? from->token () : (Token*) 0; 
      _from_to   = from ? from->end_token () : (Token*) 0;
      _to        = to ? to->end_token () : (Token*) 0;
      manipulate (); }
              
  void manipulate (Token *from, Token *to, Token *at)
    { _from_from = from; _from_to = to; _to = at; manipulate (); }
};


// The C++ syntax tree `copy_before' manipulator.

class CCopyBeforeManipulator : public Manipulator {
  Token *_from_from;
  Token *_from_to;
  Token *_to;
  Array<Token*> _arguments;
        
public:
  CCopyBeforeManipulator (CTree *from, CTree *to);
  CCopyBeforeManipulator (Token*, Token*, Token*);
  ~CCopyBeforeManipulator () {}

  const char *op () const { return cpp_copy_before_op; }
  ListElement *duplicate ()
    { return new CCopyBeforeManipulator (_from_from, _from_to, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *from, CTree *to)
    { _from_from = from ? from->token () : (Token*) 0; 
      _from_to   = from ? from->end_token () : (Token*) 0;
      _to        = to ? to->token () : (Token*) 0;
      manipulate (); }

  void manipulate (Token *from, Token *to, Token *at)
    { _from_from = from; _from_to = to; _to = at; manipulate (); }
};


// The C++ syntax tree `move' manipulator.

class CMoveManipulator : public Manipulator {
  Token *_from_from;
  Token *_from_to;
  Token *_to;
  Array<Token*> _arguments;
        
public:
  CMoveManipulator (CTree *from, CTree *to);
  CMoveManipulator (Token*, Token*, Token*);
  ~CMoveManipulator () {}

  const char *op () const { return cpp_move_op; }
  ListElement *duplicate ()
    { return new CMoveManipulator (_from_from, _from_to, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *from, CTree *to)
    { _from_from = from ? from->token () : (Token*) 0; 
      _from_to   = from ? from->end_token () : (Token*) 0;
      _to        = to ? to->end_token () : (Token*) 0;
      manipulate (); }

  void manipulate (Token *from, Token *to, Token *at)
    { _from_from = from; _from_to = to; _to = at; manipulate (); }
};


// The C++ syntax tree `move_before' manipulator.

class CMoveBeforeManipulator : public Manipulator {
  Token *_from_from;
  Token *_from_to;
  Token *_to;
  Array<Token*> _arguments;

public:
  CMoveBeforeManipulator (CTree *from, CTree *to);
  CMoveBeforeManipulator (Token*, Token*, Token*);
  ~CMoveBeforeManipulator () {}

  const char *op () const { return cpp_move_before_op; }
  ListElement *duplicate ()
    { return new CMoveBeforeManipulator (_from_from, _from_to, _to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *from, CTree *to)
    { _from_from = from ? from->token () : (Token*) 0; 
      _from_to   = from ? from->end_token () : (Token*) 0;
      _to        = to ? to->token () : (Token*) 0;
      manipulate (); }

  void manipulate (Token *from, Token *to, Token *at)
    { _from_from = from; _from_to = to; _to = at; manipulate (); } 
};


// The C++ syntax tree `replace' manipulator.

class CReplaceManipulator : public Manipulator {
  Token *_what_from;
  Token *_what_to;
  Token *_with_from;
  Token *_with_to;
  Array<Token*> _arguments;
        
public:
  CReplaceManipulator (CTree *what, CTree *with);
  CReplaceManipulator (Token*, Token*, Token*, Token*);
  ~CReplaceManipulator () {}

  const char *op () const { return cpp_replace_op; }
  ListElement *duplicate ()
    { return new CReplaceManipulator (_what_from, _what_to, 
                                                _with_from, _with_to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *what, CTree *with)
    { _what_from = what ? what->token () : (Token*) 0; 
      _what_to   = what ? what->end_token () : (Token*) 0;
      _with_from = with ? with->token () : (Token*) 0; 
      _with_to   = with ? with->end_token () : (Token*) 0;
      manipulate (); }

  void manipulate (Token *wf, Token *wt, Token *tf, Token *tt)
    { _what_from = wf; _what_to = wt; _with_from = tf;
      _with_to = tt; manipulate (); }
};


// The C++ syntax tree `swap' manipulator.

class CSwapManipulator : public Manipulator {
  Token *_what_from;
  Token *_what_to;
  Token *_with_from;
  Token *_with_to;
  Array<Token*> _arguments;
        
public:
  CSwapManipulator (CTree *what, CTree *with);
  CSwapManipulator (Token*, Token*, Token*, Token*);
  ~CSwapManipulator () {}

  const char *op () const { return cpp_swap_op; }
  ListElement *duplicate ()
    { return new CSwapManipulator (_what_from, _what_to, 
                                   _with_from, _with_to); }
                
  Array<Token*> &args () { return _arguments; }
  void args (Array<Token*>&);
                
  ManipError valid (ManipMode) const;
  void manipulate ();
        
  void manipulate (CTree *what, CTree *with)
    { _what_from = what ? what->token () : (Token*) 0; 
      _what_to   = what ? what->end_token () : (Token*) 0;
      _with_from = with ? with->token () : (Token*) 0; 
      _with_to   = with ? with->end_token () : (Token*) 0;
      manipulate (); }

  void manipulate (Token *wf, Token *wt, Token *tf, Token *tt)
    { _what_from = wf; _what_to = wt; _with_from = tf;
      _with_to = tt; manipulate (); }
};


} // namespace Puma

#endif /* __puma_manipulator_classes__ */
