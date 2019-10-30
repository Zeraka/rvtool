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

#include "Puma/List.h"
#include <assert.h>

namespace Puma {

List::List(const List &list) {
  m_first = m_last = (ListElement*) 0;
  *this = list;
}

List::~List() {
  clear();
}

List &List::operator =(const List &list) {
  clear();
  return *this += list;
}

List &List::operator +=(const List &list) {
  ListElement *curr;

  curr = list.m_first;
  while (curr) {
    append(*curr->duplicate());
    curr = curr->m_next;
  }
  return *this;
}

List List::operator +(const List &list) {
  List result;

  result += *this;
  result += list;
  return result;
}

void List::clear() {
  ListElement *curr;
  ListElement *next;

  curr = m_first;
  while (curr) {
    next = curr->m_next;
    curr->m_prev = 0;
    curr->m_next = 0;
    assert(curr->m_belonging_to == this);
    curr->m_belonging_to = 0;
    delete curr;
    curr = next;
  }
  m_first = m_last = (ListElement*) 0;
}

List *List::copy(ListElement *from, ListElement *to) {
  ListElement *curr = from ? from : m_first;
  ListElement *the_end = to ? to->m_next : 0;
  List *list = new List;

  while (curr && curr != the_end) {
    list->append(*curr->duplicate());
    curr = curr->m_next;
  }
  return list;
}

void List::cut(List &out, ListElement *from, ListElement *to) {
  ListElement *next, *curr = from ? from : m_first;
  ListElement *the_end = to ? to->m_next : from ? from->m_next : 0;

  while (curr && curr != the_end) {
    next = curr->m_next;
    remove(curr);
    out.append(*curr);
    curr = next;
  }
}

void List::paste(ListElement *at, const List &l) {
  ListElement *curr = l.m_first;
  ListElement *inspos = at ? at : m_last;

  while (curr) {
    if (!inspos) {
      append(*(curr->duplicate()));
    } else {
      insert(inspos, *(curr->duplicate()));
      inspos = inspos->m_next;
    }
    curr = curr->m_next;
  }
}

void List::paste_before(ListElement *at, const List &l) {
  if (at && at->m_prev) {
    paste(at->m_prev, l);
  } else {
    ListElement *curr = l.m_last;

    while (curr) {
      prepend(*(curr->duplicate()));
      curr = curr->m_prev;
    }
  }
}

void List::move(ListElement *at, List &l) {
  ListElement *curr = l.m_first;
  ListElement *inspos = at ? at : m_last;

  while (curr) {
    l.remove(curr);
    if (!inspos) {
      append(*curr);
    } else {
      insert(inspos, *curr);
      inspos = inspos->m_next;
    }
    curr = l.m_first;
  }
}

void List::move_before(ListElement *at, List &l) {
  if (at && at->m_prev) {
    move(at->m_prev, l);
  } else {
    ListElement *curr = l.m_last;

    while (curr) {
      l.remove(curr);
      prepend(*curr);
      curr = l.m_last;
    }
  }
}

void List::append(ListElement &element) {
  assert(!element.m_belonging_to);
  element.m_next = (ListElement*) 0;
  element.m_prev = m_last;
  element.m_belonging_to = this;
  if (empty())
    m_first = &element;
  else
    m_last->m_next = &element;
  m_last = &element;
}

void List::prepend(ListElement &element) {
  assert(!element.m_belonging_to);
  element.m_prev = (ListElement*) 0;
  element.m_next = m_first;
  element.m_belonging_to = this;
  if (empty())
    m_last = &element;
  else
    m_first->m_prev = &element;
  m_first = &element;
}

void List::insert(ListElement *at, ListElement &element) {
  assert(!element.m_belonging_to);
  element.m_prev = at;
  element.m_next = at->m_next;
  element.m_belonging_to = this;
  if (at && at->m_next)
    at->m_next->m_prev = &element;
  else
    m_last = &element;
  if (at)
    at->m_next = &element;
}

void List::remove(ListElement *element) {
  if (!element)
    return;

  assert(element->m_belonging_to == this);
  element->m_belonging_to = (List*) 0;

  if (!element->m_next)
    m_last = element->m_prev;
  else
    element->m_next->m_prev = element->m_prev;

  if (!element->m_prev)
    m_first = element->m_next;
  else
    element->m_prev->m_next = element->m_next;

  element->m_prev = element->m_next = (ListElement*) 0;
}

} // namespace Puma
