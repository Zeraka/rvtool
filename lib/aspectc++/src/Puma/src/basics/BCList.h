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

#ifndef PUMA_BCList_H
#define PUMA_BCList_H

/** \file
 * Bucket list implementation. */

namespace Puma {

/** \class BCList BCList.h Puma/BCList.h
 * %List of fixed-size buckets.
 * A bucket is a fixed-size array that is filled sequentially. Each bucket
 * is linked with the previous and next buckets forming a double linked list.
 * Random access to list items is not supported.
 *
 * \tparam Item The list item type.
 * \tparam BUCKET_SIZE Optional size of each bucket. Defaults to 32. 
 * \ingroup basics */
template<class Item, int BUCKET_SIZE = 32>
class BCList {
  struct Bucket {
    Bucket *m_prev;
    Bucket *m_next;
    Item *m_write_pos;
    Item *m_end_pos;
    Item m_data[BUCKET_SIZE]; // large amount of token pointers to avoid new

    Bucket()
        : m_prev(0), m_next(0), m_write_pos(m_data), m_end_pos(m_data + BUCKET_SIZE) {
    }
    ~Bucket() {
      if (m_next)
        delete m_next;
    }
    void clear() {
      if (m_next)
        delete m_next;
      m_next = 0;
      m_write_pos = m_data;
    }
    Item *first() {
      return m_data;
    }
    Item *pos() const {
      return m_write_pos;
    }
    Item *end() const {
      return m_end_pos;
    }
    bool full() const {
      return m_write_pos == m_end_pos;
    }
    Bucket *new_bucket() {
      m_next = new Bucket;
      m_next->m_prev = this;
      return m_next;
    }
    void add(Item token) {
      *m_write_pos = token;
      m_write_pos++;
    }
  };

  Bucket *m_first_bucket; // the first bucket is part of the container
  Bucket *m_last_bucket; // pointer to the last bucket for insertion

  void check() {
    if (m_first_bucket == 0) {
      m_first_bucket = new Bucket();
      m_last_bucket = m_first_bucket;
    }
  }

public:
  /** \class Iterator BCList.h Puma/BCList.h
   * Bucket list sequential iterator. */
  class Iterator {
    friend class BCList<Item, BUCKET_SIZE> ;

    Item *m_item;
    Bucket *m_bucket;

    Iterator(Item *item, Bucket *bucket)
        : m_item(item), m_bucket(bucket) {
    }

  public:
    /** Construct an empty iterator. Do not use this iterator for 
     * other purposes than comparison. */
    Iterator()
        : m_item(0), m_bucket(0) {
    }
    /** Check whether this iterator is in a valid state. If the
     * iterator is not valid, it must not be used for other purposes
     * than comparison.
     * \return True if the iterator is valid. */
    operator bool() const {
      return m_item != 0;
    }
    /** Compare two iterators.
     * \param other The iterator to compare to.
     * \return True if both iterators point to same list item. */
    bool operator==(const Iterator& other) const {
      return m_item == other.m_item;
    }
    /** Compare two iterators.
     * \param other The iterator to compare to.
     * \return True if both iterators point to different list items. */
    bool operator!=(const Iterator &other) const {
      return !(*this == other);
    }
    /** Unary increment operator. Lets point the iterator to the next
     * item in the list.
     * \return A reference to this iterator pointing to the next item. */
    Iterator &operator++() {
      if (m_item != 0) {
        m_item++;
        if (m_item == m_bucket->pos()) {
          m_bucket = m_bucket->m_next;
          m_item = m_bucket ? m_bucket->first() : 0;
        }
      }
      return *this;
    }
    /** Binary increment operator. Lets point the iterator to the next
     * item in the list.
     * \return An new iterator pointing to the same item before the iterator was incremented. */
    Iterator operator++(int) {
      Iterator temp(*this);
      ++(*this);
      return temp;
    }
    /** Unary decrement operator. Lets point the iterator to the previous
     * item in the list.
     * \return A reference to this iterator pointing to the previous item. */
    Iterator &operator--() {
      if (m_item == m_bucket->first()) {
        m_bucket = m_bucket->m_prev;
        if (m_bucket) {
          m_item = m_bucket->pos();
          m_item--;
        } else {
          m_item = 0;
        }
      } else if (m_item != 0) {
        m_item--;
      }
      return *this;
    }
    /** Binary decrement operator. Lets point the iterator to the previous
     * item in the list.
     * \return An new iterator pointing to the same item before the iterator was decremented. */
    Iterator operator--(int) {
      Iterator temp(*this);
      --(*this);
      return temp;
    }
    /** Get the list item the iterator currently points to.
     * Ensure the iterator is valid before calling this method.
     * \return A reference to the current list item. */
    Item &operator*() const {
      return *m_item;
    }
    /** Get a pointer to the list item the iterator currently points to.
     * Ensure the iterator is valid before calling this method.
     * \return A pointer to the current list item. */
    Item *operator->() const {
      return m_item;
    }
  };

  /** Construct an empty bucket list. */
  BCList()
      : m_first_bucket(0), m_last_bucket(0) {
  }

  /** Destroy the bucket list. Will not deleted the list items. */
  ~BCList() {
    if (m_first_bucket != 0)
      delete m_first_bucket;
  }

  /** Get an iterator pointing to the beginning of the list.
   * \return A new iterator pointing to the first list item. */
  Iterator begin() {
    check();
    return Iterator(m_first_bucket->first(), m_first_bucket);
  }

  /** Get an iterator pointing to the end of the list. This iterator
   * does not point to the last list item. It is an empty iterator
   * that is intended to be used for comparisons like this:
   *
   * \code
   * BCList<int>::Iterator idx = list.begin();
   * for (; idx != list.end(); ++idx) {
   *   // do something
   * }
   * \endcode
   * \return A new empty iterator. */
  Iterator end() const {
    return Iterator();
  }

  /** Add an item to the list.
   * \param item Them item to add.
   * \return An iterator pointing to the new list item. */
  Iterator add(Item item) {
    check();
    // if the last bucket is full extend the bucket chain
    if (m_last_bucket->full())
      m_last_bucket = m_last_bucket->new_bucket();
    // the result is the position of the inserted item
    Item *result = m_last_bucket->pos();
    // add the item; the bucket is not full!
    m_last_bucket->add(item);
    // return an iterator for the new item
    return Iterator(result, m_last_bucket);
  }

  /** Reset the list. Will not delete the list items. */
  void reset() {
    if (m_first_bucket != 0) {
      m_first_bucket->clear();
      m_last_bucket = m_first_bucket;
    }
  }
};

} // namespace Puma

#endif /* PUMA_BCList_H */
