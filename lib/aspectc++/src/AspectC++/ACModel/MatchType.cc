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

#include "MatchTypeInfos.h"

// MatchType default constructor
MatchTypeRef::MatchTypeRef () { _type_info = new MTUndefined; }

// functions used to instantiate built-in primitive types
MatchTypeRef MatchTypeRef::make_any () { return new MTAny; }
MatchTypeRef MatchTypeRef::make_void () { return new MTVoid; }
MatchTypeRef MatchTypeRef::make_bool () { return new MTBool; }
MatchTypeRef MatchTypeRef::make_signed_char () { return new MTSignedChar; }
MatchTypeRef MatchTypeRef::make_unsigned_char () { return new MTUnsignedChar; }
MatchTypeRef MatchTypeRef::make_char () { return new MTChar; }
MatchTypeRef MatchTypeRef::make_wchar_t () { return new MTWCharT; }
MatchTypeRef MatchTypeRef::make_unsigned_short () { return new MTUnsignedShort; }
MatchTypeRef MatchTypeRef::make_short () { return new MTShort; }
MatchTypeRef MatchTypeRef::make_unsigned_int () { return new MTUnsignedInt; }
MatchTypeRef MatchTypeRef::make_int () { return new MTInt; }
MatchTypeRef MatchTypeRef::make_unsigned_long () { return new MTUnsignedLong; }
MatchTypeRef MatchTypeRef::make_long () { return new MTLong; }
MatchTypeRef MatchTypeRef::make_unsigned_long_long () { return new MTUnsignedLongLong; }
MatchTypeRef MatchTypeRef::make_long_long () { return new MTLongLong; }
MatchTypeRef MatchTypeRef::make_unsigned_int128 () { return new MTUnsignedInt128; }
MatchTypeRef MatchTypeRef::make_int128 () { return new MTInt128; }
MatchTypeRef MatchTypeRef::make_float () { return new MTFloat; }
MatchTypeRef MatchTypeRef::make_double () { return new MTDouble; }
MatchTypeRef MatchTypeRef::make_long_double () { return new MTLongDouble; }

// function used to instantiate a new named type
MatchTypeRef MatchTypeRef::make_named (const MatchName &mn) {
  return new MTNamed (mn);
}

// functions to construct complex types from built-in or named types
void MatchTypeRef::to_pointer () {
  _type_info = new MTPointer (_type_info);
}

void MatchTypeRef::to_memb_pointer (const MatchName &mn) {
  _type_info = new MTMembPointer (mn, _type_info);
}

void MatchTypeRef::to_reference () {
  _type_info = new MTReference (_type_info);
}

void MatchTypeRef::to_function (const vector<MatchTypeRef> &args, bool va) {
  _type_info = new MTFunction (args, va, _type_info);
}

void MatchTypeRef::to_array (unsigned long dim) {
  _type_info = new MTArray (dim, _type_info);
}

void MatchTypeRef::to_array () {
  _type_info = new MTArray (_type_info);
}

const MatchName &MatchTypeRef::name () const {
  return ((MTNamed*)_type_info)->name();
}
