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

#ifndef __InstantiationParser__
#define __InstantiationParser__

#include "Puma/Parser.h"
#include "Puma/InstantiationSyntax.h"
#include "Puma/InstantiationBuilder.h"
#include "Puma/InstantiationSemantic.h"

namespace Puma {


class CTree;
class CObjectInfo;
class CTemplateInfo;
class CStructure;
class CCInstantiation;

class InstantiationParser : public Parser {
  InstantiationSyntax _syntax;
  InstantiationBuilder _builder;
  InstantiationSemantic _semantic;

public:
  InstantiationParser() :
    Parser(_syntax, _builder, _semantic),
    _syntax(_builder, _semantic),
    _semantic(_syntax, _builder) {
  }

  // initialize the parser
  void init(CObjectInfo* obj, CTemplateInfo* tpl, CStructure* instance_scope, CCInstantiation* inst = 0);

  // parse a template instance or template argument
  CTree *parse(CTemplateInfo* tpl, CTree* tree, bool (InstantiationSyntax::*rule)(), bool &delayed_parse_node, bool add_tree = true);

  // do semantic expression analysis
  void resolve(CTemplateInfo* tpl, CStructure* scope, CTree* tree);

  // check for parse errors
  bool failed();

  InstantiationSyntax &syntax() const { return (InstantiationSyntax&) _syntax; }
  InstantiationBuilder &builder() const { return (InstantiationBuilder&) _builder; }
  InstantiationSemantic &semantic() const { return (InstantiationSemantic&) _semantic; }
};


} // namespace Puma

#endif /* __InstantiationParser__ */
