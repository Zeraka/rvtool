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

#ifndef __SyncParser_h__
#define __SyncParser_h__

#include <Puma/Parser.h>
#include "SyncSyntax.h"
#include "SyncBuilder.h"
#include "SyncSemantic.h"


// Parser of the "synchronized" extension.
class SyncParser : public Puma::Parser {
  // The "synchronized" extension grammar.
  SyncSyntax m_Syntax;
  // The "synchronized" extension syntax tree builder.
  SyncBuilder m_Builder;
  // The "synchronized" extension semantic analyzer.
  SyncSemantic m_Semantic;

public:
  // Constructor, initialize the members and base class.
  SyncParser() : Puma::Parser(m_Syntax, m_Builder, m_Semantic),
                 m_Syntax(m_Builder, m_Semantic),
                 m_Semantic(m_Syntax, m_Builder) {}

  // Member access methods.
  SyncSyntax &syntax() const { return (SyncSyntax&)m_Syntax; }
  SyncBuilder &builder() const { return (SyncBuilder&)m_Builder; }
  SyncSemantic &semantic() const { return (SyncSemantic&)m_Semantic; }
};


#endif /* __SyncParser_h__ */
