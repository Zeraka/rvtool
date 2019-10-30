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

#ifndef PUMA_Array_H
#define PUMA_Array_H

/** \file
 *  Variable length array implementation. */

#include <assert.h>

namespace Puma {

/** \class Array Array.h Puma/Array.h
 *  Variable length array implementation. Grows with an increasing increment.
 *  \tparam Item The array item type. 
 *  \ingroup basics */
template<class Item>
class Array {
  Item* data;
  long size;
  long count;
  long increment;

  void grow(long wanted);

protected:
  /** Default array size. */
  static const long default_init_size = 5;
  /** Default array size increment. */
  static const long default_increment = 5;

public:
  /** Constructor.
   *  \param is The default size of the array. 
   *  \param incr The default increment value. */
  Array(long is = default_init_size, long incr = default_increment);
  /** Copy-constructor.
   *  \param array The array to copy. */
  Array(const Array<Item>& array);
  /** Assignment operator.
   *  \param copy The array to copy. */
  Array<Item>& operator=(const Array<Item>& copy);
  /** Destructor. Destroys the array. */
  ~Array();
  /** Append an item to the array. 
   *  \param item The item to append. */
  void append(const Item& item);
  /** Insert an item at the given array index. 
   *  \param index The array index. 
   *  \param item The item to insert. */
  void insert(long index, const Item& item);
  /** Prepend an item. 
   *  \param item The item to prepend. */
  void prepend(const Item& item);
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
  Item& get(long index);
  /** Get a reference to the array item at the given 
   *  array index. If the index exceeds the array
   *  bounds, then the array is reallocated accordingly.
   *  \param index The array index. 
   *  \return A reference to the n-th array item. */
  Item& operator[](long index);
  /** Get the array item at the given array index.
   *  Does not return a reference, thus the item may
   *  be implicitely copied. 
   *  \warning Does not validate the index!
   *  \param index The array index. 
   *  \return The n-th array item. */
  Item fetch(long index) const;
  /** Get a reference to the array item at the given 
   *  array index.
   *  \warning Does not validate the index!
   *  \param index The array index. 
   *  \return A reference to the n-th array item. */
  Item& lookup(long index) const;
  /** Get the number of items in the array. 
   *  \return The length of the array. */
  long length() const;
};

template<class Item>
inline Array<Item>::Array(long is, long incr) {
  count = 0;
  size = is;
  increment = incr;
  data = 0;
}

template<class Item>
inline Array<Item>::Array(const Array<Item>& array) {
  count = 0;
  size = array.size;
  increment = array.increment;

  if (size && array.data) {
    data = new Item[size];
    for (; count < array.count; count++) {
      data[count] = array.data[count];
    }
  } else
    data = 0;
}

template<class Item>
inline Array<Item>& Array<Item>::operator=(const Array<Item>& array) {
  if (data)
    delete[] data;

  count = 0;
  size = array.size;
  increment = array.increment;

  if (size && array.data) {
    data = new Item[size];
    for (; count < array.count; count++) {
      data[count] = array.data[count];
    }
  } else
    data = 0;

  return *this;
}

template<class Item>
inline Array<Item>::~Array() {
  if (data)
    delete[] data;
}

template<class Item>
inline void Array<Item>::grow(long wanted) {
  do {
    size += increment;
    increment *= 2;
  } while (wanted >= size);

  if (data) {
    Item* new_data = new Item[size];
    for (long i = 0; i < count; i++) {
      new_data[i] = data[i];
    }
    delete[] data;
    data = new_data;
  } else
    data = new Item[size];
}

template<class Item>
inline void Array<Item>::append(const Item& item) {
  if (count >= size || !data) {
    grow(count);
  }
  data[count++] = item;
}

template<class Item>
inline void Array<Item>::prepend(const Item& item) {
  insert(0, item);
}

template<class Item>
inline void Array<Item>::insert(long index, const Item& item) {
  if (count >= size || !data) {
    grow(count);
  }
  for (long i = count; i > index; i--)
    data[i] = data[i - 1];
  data[index] = item;
  count++;
}

template<class Item>
inline Item& Array<Item>::get(long index) {
  if (index >= size || !data) {
    grow(index);
  }
  if (index >= count)
    count = index + 1;
  return data[index];
}

template<class Item>
inline Item& Array<Item>::operator[](long index) {
  return get(index);
}

template<class Item>
inline Item Array<Item>::fetch(long index) const {
  assert(index < count && data);
  return data[index];
}

template<class Item>
inline long Array<Item>::length() const {
  return count;
}

template<class Item>
inline void Array<Item>::remove(long index) {
  if (index < count && count > 0) {
    for (long i = index; i < count - 1; i++)
      data[i] = data[i + 1];
    count--;
  }
}

template<>
inline void Array<int>::reset() {
  count = 0;
}

template<class Item>
inline void Array<Item>::reset() {
  if (data)
    delete[] data;

  count = 0;
  size = default_init_size;
  increment = default_increment;
  data = new Item[size];
}

template<class Item>
inline Item& Array<Item>::lookup(long index) const {
  assert(index >= 0 && index < count);
  return data[index];
}

} // namespace Puma

