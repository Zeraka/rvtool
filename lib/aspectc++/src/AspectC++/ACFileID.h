// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2013  The 'ac++' developers (see aspectc.org)
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

#ifndef __ACFileID_h__
#define __ACFileID_h__

#include <string>

#ifdef FRONTEND_PUMA
#include "Puma/Unit.h"
typedef const Puma::Unit *ACFileIDData;
#else

#include "clang/Basic/FileManager.h"
typedef const clang::FileEntry *ACFileIDData;
#endif

// A wrapper class that only allows to take the filename from a unit.
class ACFileID {

  ACFileIDData _entry;

public:

  ACFileID () : _entry(0) {}
  ACFileID (ACFileIDData entry) : _entry (entry) {}

  std::string name () const {
#ifdef FRONTEND_PUMA
    return _entry->name ();
#else
    return _entry->getName ();
#endif
  }
  bool is_valid () const { return _entry != 0; }
  bool operator== (ACFileID rhs) const { return _entry == rhs._entry; }
  bool operator!= (ACFileID rhs) const { return !operator==(rhs); }
  bool operator< (ACFileID rhs) const { return _entry < rhs._entry; }
  ACFileIDData file_entry() const { return _entry; }
};

#endif // __ACFileID_h__
