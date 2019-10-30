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

#ifndef __InstantiationSemantic_h__
#define __InstantiationSemantic_h__

#include "Puma/CCSemantic.h"
#include "Puma/InstantiationBuilder.h"
#include "Puma/InstantiationSyntax.h"
#include "Puma/CTree.h"

namespace Puma {


class InstantiationSemantic : public CCSemantic {
  InstantiationSyntax &_syntax;

public:
  InstantiationSemantic(InstantiationSyntax &, InstantiationBuilder &);

protected:
  virtual InstantiationBuilder &builder() const;
  virtual InstantiationSyntax &syntax() const;

public:
  void parse_delayed(bool (CCSyntax::*rule)(), CTree *tree);

  CTree *non_type_param_without_init();
  CTree *introduce_function();
  CTree *introduce_class();
  CTree *introduce_object(CTree* = 0);

  bool is_class_name();
  bool is_enum_name();
  bool is_typedef_name();
  bool is_template_name();
  bool is_class_template_name();
  bool is_namespace_name();

  bool is_this_specialization();
  bool is_this_instance();
  void set_this_instance();

private:
  CTree* getParent(CTree* node, int n = 1);

  template<class NODE>
  bool is(CTree* node) const;
};

inline InstantiationSemantic::InstantiationSemantic(InstantiationSyntax &s, InstantiationBuilder &b) :
  CCSemantic(s, b), _syntax(s) {}

inline InstantiationBuilder &InstantiationSemantic::builder() const
 { return (InstantiationBuilder&) CCSemantic::builder(); }

inline InstantiationSyntax &InstantiationSemantic::syntax() const
 { return (InstantiationSyntax&) _syntax; }

template<class NODE>
inline bool InstantiationSemantic::is(CTree* node) const
 { return node && node->NodeName() == NODE::NodeId(); }


} // namespace Puma

#endif /* __InstantiationSemantic_h__ */