// Array specialization for pointer types sharing method definitions

#include "Puma/VoidPtrArray.h"

namespace Puma {

/** Variable length array implementation specialized for pointer types. 
 * Grows with an increasing increment.
 * \tparam Item The array item type. Needs to be a pointer type. 
 * \ingroup basics */
template<class Item>
class Array<Item*> : private VoidPtrArray {
  typedef VoidPtrArray Base;

public:
  /** Constructor.
   *  \param is The default size of the array. 
   *  \param incr The default increment value. */
  Array(long is = default_init_size, long incr = default_increment)
      : Base(is, incr) {
  }
  /** Copy-constructor.
   *  \param array The array to copy. */
  Array(const Array<Item*> &array)
      : Base(array) {
  }
  /** Destructor. Destroys the array. */
  ~Array() {
  }
  /** Assignment operator.
   *  \param array The array to copy.
   *  \return A reference to this array. */
  Array<Item*> &operator=(const Array<Item*> &array) {
    return (Array<Item*>&) Base::operator=(array);
  }
  /** Append an item to the array. 
   *  \param item The item to append. */
  void append(const Item *item) {
    Base::append((void*) item);
  }
  /** Insert an item at the given array index. 
   *  \param idx The array index.
   *  \param item The item to insert. */
  void insert(long idx, const Item *item) {
    Base::insert(idx, (void*) item);
  }
  /** Prepend an item. 
   *  \param item The item to prepend. */
  void prepend(const Item *item) {
    Base::prepend((void*) item);
  }
  /** Remove the array item at the given array index.
   *  \param idx The array index. */
  void remove(long idx) {
    Base::remove(idx);
  }
  /** Reset the array. Destroys the current array. */
  void reset() {
    Base::reset();
  }
  /** Get a reference to the array item at the given 
   *  array index. If the index exceeds the array
   *  bounds, then the array is reallocated accordingly.
   *  \param idx The array index.
   *  \return A reference to the n-th array item. */
  Item *&get(long idx) {
    return (Item*&) Base::get(idx);
  }
  /** Get a reference to the array item at the given 
   *  array index. If the index exceeds the array
   *  bounds, then the array is reallocated accordingly.
   *  \param idx The array index.
   *  \return A reference to the n-th array item. */
  Item *&operator[](long idx) {
    return (Item*&) Base::operator[](idx);
  }
  /** Get the array item at the given array index.
   *  \warning Does not validate the index!
   *  \param idx The array index.
   *  \return The n-th array item. */
  Item *fetch(long idx) const {
    return (Item*) Base::fetch(idx);
  }
  /** Get a reference to the array item at the given 
   *  array index.
   *  \warning Does not validate the index!
   *  \param idx The array index.
   *  \return A reference to the n-th item. */
  Item *&lookup(long idx) const {
    return (Item*&) Base::lookup(idx);
  }
  /** Get the number of items in the array. 
   *  \return Length of the array. */
  long length() const {
    return Base::length();
  }
};

} // namespace Puma

#endif /* PUMA_Array_H */
