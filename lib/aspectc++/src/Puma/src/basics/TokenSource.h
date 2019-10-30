// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#ifndef PUMA_TokenSource_H
#define PUMA_TokenSource_H

/** \file
 * Token source interface. */

namespace Puma {

class Token;

/** \class TokenSource TokenSource.h Puma/TokenSource.h
 * Token source interface. To be implemented by all
 * sources providing lexical tokens.
 * \ingroup basics */
class TokenSource {
public:
  /** Destructor. */
  virtual ~TokenSource() {
  }
  /** Get the next token.
   * \return The next token from the source, or NULL if no more tokens. */
  virtual Token *next() = 0;
};

} // namespace Puma

#endif /* PUMA_TokenSource_H */
