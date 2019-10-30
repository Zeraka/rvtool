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

#include "Puma/InstantiationParser.h"
#include "Puma/CTree.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CStructure.h"
#include "Puma/CFileInfo.h"
#include "Puma/CCInstantiation.h"
#include "Puma/InstantiationTokenProvider.h"
#include "Puma/InstantiationTokenSource.h"
#include "Puma/CCSemVisitor.h"
#include "Puma/CPrintVisitor.h"

namespace Puma {


extern int TRACE_INSTANCE_CODE;
extern int TRACE_PARSE_INSTANCE;



// initialize the parser
void InstantiationParser::init(CObjectInfo* obj, CTemplateInfo* tpl, CStructure* instance_scope, CCInstantiation* inst) {
  // configure the parser
  configure(obj->SemDB()->Project()->config());

  // initialize semantic analyzer
  bool is_fct = obj->FunctionInfo();
  Unit* primary = obj->SourceInfo()->FileInfo()->Primary();
  semantic().init(*obj->SemDB(), *primary, instance_scope, is_fct, !is_fct, inst);
  ((ErrorCollector&) builder().err()).index(0);
  semantic().error_sink(builder().err());

  if (TRACE_PARSE_INSTANCE) {
#ifdef __PUMA_TRACING__
    trace(std::cout);
#endif
  }
}


// parse a template instance
CTree *InstantiationParser::parse(CTemplateInfo* tpl, CTree* tree, bool (InstantiationSyntax::*rule)(), bool &delayed_parse_node, bool add_tree) {
  if (TRACE_PARSE_INSTANCE && semantic().Instantiation())
    semantic().Instantiation()->printInstantiationHeader();

  // increase instantiation depth
  tpl->increaseDepth();

  // parse the template or template argument
  InstantiationTokenSource source(tree);
  InstantiationTokenProvider provider(source);
  tree = syntax().run(provider, rule);
  if (add_tree)
    tpl->addTreeFragment(tree);

  // decrease instantiation depth
  tpl->decreaseDepth ();

  if (!tree && !failed()) {
    // failed with no error message
    builder().err() << sev_error;
    if (provider.current())
      builder().err() << provider.current()->location();
    builder().err() << "invalid declaration";
    if (provider.current())
      builder().err() << " near token `" << provider.current()->text() << "'";
    builder().err() << endMessage;
  }

  // if a delayed parse node was part of the syntax tree,
  // this information has to be returned to the called here.
  // It might be necessary to issue an error message later.
  delayed_parse_node = source.detectedDelayedParseNode ();

  if (tree && TRACE_INSTANCE_CODE) {
    if (semantic().Instantiation())
      semantic().Instantiation()->printInstantiationHeader();
    // dump tree
    CPrintVisitor printer;
    printer.configure(tpl->SemDB()->Project()->config());
    printer.print(tree, std::cout);
  }

  return tree;
}


// resolve expressions
void InstantiationParser::resolve(CTemplateInfo* tpl, CStructure* scope, CTree* tree) {
  // increase instantiation depth
  tpl->increaseDepth();

  // do semantic expression analysis
  CCSemVisitor resolver(builder().err());
  resolver.run(tree, scope);

  // decrease instantiation depth
  tpl->decreaseDepth ();
}


// check for parse errors
bool InstantiationParser::failed() {
  return ((ErrorCollector&)builder().err()).severity() > sev_warning;
}


} // namespace Puma
