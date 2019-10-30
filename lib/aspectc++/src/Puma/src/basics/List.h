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

#ifndef PUMA_List_H
#define PUMA_List_H

/** \file
 *  Double-linked list implementation. */

#include "Puma/ListElement.h"

namespace Puma {

/** \class List List.h Puma/List.h
 * Double-linked list implementation. 
 * \ingroup basics */
class List {
  ListElement *m_first;
  ListElement *m_last;

public:
  /** Constructor. */
  List()
      : m_first((ListElement*) 0), m_last((ListElement*) 0) {
  }
  /** Copy-constructor.
   *  \param copy The list to copy. */
  List(const List &copy);
  /** Destructor. Destroys the list elements. */
  ~List();

  /** Assignment operator.
   *  \param copy The list to copy.
   *  \return A reference to this list. */
  List &operator =(const List &copy);
  /** Append the given list.
   *  \param list The list to append.
   *  \return A reference to this list. */
  List &operator +=(const List &list);
  /** Create a new list containing the elements of this
   *  list followed by the elements of the given list. 
   *  \param list The other list.
   *  \return A new list containing the elements of both lists. */
  List operator +(const List &list);

  /** Destroy the list elements. */
  void clear();

  /** Append an element to the list. 
   *  \param element The element. */
  void append(ListElement &element);
  /** Prepend an element to the list. 
   *  \param element The element. */
  void prepend(ListElement &element);
  /** Insert an element at the given list position. 
   *  \param at A list element where to insert the element. 
   *  \param element The element. */
  void insert(ListElement *at, ListElement &element);
  /** Remove the given element from the list.
   *  \param element The list element. */
  void remove(ListElement *element);
  /** Remove and destroy all elements between the
   *  given list elements.
   *  \param from The first list element to kill.
   *  \param to The last list element to kill. */
  void kill(ListElement *from, ListElement *to = (ListElement*) 0) {
    List discard;
    cut(discard, from, to);
  }
  /** Cut all elements between the given list elements.
   *  \param out The container for the cutted elements (are appended).
   *  \param from The first list element to cut.
   *  \param to The last list element to cut. */
  void cut(List &out, ListElement *from, ListElement *to = (ListElement*) 0);
  /** Copy all elements between the given list elements.
   *  \param from The first list element to cut.
   *  \param to The last list element to cut. 
   *  \return A new list containing the copied elements. */
  List *copy(ListElement *from = (ListElement*) 0, ListElement *to = (ListElement*) 0);
  /** Insert copies of the elements of the given list at
   *  the given list position.
   *  \param at The list element at which to paste the list elements.
   *  \param l The list containing the elements to paste. */
  void paste(ListElement *at, const List &l);
  /** Insert copies of the elements of the given list before
   *  the given list position.
   *  \param at The list element before which to paste the list elements.
   *  \param l The list containing the elements to paste. */
  void paste_before(ListElement *at, const List &l);
  /** Insert the elements of the given list at the given 
   *  list position. The elements are removed from the source
   *  list.
   *  \param at The list element at which to insert the list elements.
   *  \param l The list containing the elements to insert. */
  void move(ListElement *at, List &l);
  /** Insert the elements of the given list before the given 
   *  list position. The elements are removed from the source
   *  list.
   *  \param at The list element before which to insert the list elements.
   *  \param l The list containing the elements to insert. */
  void move_before(ListElement *at, List &l);

  /** Check if the list is empty.
   * return True if the list is empty. */
  bool empty() const {
    return m_first == (ListElement*) 0;
  }

  /** Get the first element in the list. 
   *  \return A pointer to the first element, or NULL if list is empty. */
  const ListElement *first() const {
    return m_first;
  }
  /** Get the last element in the list. 
   *  \return A pointer to the last element, or NULL if list is empty. */
  const ListElement *last() const {
    return m_last;
  }
  /** Get the next element of the given element. 
   *  \return A pointer to the next element, or NULL if the given element is NULL. */
  const ListElement *next(const ListElement *element) const {
    return element ? element->m_next : 0;
  }
  /** Get the previous element of the given element. 
   *  \return A pointer to the previous element, or NULL if the given element is NULL. */
  const ListElement *prev(const ListElement *element) const {
    return element ? element->m_prev : 0;
  }
};

} // namespace Puma

#endif /* PUMA_List_H */
