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

#ifndef __InstantiationTokenProvider__
#define __InstantiationTokenProvider__

/** \file
 *  Instantiation token provider. */

#include "Puma/TokenProvider.h"
#include "Puma/InstantiationTokenSource.h"

namespace Puma {

/** \class InstantiationTokenProvider InstantiationTokenProvider.h Puma/InstantiationTokenProvider.h
 *  Provides token infos (token pointer + context) from a syntax tree. The context is set to
 *  the syntax tree node in which the token was found. */
class InstantiationTokenProvider : public TokenProvider {
protected:
  /** Read next token info. */
  virtual TokenInfo read();

public:
  /** Constructor.
   *  \param s The token source from which to read the tokens. */
  InstantiationTokenProvider(InstantiationTokenSource &s) : TokenProvider(s) {}
};


} // namespace Puma

#endif /* __InstantiationTokenProvider__ */
