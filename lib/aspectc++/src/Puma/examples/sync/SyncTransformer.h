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

#ifndef __SyncTransformer_h__
#define __SyncTransformer_h__

#include "Puma/CVisitor.h"
#include "Puma/ErrorStream.h"
#include "SyncTree.h"


// Transformer for the "synchronized" extension.
class SyncTransformer : private Puma::CVisitor {
  // This counter is later used for generating
  // variable names.
  static unsigned m_Counter;
  // A reference to the global error stream.
  Puma::ErrorStream &m_Err;

public:
  // Constructor.
  SyncTransformer(Puma::ErrorStream &err);

  // Start the transformation of the
  // "synchronized" statements.
  void transform(Puma::CTree *node);

protected:
  // Called for every node before the sub-nodes of
  // this node are visited. This is part of the
  // visitor mechanism.
  void pre_visit(Puma::CTree *node);

  // The transformation action to be performed before
  // the sub-nodes of the "synchronized" statement node
  // are visited.
  void pre_action(SyncStmt *node);
};


#endif /* __SyncTransformer_h__ */
