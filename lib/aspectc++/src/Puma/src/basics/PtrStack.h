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

#ifndef PUMA_PtrStack_H
#define PUMA_PtrStack_H

/** \file
 * Layered pointer stack implementation. */

#include "Puma/Stack.h"
#include "Puma/BCStack.h"

namespace Puma {

/** \class PtrStack PtrStack.h Puma/PtrStack.h
 * Layered stack for pointers to objects. Each layer is a separate stack.
 * Layers can be added, dropped, or destroyed. 
 * \ingroup basics */
template<class T>
class PtrStack : private Array<T*> {
  BCStack<long, 8192> m_indices; // start indices

public:
  /** Construct a new layered stack.
   * \param size The size of each layer. Defaults to 8192.
   * \param incr The increment for the size of each layer. Defaults to 8192. */
  PtrStack(long size = 8192, long incr = 8192);
  /** Destructor. */
  virtual ~PtrStack() {
  }

  /** \name Stack operations */
  //@{
  /** Add a new item on the top of the current layer.
   * \param item The item to add. */
  void Push(const T *item);
  /** Remove the top item from the current layer. */
  void Pop();
  /** Get the top item from the current layer.
   * \return The top item, or NULL if the current layer is empty. */
  T *Top() const;
  /** Get the number of items added to the current layer.
   * \return The number of items on the current layer. */
  long Length() const;
  /** Get the n-th item on the current layer.
   * \param n The index of the item to get.
   * \return The n-th item, or NULL if the index is invalid. */
  T *Get(long n) const;
  //@}

  /** \name Layer operations */
  //@{
  /** Create a new layer. */
  void New();
  /** Remove the top layer and add its items to the underlying layer. */
  void Forget();
  /** Remove the top layer. */
  void Reject();
  /** Remove the top layer and delete its items. */
  void Destroy();
  /** Get the number of layers.
   * \return Number of the layers. */
  long Stacks() const;
  /** Remove all layers. */
  void Reset();
  //@}

protected:
  /** Remove the top item of the current layer. */
  virtual void Remove();
  /** Remove and delete the top item of the current layer. */
  virtual void Delete();

private:
  long Index() const;    // return current index
  long Bottom() const;   // return start index of current stack
};

template<class T>
inline PtrStack<T>::PtrStack(long is, long incr)
    : Array<T*>(is, incr) {
  New();
}

template<class T>
inline void PtrStack<T>::Reset() {
  m_indices.reset();
  Array<T*>::reset();
  New();
}

template<class T>
inline long PtrStack<T>::Index() const {
  return Array<T*>::length() - 1;
}

template<class T>
inline long PtrStack<T>::Bottom() const {
  return m_indices.top();
}

template<class T>
inline long PtrStack<T>::Stacks() const {
  return m_indices.length();
}

template<class T>
inline long PtrStack<T>::Length() const {
  return Index() - Bottom();
}

template<class T>
inline T *PtrStack<T>::Top() const {
  if (Length() > 0)
    return Array<T*>::lookup(Index());
  else
    return (T*) 0;
}

template<class T>
inline void PtrStack<T>::Push(const T *item) {
  this->append((T*) item);
}

template<class T>
inline void PtrStack<T>::Pop() {
  if (Length() > 0)
    Array<T*>::remove(Index());
}

template<class T>
inline T *PtrStack<T>::Get(long idx) const {
  if (Length() <= 0 || idx < 0 || (unsigned long) idx >= (unsigned long) Length())
    return (T*) 0;
  return Array<T*>::lookup(Bottom() + 1 + idx);
}

template<class T>
inline void PtrStack<T>::New() {
  m_indices.push(Index());
}

template<class T>
inline void PtrStack<T>::Forget() {
  m_indices.pop();
}

template<class T>
inline void PtrStack<T>::Reject() {
  while (Length() > 0)
    Remove();
  Forget();
}

template<class T>
inline void PtrStack<T>::Destroy() {
  while (Length() > 0) {
    Delete();
    Array<T*>::remove(Index()); // Pop()
  }
  Forget();
}

template<class T>
inline void PtrStack<T>::Remove() {
  Array<T*>::remove(Index()); // Same as Pop()
}

template<class T>
inline void PtrStack<T>::Delete() {
  T *item = Top();
  if (item)
    delete item;
}

} // namespace Puma

#endif /* PUMA_PtrStack_H */
