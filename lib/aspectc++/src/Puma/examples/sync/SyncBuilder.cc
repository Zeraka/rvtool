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

#include "SyncBuilder.h"
#include "SyncSyntax.h"
#include "SyncTokens.h"
#include "SyncTree.h"


/*****************************************************************************/
/*                                                                           */
/*                      A . 5   S t a t e m e n t s                          */
/*                                                                           */
/*****************************************************************************/


Puma::CTree *SyncBuilder::sync_stmt() {
  // This method creates a new syntax tree node resp.
  // sub-tree for the parsed "synchronized" statement
  // from the nodes and tokens laying on the builder
  // stack.
  //
  // 3: SYNCHRONIZED  {  }            => token token token
  // 4: SYNCHRONIZED  {  stmt_seq  }  => token token container token

  // Create a new "synchronized" statement node.
  SyncStmt *result = new SyncStmt;

  // Add the keyword "synchronized" to the node.
  // get_node() returns the given node from the
  // builder stack.
  result->AddSon(get_node(0));
  // Add the opening curly bracket.
  result->AddSon(get_node(1));

  // Distinguish between the two variations.
  // nodes() returns the number of nodes
  // laying on the builder stack.
  if (nodes() == 4) {
    // The statements of the parsed statement sequence are
    // collected in a generic container object, and have to
    // be added to the new node one by one.
    Container *container = (Container*)get_node(2);
    for (int i = 0; i < container->Sons(); i++) {
      result->AddSon(container->Son(i));
    }
    // The container is not needed anymore, so delete it.
    delete container;

    // Add the closing curly bracket.
    result->AddSon (get_node (3));
  // An empty statement sequence.
  } else {
    // Add the closing curly bracket.
    result->AddSon (get_node (2));
  }

  return result;
}
