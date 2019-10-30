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

#ifndef PUMA_ListElement_H
#define PUMA_ListElement_H

/** \file
 *  Double-linked list element. */

#include <assert.h>

namespace Puma {

class List;

/** \class ListElement ListElement.h Puma/ListElement.h
 * Double-linked list element type to be used as elements
 * for class List. To be derived for concrete list elements. 
 * \ingroup basics */
class ListElement {
  friend class List;

  List *m_belonging_to;
  ListElement *m_next;
  ListElement *m_prev;

public:
  /** Constructor. */
  ListElement()
      : m_belonging_to(0), m_next(0), m_prev(0) {
  }
  /** Destructor. */
  virtual ~ListElement() {
    assert(!m_belonging_to);
  }
  /** Get the list this element belongs to. 
   *  \return A pointer to the list, or NULL if not belonging to any list. */
  List *belonging_to() const {
    return m_belonging_to;
  }
  /** Duplicate this list element. To be implemented by concrete
   * list element implementations.
   * \return A pointer to the new list element. */
  virtual ListElement *duplicate() = 0;
};

} // namespace Puma

#endif /* PUMA_ListElement_H */
