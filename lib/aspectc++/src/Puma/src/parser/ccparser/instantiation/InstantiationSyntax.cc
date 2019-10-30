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

#include "Puma/InstantiationSyntax.h"
#include "Puma/InstantiationSemantic.h"
#include "Puma/InstantiationBuilder.h"
#include "Puma/InstantiationTokenProvider.h"
#include "Puma/InstantiationTokenSource.h"
#include "Puma/CTokens.h"
#include "Puma/CTree.h"

namespace Puma {


InstantiationSyntax::InstantiationSyntax(InstantiationBuilder &b, InstantiationSemantic &s) : CCSyntax(b, s) {
}

InstantiationBuilder &InstantiationSyntax::builder() const {
  return (InstantiationBuilder&) Syntax::builder();
}

InstantiationSemantic &InstantiationSyntax::semantic() const {
  return (InstantiationSemantic&) Syntax::semantic();
}

InstantiationTokenSource &InstantiationSyntax::source() const {
  return (InstantiationTokenSource&)((InstantiationTokenProvider*)token_provider)->source();
}

InstantiationTokenProvider &InstantiationSyntax::provider() const {
  return (InstantiationTokenProvider&) *token_provider;
}

void InstantiationSyntax::configure(Config &config) {
  // configure the CC syntax
  CCSyntax::configure(config);

  // instance function bodies or not
  config_skip_fct_body(!config.Option("--inst-fct-bodies"));
}

bool InstantiationSyntax::isRoot(int up) const {
//  CTree* node = provider().node();
  CTree* node = (CTree*)provider().current_context();
  for (int i = 0; i < up && node; i++)
    node = node->Parent();
  return node == source().root();
}

bool InstantiationSyntax::type_param_without_init() {
  return parse(&InstantiationSyntax::rule_type_param_without_init);
}

CTree *InstantiationSyntax::rule_type_param_without_init() {
  // 1: CLASS
  // 1: TYPENAME
  // 2: CLASS  identifier
  // 2: TYPENAME  identifier
  // 5: TEMPLATE  <  template_param_list  >  CLASS
  // 6: TEMPLATE  <  template_param_list  >  CLASS  identifier
  return ((parse(TOK_CLASS) ||
           parse(TOK_TYPENAME) ||
           (parse(TOK_TEMPLATE) &&
            parse(TOK_LESS) && template_param_list() && parse(TOK_GREATER) &&
            parse(TOK_CLASS))) &&
          (Identifier::check(*this) || PrivateName::check(*this))) ?
    builder().type_param() : (CTree*)0;
}

bool InstantiationSyntax::non_type_param_without_init() {
  return parse(&InstantiationSyntax::rule_non_type_param_without_init);
}

CTree *InstantiationSyntax::rule_non_type_param_without_init() {
  // 1: decl_spec_seq
  // 2: decl_spec_seq  abst_declarator
  // 2: decl_spec_seq  declarator
  if (!(DeclSpecSeq::check(*this) &&
        (Declarator::check(*this) ||
         abst_declarator() ||
         PrivateName::check(*this)))) {
    semantic().finish_declarator();
    return (CTree*)0;
  }
  return semantic().non_type_param_without_init();
}

bool InstantiationSyntax::class_name() {
  // 1: template_id
  // 1: ID
  if (!semantic().is_class_name())
    return false;
  if (!is<CT_TemplateName>(2) && semantic().is_this_instance()) {
    if (CCSyntax::identifier()) {
      semantic().set_this_instance();
      return true;
    }
  }
  return CCSyntax::class_name();
}

bool InstantiationSyntax::enum_name() {
  // 1: ID
  if (!semantic().is_enum_name())
    return false;
  return CCSyntax::enum_name();
}

bool InstantiationSyntax::typedef_name() {
  // 1: ID
  if (!semantic().is_typedef_name())
    return false;
  return CCSyntax::typedef_name();
}

bool InstantiationSyntax::template_name() {
  // 1: ID
  if (!semantic().is_template_name())
    return false;
  return CCSyntax::template_name();
}

bool InstantiationSyntax::class_template_name() {
  // 1: ID
  if (!semantic().is_class_template_name())
    return false;
  return CCSyntax::class_template_name();
}

bool InstantiationSyntax::namespace_name() {
  // 1: ID
  if (!semantic().is_namespace_name())
    return false;
  return CCSyntax::namespace_name();
}

bool InstantiationSyntax::template_id() {
  // 3: template_name  <  >
  // 4: template_name  <  template_arg_list  >
  if (!is<CT_TemplateName>(2))
    return false;
  if (semantic().is_this_specialization()) {
    if (CCSyntax::identifier()) {
      skip_block(TOK_LESS, TOK_GREATER);
      return true;
    }
  }
  return CCSyntax::template_id();
}


bool InstantiationSyntax::class_template_id() {
  // 3: template_name  <  >
  // 4: template_name  <  template_arg_list  >
  if (!is<CT_TemplateName>(2))
    return false;
  if (semantic().is_this_specialization()) {
    if (CCSyntax::identifier()) {
      skip_block(TOK_LESS, TOK_GREATER);
      return true;
    }
  }
  return CCSyntax::class_template_id();
}


} // namespace Puma
