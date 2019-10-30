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

#ifndef PUMA_BCStack_H
#define PUMA_BCStack_H

/** \file
 * Bucket stack implementation. */

#include "Puma/BCList.h"

namespace Puma {

/** \class BCStack BCStack.h Puma/BCStack.h
 * %Stack of fixed-size buckets.
 * A bucket is a fixed-size array that is filled sequentially. Each bucket
 * is linked with the previous and next buckets forming a double linked list.
 * Items are accessed last-in first-out. No random access supported.
 *
 * \tparam Item The stack item type.
 * \tparam BUCKET_SIZE Optional size of each bucket. Defaults to 32. 
 * \ingroup basics */
template<class Item, int BUCKET_SIZE = 32>
class BCStack : private BCList<Item, BUCKET_SIZE> {
  typedef BCList<Item, BUCKET_SIZE> List;

public:
  /** Iterator type for the bucket stack. */
  typedef typename List::Iterator Iterator;

private:
  Iterator m_tos;
  long m_length;

public:
  /** Construct an empty bucket stack. */
  BCStack()
      : m_length(0L) {
  }
  /** Get the length of the bucket stack.
   * \return The current length. */
  long length() const {
    return m_length;
  }
  /** Push a new item on top of the stack.
   * \param item The item to add. */
  void push(Item item) {
    if (m_tos)
      ++m_tos;
    if (!m_tos)
      m_tos = this->add(item);
    else
      *m_tos = item;
    m_length++;
  }
  /** Get the top item of the stack.
   * \return A reference to the top item. */
  Item &top() const {
    return *m_tos;
  }
  /** Remove the top item from the stack. */
  void pop() {
    if (!empty()) {
      --m_tos;
      m_length--;
    }
  }
  /** Reset the bucket stack. Does not delete the stack items. */
  void reset() {
    List::reset();
    m_tos = typename List::Iterator();
    m_length = 0L;
  }
  /** Check if the stack is empty.
   * \return True if the stack is empty. */
  bool empty() const {
    return m_length == 0L;
  }
};

} // namespace Puma

#endif /* PUMA_BCStack_H */
