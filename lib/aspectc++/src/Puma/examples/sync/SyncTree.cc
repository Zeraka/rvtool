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

#include "SyncTree.h"


SyncStmt::SyncStmt() {
  // The "synchronized" statement is simply a statement
  // list introduced by the keyword "synchronized" and
  // enclosed by an opening and closing curly bracket.
  AddProperties(INTRO | OPEN_CLOSE);
}


const char *SyncStmt::NodeId() {
  // Every kind of syntax tree node has a unique identifier.
  return "SyncStmt";
}


const char *SyncStmt::NodeName() const {
  // This method returns the unique identifier of
  // this kind of syntax tree node. It is mostly used
  // in conjunction with NodeId() to identify a node,
  // e.g. if (node->NodeName() == SyncStmt::NodeId())
  return NodeId();
}
