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

#ifndef __pre_macro_manager__
#define __pre_macro_manager__

// The macro manager manages all defined macros.

#include "Puma/Chain.h"
#include "Puma/Location.h"
#include "Puma/StrHashTable.h"
#include "Puma/PreMacro.h"
#include <iostream>
#include <map>


namespace Puma {


class ErrorStream;

class PreMacroManager {
  PreMacro *_Line;
  PreMacro *_Date;
  PreMacro *_Time;
  PreMacro *_IncLevel;
  PreMacro *_File;
  PreMacro *_BaseFile;
  ErrorStream *_err;

  /** Map type for macro name to macro object mapping. */
  typedef std::map<const char*,PreMacro*> MacrosByName;
  MacrosByName _Macros;

private:
  void add (PreMacro *);
  void kill (const DString &);
  PreMacro *get (const DString &) const;
  void clear ();

public:
  PreMacroManager (ErrorStream *err) : _err (err) {}

  // Destroy the predicates too.
  ~PreMacroManager () { clear (); }

  // Initialize the macro manager.
  void init (const char *filename);

  // Add given macro to macro table.
  void addMacro (PreMacro *macro);

  // Remove macro `name' from macro table.
  void removeMacro (const DString &name, const Location& location = Location());

  // Get macro `name' from macro table.
  PreMacro *getMacro (const DString &name) const;

  // Get pointers to some special macros
  PreMacro *getLineMacro () const { return _Line; }
  PreMacro *getDateMacro () const { return _Date; }
  PreMacro *getTimeMacro () const { return _Time; }
  PreMacro *getIncLevelMacro () const { return _IncLevel; }
  PreMacro *getFileMacro () const { return _File; }
  PreMacro *getBaseFileMacro () const { return _BaseFile; }
};


} // namespace Puma

#endif /* __pre_macro_manager__ */
