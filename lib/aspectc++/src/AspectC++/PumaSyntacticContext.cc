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

#include "PumaSyntacticContext.h"

using namespace Puma;

// search the set of actually *used* arguments (needed only for constructor
// wrapper functions).
void PumaSyntacticContext::search_used_args (set<string> &args, CTree *tree) const {
  if (!tree) {
    assert (_obj->FunctionInfo() && _obj->Tree()->NodeName() == CT_FctDef::NodeId ());
    tree = ((CT_FctDef*)_obj->Tree ())->Body();
  }

  CObjectInfo *obj;
  const char *nodename = tree->NodeName ();

  if (nodename == CT_SimpleName::NodeId () &&
      (obj = ((CT_SimpleName*)tree)->Object ()) &&
      obj->ArgumentInfo() &&
      obj->ArgumentInfo()->Scope () &&
      obj->ArgumentInfo()->Scope () == _obj) {
    args.insert (obj->Name ().c_str ());
  }

  for (int s = 0; s < tree->Sons (); s++)
    if (tree->Son(s))
      search_used_args (args, tree->Son (s));
}

void PumaSyntacticContext::print_tree (ostream &out, CTree *node, bool expand_implicit_calls) {

  if (expand_implicit_calls && node->NodeName () == CT_ImplicitCall::NodeId() &&
      !((CT_ImplicitCall*)node)->Object()->FunctionInfo()->isConstructor()) {
    out << "(";
    print_tree (out, node->Son (0), expand_implicit_calls);
    out << ")." << ((CT_ImplicitCall*)node)->Object()->Name().c_str () << "()";
  }
  else if (node->NodeName () == CT_Token::NodeId ())
    out << node->token ()->text () << " ";
  else
    for (int s = 0; s < node->Sons (); s++)
      print_tree (out, node->Son (s), expand_implicit_calls);
}

int PumaSyntacticContext::is_in_extern_c_block () const {
  Puma::CTree *tree = _obj->Tree ();
  assert (tree);
  int depth = 0;
  while (tree->NodeName () != Puma::CT_Program::NodeId ()) {
    if (tree->NodeName() == Puma::CT_LinkageSpec::NodeId () &&
        ((Puma::CT_LinkageSpec*)tree)->isList())
      depth++;
    tree = tree->Parent();
  }
  return depth;
}
