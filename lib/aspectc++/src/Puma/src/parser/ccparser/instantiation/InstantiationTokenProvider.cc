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

/** \file
 *  Instantiation token provider implementation. */

#include "Puma/InstantiationTokenProvider.h"
#include "Puma/CTree.h"

namespace Puma {


/** Read next token. */
TokenInfo InstantiationTokenProvider::read() {
  TokenInfo ti;
  ti._token = _source.next();
  ti._context = ((InstantiationTokenSource&) _source).node();
  return ti;
}


} // namespace Puma
