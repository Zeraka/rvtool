// This file is part of PUMA.
// Copyright (C) 1999-2016  The PUMA developer team.
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

#ifndef PUMA_Tokenizer_H
#define PUMA_Tokenizer_H

/** \file
 * String tokenizer interface. */

#include "Puma/Source.h"
#include "Puma/Unit.h"

namespace Puma {

/** \class Tokenizer Tokenizer.h Puma/Tokenizer.h
 * Tokenizers read input from a Source to form tokens
 * which are stored in a Unit.
 * \ingroup common */
class Tokenizer {
public:
  /** Destructor. */
  virtual ~Tokenizer() {
  }
  /** Fill a unit from the given source.
   * \param in The input source.
   * \param unit The output unit. */
  virtual void fill_unit(Source &in, Unit &unit) = 0;
};

} // namespace Puma

#endif /* PUMA_Tokenizer_H */
