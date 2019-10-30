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

#ifndef PUMA_Stack_H
#define PUMA_Stack_H

/** \file
 * Stack implementation. */

#include "Puma/Array.h"

namespace Puma {

/** \class Stack Stack.h Puma/Stack.h
 * Stack implementation based on a variable length array.
 * \tparam Item The stack item type.
 * \ingroup basics */
template<class Item>
class Stack : private Array<Item> {
public:
  /** Construct a new stack.
   *  \param is The default size of the stack.
   *  \param incr The default increment value for the stack size. */
  Stack(long is = 8192, long incr = 8192)
      : Array<Item>(is, incr) {
  }
  /** Add a new item on top of the stack.
   * \param item The item to add. */
  void push(Item item) {
    Array<Item>::append(item);
  }
  /** Get the top item from the stack.
   * \return A reference to the top stack item. */
  Item &top() const {
    long len = Array<Item>::length() - 1;
    return Array<Item>::lookup(len >= 0 ? len : 0);
  }
  /** Remove the top item from the stack. */
  void pop() {
    Array<Item>::remove(Array<Item>::length() - 1);
  }
  /** Get the number of items on the stack.
   * \return The number of items. */
  long length() const {
    return Array<Item>::length();
  }
  /** Reset the stack. */
  void reset() {
    Array<Item>::reset();
  }
};

} // namespace Puma

#endif /* __stack_h__ */
