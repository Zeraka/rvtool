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

#ifndef __SyncSemantic_h__
#define __SyncSemantic_h__

#include <Puma/CCSemantic.h>
#include "SyncSyntax.h"
#include "SyncBuilder.h"
#include "SyncTree.h"


// Semantic analyzer of the "synchronized" extension.
class SyncSemantic : public Puma::CCSemantic {
  // State variable, is greater than 0 while parsing the
  // statement sequence of a "synchronized" statement.
  int m_InSyncStmt;

public:
  // Constructor.
  SyncSemantic(SyncSyntax &syntax, SyncBuilder &builder);

protected:
  // Return the concrete builder and syntax objects.
  virtual SyncBuilder &builder() const;
  virtual SyncSyntax &syntax() const;

public:
  // Set the "parsing a synchronized statement" state.
  void enter_sync_stmt();
  // Reset the "parsing a synchronized statement" state.
  void leave_sync_stmt();

  // Check the "parsing a synchronized statement" state.
  // If not parsing a "synchronized" statement, build the
  // corresponding syntax tree node. Otherwise generate an
  // error message and return 0.
  Puma::CTree *sync_stmt();
};


#endif /* __SyncSemantic_h__ */
