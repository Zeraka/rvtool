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

#ifndef __c_lexer_buffer_h__
#define __c_lexer_buffer_h__

#include "Puma/LexerBuffer.h"

namespace Puma {

class CLexerBuffer : public LexerBuffer {
public:
  CLexerBuffer () : LexerBuffer () {}
  CLexerBuffer (const char *str, int l = 0) : LexerBuffer (str, l) {}

  class ConstIterator : public LexerBuffer::ConstIterator {
    friend class CLexerBuffer;
    int _continuations;
    int _lines;

    void skip_nl () {
      if (*_ptr == '\\') {
        while (*_ptr != '\0' && *_ptr == '\\') {
          const char *p = _ptr + 1;
          if (*p != '\0' && *p == '\r') p++;
          if (*p != '\0' && *p == '\n') {
            _ptr = p + 1;
            _continuations++;
            _lines++;
          }
          else
            break;
        }
      }
    }
    void next () {
      if (*_ptr == '\n')
        _lines++;
      _ptr++;
      skip_nl ();
    }
  public:
    ConstIterator (const char *ptr = 0) :
      LexerBuffer::ConstIterator (ptr), _continuations (0), _lines (0) {}
    ConstIterator (const ConstIterator &r) :
      LexerBuffer::ConstIterator (r), _continuations (r._continuations), _lines (r._lines) {}
    ConstIterator &operator ++ () { next (); return *this; }
    ConstIterator operator ++ (int) { ConstIterator res(*this); next (); return res; }
    ConstIterator &adjust () { skip_nl (); return *this; }
    int continuations () const { return _continuations; }
    int lines () const { return _lines + 1; }
    void reset_counters () { _lines = 0; _continuations = 0; }
  };

  ConstIterator begin () const { return ConstIterator(_buffer); }
  ConstIterator end () const { return ConstIterator(_buffer ? _buffer + _buffer_size : 0); }
};

} // namespace Puma

#endif /* __c_lexer_buffer_h__ */
