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

#ifndef __Transformer_h__
#define __Transformer_h__

#include <Puma/ErrorStream.h>
#include <Puma/CVisitor.h>
#include <Puma/CTree.h>


// Transform the source code on syntax tree level.
class Transformer : private Puma::CVisitor {
  Puma::ErrorStream& m_Err;

private:
  // Insert the comment before the first token of
  // the matched syntax tree node.
  void insertComment(Puma::CTree *node);

protected:
  // Overwrite pre_visit(...) from Puma::CVisitor
  // that is called before a subtree is visited.
  void pre_visit(Puma::CTree* node);

public:
  Transformer(Puma::ErrorStream& err);
  void transform(Puma::CTree* node);
};


#endif /* __Transformer_h__ */
