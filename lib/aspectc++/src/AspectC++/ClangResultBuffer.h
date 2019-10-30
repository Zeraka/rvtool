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

#ifndef __ClangResultBuffer_h__
#define __ClangResultBuffer_h__

#include "ACModel/Elements.h"
#include "TransformInfo.h"
#include "ACProject.h"
#include "ACModel/Utils.h"

#include <string>
using std::string;

namespace Puma {
  class CTypeInfo;
}

class ClangResultBuffer {
  clang::QualType _type;
  ACProject &_project;
  clang::ASTContext& _ast_ctx;
  bool _problem;
  bool _use_typedef;
  bool _const_result;
  bool has_constructor_problem () const;
  bool workaround_const_result() const { return _const_result; }
public:
  ClangResultBuffer (ACProject &project, ACM_Code *jpl, bool use_typedef = false) :
    _project (project),
    _ast_ctx(project.get_compiler_instance ()->getASTContext ()),
    _use_typedef( use_typedef ), _const_result( false ) {
    //_policy.SuppressUnwrittenScope = true; // Suppress printing of <anonymous>
    // builtin constructors/destructors have no decl -> result type is 'void'
    if ( jpl->type_val() == JPT_Get )
      // FIXME: If the get join point accesses an array, the function
      //        get_result_type( *jpl ) returns the type of the array
      //        instead of the actual element type.
      //        As a workaround, use TI_GET::entity_type() here.
      //        Once get_result_type( *jpl ) yields the correct type,
      //        this special case can be removed.
      _type = TI_Get::of(*static_cast<ACM_Get*>(jpl))->entity_type();
    else if( has_result_type( *jpl ) )
      _type = TI_Type::of( *get_result_type( *jpl ) )->type();
    else
      _type = _ast_ctx.VoidTy;
    _problem = has_constructor_problem ();
  }
  string tjp_result_type () const;
  string result_type (const string &name, bool unqual = false) const;
  string result_declaration() const;
  string result_assignment(const string &result) const;
  string action_result_assignment(const string &result) const;
  string result_return() const;

  void has_const_result() { _const_result = true; }
  
  // the name of the result buffer allocated in each wrapper function.
  // must not shadow the functions' arguments
  static const string result_name() { return "__result_buffer"; }
};

#endif // __ClangResultBuffer_h__
