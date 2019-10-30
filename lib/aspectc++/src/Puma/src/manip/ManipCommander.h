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

#ifndef __ManipCommander__
#define __ManipCommander__

/** \file
 *  Token chain manipulation. */

/** \page transformation Transformation
 */

#include <list>
#include <set>

#include "Puma/ManipController.h"
#include "Puma/Manipulators.h"

namespace Puma {

class MacroUnit;

class ManipCommander {
  ManipulatorSequence _commands;
  std::list<Unit*>         _buffers;
  ManipIgnoreMask     _ignore;
  std::set<MacroUnit*>     _expanded_macro_units;
  ManipMode           _manip_mode;

public:
  ManipCommander () : _ignore (MIM_NONE), _manip_mode (MM_NO_MACRO_MANIP) {}
  ~ManipCommander ();
                
  Unit *newBuffer ();
  void addBuffer (Unit *);

  ManipError valid () const;
  void commit ();
  void commit (ManipController &);
  void add (Manipulator *);
  void clear ();

  // set/get the ignore mask
  void ignore_mask (ManipIgnoreMask mim) { _ignore = mim; }
  ManipIgnoreMask ignore_mask () const { return _ignore; }
        
  // set/get the manipulation mode
  void manip_mode (ManipMode mm) { _manip_mode = mm; }
  ManipMode manip_mode () const { return _manip_mode; }

  // Low level manipulators using named intern buffers 
  // (or a no-name shared buffer). Some may use extern buffers too.
  void cut (Unit *buffer, Token *from, Token *to = (Token*) 0);
  void copy (Unit *buffer, Token *from, Token *to = (Token*) 0);
  void kill (Token *from, Token *to = (Token*) 0);
  void paste (Token *behind, Unit*);
  void paste_before (Token *before, Unit*);

  // Composed manipulators.
  void paste (Token *behind, Token *which);
  void paste_before (Token *before, Token *which);
  void replace (Token *which, Token *with);
  void copy (Token *from, Token *to, Token *behind);
  void move (Token *from, Token *to, Token *behind);
  void copy_before (Token *from, Token *to, Token *before);
  void move_before (Token *from, Token *to, Token *before);
  void replace (Token *from1, Token *to1, Token *from2, Token *to2);
  void swap (Token *from1, Token *to1, Token *from2, Token *to2);
        
  // Special C++ syntax tree manipulator interfaces.
  void kill (CTree *what);
  void copy (CTree *what, CTree *behind);
  void move (CTree *what, CTree *behind);
  void copy_before (CTree *what, CTree *before);
  void move_before (CTree *what, CTree *before);
  void replace (CTree *what, CTree *with);
  void swap (CTree *what, CTree *with);        

private:
  Token *getStartToken (Token *);
  Token *getEndToken (Token *);
  void auto_expand_macro (MacroUnit *macro);
};


} // namespace Puma

#endif /* __ManipCommander__ */
