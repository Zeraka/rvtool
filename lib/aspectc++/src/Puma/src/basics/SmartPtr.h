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

#ifndef PUMA_SmartPtr_H
#define PUMA_SmartPtr_H

/** \file
 * Smart pointer implementation. */

namespace Puma {

/** \class SmartPtr SmartPtr.h Puma/SmartPtr.h
 * Template based smart pointer implementation. Classes must
 * be derived from SmartPtr::RefCnt to be compatible with
 * this implementation. Example:
 *
 * \code
 * class Object : public SmartPtr<Object>::RefCnt {
 *   //...
 * };
 * SmartPtr<Object> object = new Object();
 * \endcode
 * \ingroup basics */
template<class T>
class SmartPtr {
public:
  /** \class RefCnt SmartPtr.h Puma/SmartPtr.h
   * Reference counter for objects used with SmartPtr. */
  class RefCnt {
    int m_counter;
    friend class SmartPtr<T> ;
  protected:
    /** Constructor. */
    RefCnt()
        : m_counter(0) {
    }
  };

private:
  T *m_pointer;

public:
  /** Construct an empty smart pointer. */
  SmartPtr()
      : m_pointer(0) {
  }
  /** Construct a new smart pointer.
   * \param pointer A pointer to the object to manage. */
  SmartPtr(const RefCnt *pointer)
      : m_pointer(0) {
    *this = pointer;
  }
  /** Copy a smart pointer. Increases the reference count
   * of the managed object.
   * \param copy The smart pointer to copy. */
  SmartPtr(const SmartPtr &copy)
      : m_pointer(0) {
    *this = copy.m_pointer;
  }
  /** Destroy the smart pointer. If this was the last
   * smart pointer referencing the managed object, the
   * object is destroyed too. */
  ~SmartPtr() {
    clear();
  }
  /** Clear this smart pointer. If this was the last
   * smart pointer referencing the managed object, the
   * object is destroyed. */
  void clear();
  /** Check whether the smart pointer is valid. A smart pointer
   * is valid if the managed pointer isn't NULL.
   * \return True if valid. */
  bool valid() const {
    return m_pointer != 0;
  }
  /** Compare this smart pointer with another smart pointer.
   * \param other The smart pointer to compare with.
   * \return True if both smart pointers point to the same object. */
  bool operator==(const SmartPtr &other) const {
    return m_pointer == other.m_pointer;
  }
  /** Assign a new object to this smart pointer. If this smart pointer
   * already points to an object, then this object might be destroyed
   * if not referenced by another smart pointer.
   * \param copy The smart pointer to copy.
   * \return A reference to this smart pointer. */
  SmartPtr &operator=(const SmartPtr &copy) {
    return *this = copy.m_pointer;
  }
  /** Assign a new object to this smart pointer. If this smart pointer
   * already points to an object, then this object might be destroyed
   * if not referenced by another smart pointer.
   * \param copy The smart pointer to copy.
   * \return A reference to this smart pointer. */
  SmartPtr &operator=(const RefCnt *copy);
  /** Conversion operator to check whether the smart pointer is valid.
   * \return True if valid. */
  operator bool() const {
    return valid();
  }
  /** Conversion operator for the managed object pointer type.
   * \return A pointer to the managed object. */
  operator T*() const {
    return m_pointer;
  }
  /** Conversion operator for the managed object pointer type.
   * \return A constant pointer to the managed object. */
  operator const T*() const {
    return m_pointer;
  }
  /** Dereference operator for the managed object pointer type.
   * \return A pointer to the managed object. */
  T* operator->() {
    return m_pointer;
  }
  /** Dereference operator for the managed object pointer type.
   * \return A constant pointer to the managed object. */
  const T* operator->() const {
    return m_pointer;
  }
  /** Indirection operator for the managed object pointer type.
   * \return A pointer to the managed object. */
  T* operator*() {
    return m_pointer;
  }
  /** Indirection operator for the managed object pointer type.
   * \return A constant pointer to the managed object. */
  const T* operator*() const {
    return m_pointer;
  }
};

template<class T>
inline SmartPtr<T> &SmartPtr<T>::operator=(const RefCnt *copy) {
  if (copy != m_pointer) {
    clear();
    m_pointer = (T*) copy;
    if (valid())
      m_pointer->m_counter++;
  }
  return *this;
}

template<class T>
inline void SmartPtr<T>::clear() {
  if (valid()) {
    m_pointer->m_counter--;
    if (m_pointer->m_counter == 0)
      delete m_pointer;
    m_pointer = (T*) 0;
  }
}

} // namespace Puma

#endif /* PUMA_SmartPtr_H */
