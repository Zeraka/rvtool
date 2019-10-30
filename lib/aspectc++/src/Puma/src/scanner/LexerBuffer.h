// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#ifndef __lexer_buffer_h__
#define __lexer_buffer_h__

#include <algorithm>

namespace Puma {

class LexerBuffer {

protected:
  const char *_buffer; // buffer that contains all input characters
  int  _buffer_size;     // size of the input buffer

public:  
  LexerBuffer () : _buffer(0), _buffer_size(0) {}
  LexerBuffer (const char *str, int l = 0) { init (str, l); }

  // init buffer to directly scan from a string
  void init (const char *str, int l = 0) {
    _buffer = str;
    _buffer_size = l;
    if (!l) while (_buffer[_buffer_size] != '\0') _buffer_size++;
  }
  
  // an iterator class
  class ConstIterator {
    friend class LexerBuffer;
  protected:
    const char *_ptr;
  public:
    typedef std::forward_iterator_tag iterator_category;
    typedef std::size_t difference_type;
    typedef char value_type;
    typedef char *pointer;
    typedef char &reference;


    ConstIterator (const char *ptr = 0) : _ptr (ptr) {}
    ConstIterator (const ConstIterator &r) : _ptr (r._ptr) {}
    ConstIterator &operator = (const ConstIterator &r) { _ptr = r._ptr; return *this; }
    bool operator == (const ConstIterator &r) const { return _ptr == r._ptr; }
    bool operator != (const ConstIterator &r) const { return _ptr != r._ptr; }
    ConstIterator &operator ++ () { _ptr++; return *this; }
    ConstIterator operator ++ (int) { ConstIterator res(*this); _ptr++; return res; }
    const char &operator * () const { return *_ptr; }
  };

  ConstIterator begin () const { return ConstIterator(_buffer); }
  ConstIterator end () const { return ConstIterator(_buffer ? _buffer + _buffer_size : 0); }
};


} // namespace Puma

#endif /* __lexer_buffer_h__ */
