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

#include <Puma/CTree.h>
#include <Puma/CUnit.h>
#include <Puma/ManipCommander.h>
#include "Transformer.h"


Transformer::Transformer(Puma::ErrorStream& err) : m_Err(err) {
}


void Transformer::transform(Puma::CTree* node) {
  // Visit the syntax tree top-down.
  visit(node);
}


// Overwrite pre_visit(...) from Puma::CVisitor
// that is called before a subtree is visited.
void Transformer::pre_visit(Puma::CTree* node) {
  // Match if and if-else statements.
  if (node->NodeName() == Puma::CT_IfStmt::NodeId() ||
      node->NodeName() == Puma::CT_IfElseStmt::NodeId()) {
    // Insert the command before the statement.
    insertComment(node);
  }
}


// Overwrite pre_visit(...) from Puma::CVisitor
// that is called before a subtree is visited.
void Transformer::insertComment(Puma::CTree* node) {
  // Get the first token of the statement, i.e. the 'if'.
  Puma::Token* token_if = node->token();

  // Scan the commant tokens.
  Puma::CUnit comment(m_Err);
  comment.name(token_if->location().filename().name());
  comment << "/* " << token_if->location() << " */ " << Puma::endu;

  // Insert the comment tokens before the 'if' token.
  Puma::ManipCommander mc;
  mc.paste_before(token_if, &comment);
  mc.commit();
}
