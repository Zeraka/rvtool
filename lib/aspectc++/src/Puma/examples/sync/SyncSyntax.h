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

#ifndef __SyncSyntax_h__
#define __SyncSyntax_h__

#include <Puma/CCSyntax.h>


// Cannot include the header of SyncSemantic because
// SyncSemantic need to include this header.
class SyncBuilder;
class SyncSemantic;


// Grammar for the "synchronized" extension.
class SyncSyntax : public Puma::CCSyntax {
public:
  // Constructor.
  SyncSyntax(SyncBuilder &, SyncSemantic &);

protected:
  // Return the concrete builder and semantic object.
  SyncBuilder &builder() const;
  SyncSemantic &semantic() const;

protected:
  // Overwrite statement parse method to first try
  // to parse a synchronized statement.
  virtual bool stmt();
  // Add syntax rule for synchronized statements.
  Puma::CTree *rule_sync_stmt();
};


#endif /* __SyncSyntax_h__ */
