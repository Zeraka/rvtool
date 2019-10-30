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

#ifndef PUMA_Chain_H
#define PUMA_Chain_H

/** \file
 * Simple chain implementation.
 */

namespace Puma {

/** \class Chain Chain.h Puma/Chain.h
 * Simple single-linked chain implementation.
 * A chain consists of elements each linked to its direct follower element. 
 * \ingroup basics */
class Chain {
  Chain *next;

public:
  /** Construct a single chain element. */
  Chain() {
    next = 0;
  }
  /** Select an element as the next element.
   * \param element The next element. */
  void select(Chain *element) {
    next = element;
  }
  /** Get the next element.
   * \return A pointer to the next chain element. */
  Chain *select() const {
    return next;
  }
  /** Unlink the next element from the chain.
   * \return The unlinked next element. */
  Chain *unlink() {
    Chain *res = next;
    if (next) {
      next = next->select();
    }
    return res;
  }
  /** Insert an element in the chain as the next element.
   * \param element The element to insert. */
  void insert(Chain *element) {
    element->select(next);
    next = element;
  }
};

} // namespace Puma

#endif /* PUMA_Chain_H */
