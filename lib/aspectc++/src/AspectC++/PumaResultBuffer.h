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

#ifndef __PumaResultBuffer_h__
#define __PumaResultBuffer_h__

#include "ACModel/Elements.h"
#include "TransformInfo.h"
#include "ACProject.h"

#include <string>
using std::string;

namespace Puma {
  class CTypeInfo;
}

class PumaResultBuffer {
  Puma::CTypeInfo *_type;
  bool _problem;
  bool _use_typedef;
  bool has_constructor_problem () const;
public:
  PumaResultBuffer (Puma::CTypeInfo *t, bool use_typedef = false) :
    _type (t), _use_typedef (use_typedef) {
    _problem = has_constructor_problem ();
  }
  PumaResultBuffer (ACProject &project, ACM_Code *jpl, bool use_typedef = false) :
    _use_typedef (use_typedef) {
    assert( has_entity_func( *jpl ) );
    ACM_Function *func = get_entity_func( *jpl );
    Puma::CFunctionInfo *fi = TI_Function::of (*func)->func_info();
    Puma::CTypeFunction *ftype = fi->TypeInfo ()->TypeFunction ();
    _type = fi->isConversion () ? fi->ConversionType () : ftype->BaseType ();
    _problem = has_constructor_problem ();
  }
  string tjp_result_type () const;
  string result_type (const string &name, bool unqual = false) const;
  string result_declaration() const;
  string result_assignment(const string &result) const;
  string action_result_assignment(const string &result) const;
  string result_return() const;
  
  // the name of the result buffer allocated in each wrapper function.
  // must not shadow the functions' arguments
  static const string result_name() { return "__result_buffer"; }
};

#endif // __PumaResultBuffer_h__
