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

#ifndef PUMA_VoidPtrArray_H
#define PUMA_VoidPtrArray_H

/** \file
 * Array of void pointers. */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

namespace Puma {

/** \class VoidPtrArray VoidPtrArray.h Puma/VoidPtrArray.h
 * Array implementation specialized for void pointers.
 * \ingroup basics */
class VoidPtrArray {
  void** data;
  long size;
  long count;
  long increment;

  void grow(long wanted);

protected:
  /** Default array size. */
  static const long default_init_size;
  /** Default array size increment. */
  static const long default_increment;

protected:
  /** Constructor.
   *  \param is The default size of the array.
   *  \param incr The default increment value. */
  VoidPtrArray(long is = default_init_size, long incr = default_increment);
  /** Copy-constructor.
   *  \param array The array to copy. */
  VoidPtrArray(const VoidPtrArray &array);
  /** Assignment operator.
   *  \param array The array to copy. 
   *  \return A reference to this array. */
  VoidPtrArray &operator =(const VoidPtrArray &array);
  /** Destructor. Destroys the array. */
  ~VoidPtrArray();
  /** Append an item to the array.
   *  \param item The item to append. */
  void append(const void *item);
  /** Insert an item at the given array index.
   *  \param index The array index.
   *  \param item The item to insert. */
  void insert(long index, const void *item);
  /** Prepend an item.
   *  \param item The item to prepend. */
  void prepend(const void *item);
  /** Remove the array item at the given array index.
   *  \param index The array index. */
  void remove(long index);
  /** Reset the array. Destroys the current array. */
  void reset();
  /** Get a reference to the array item at the given
   *  array index. If the index exceeds the array
   *  bounds, then the array is reallocated accordingly.
   *  \param index The array index.
   *  \return A reference to the n-th array item. */
  void *&get(long index);
  /** Get a reference to the array item at the given
   *  array index. If the index exceeds the array
   *  bounds, then the array is reallocated accordingly.
   *  \param index The array index.
   *  \return A reference to the n-th array item. */
  void *&operator[](long index);
  /** Get the array item at the given array index.
   *  Does not return a reference, thus the item may
   *  be implicitely copied.
   *  \warning Does not validate the index!
   *  \param index The array index.
   *  \return The n-th array item. */
  void *fetch(long index) const;
  /** Get a reference to the array item at the given
   *  array index.
   *  \warning Does not validate the index!
   *  \param index The array index.
   *  \return A reference to the n-th array item. */
  void *&lookup(long index) const;
  /** Get the number of items in the array.
   *  \return The length of the array. */
  long length() const;
};

inline VoidPtrArray::VoidPtrArray(long is, long incr) {
  count = 0;
  size = 0;
  increment = incr;
  data = 0;
}

inline VoidPtrArray::VoidPtrArray(const VoidPtrArray &array) {
  count = array.count;
  size = array.size;
  increment = array.increment;

  if (size) {
    data = (void**) ::malloc(size * sizeof(void*));
    ::memcpy(data, array.data, count * sizeof(void*));
  } else
    data = 0;
}

inline VoidPtrArray &VoidPtrArray::operator =(const VoidPtrArray &array) {
  if (data)
    ::free(data);

  count = array.count;
  size = array.size;
  increment = array.increment;

  if (size) {
    data = (void**) ::malloc(size * sizeof(void*));
    ::memcpy(data, array.data, count * sizeof(void*));
  } else
    data = 0;

  return *this;
}

inline VoidPtrArray::~VoidPtrArray() {
  if (data)
    ::free(data);
}

inline void VoidPtrArray::append(const void *item) {
  if (count >= size) {
    grow(count);
  }
  data[count++] = (void*) item;
}

inline void VoidPtrArray::prepend(const void *item) {
  insert(0, item);
}

inline void VoidPtrArray::insert(long index, const void *item) {
  if (count >= size) {
    grow(count);
  }
  for (int pos = count; pos > index; pos--)
    data[pos] = data[pos - 1];
  data[index] = (void*) item;
  count++;
}

inline void *&VoidPtrArray::get(long index) {
  if (index >= size) {
    grow(index);
  }
  if (index >= count)
    count = index + 1;
  return data[index];
}

inline void *&VoidPtrArray::operator[](long index) {
  return get(index);
}

inline void *VoidPtrArray::fetch(long index) const {
  return data[index];
}

inline long VoidPtrArray::length() const {
  return count;
}

inline void VoidPtrArray::grow(long wanted) {
  do {
    size += increment;
    increment *= 2;
  } while (wanted >= size);

  if (data)
    data = (void**) ::realloc(data, size * sizeof(void*));
  else
    data = (void**) ::malloc(size * sizeof(void*));
}

inline void VoidPtrArray::remove(long index) {
  if (index < count && count > 0) {
    for (int pos = index; pos < count - 1; pos++)
      data[pos] = data[pos + 1];
    count--;
  }
}

inline void VoidPtrArray::reset() {
  count = 0;
  if (data)
    ::free(data);
  size = 0;
  data = 0;
  increment = default_increment;
}

inline void *&VoidPtrArray::lookup(long index) const {
  assert(index >= 0 && index < count);
  return data[index];
}

} // namespace Puma

#endif /* PUMA_VoidPtrArray_H */
