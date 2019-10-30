// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2013  The 'ac++' developers (see aspectc.org)
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


#ifndef __ClangErrorStream_h__
#define __ClangErrorStream_h__

#include "clang/Basic/SourceManager.h"
#include "Puma/ErrorStream.h"

// FIXME: Decouple from Puma.
class ClangErrorStream : public Puma::ErrorStream {
  clang::SourceManager *_sm;
public:
  ClangErrorStream (clang::SourceManager *sm = 0,
                    std::ostream &s = std::cerr) : ErrorStream(s), _sm(sm) {}
  void set_source_manager (clang::SourceManager *sm) { _sm = sm; }
  clang::SourceManager &get_source_manager () const { return *_sm; }

  ClangErrorStream &operator << (clang::SourceLocation l) {
    assert (_sm && "No SourceManager!");
    clang::PresumedLoc PL = _sm->getPresumedLoc(l);
    *this << PL.getFilename() << ':' << PL.getLine() << ':' << PL.getColumn()
          << ": ";

    return *this;
  }

  ClangErrorStream &operator << (clang::FullSourceLoc fl) {
    clang::PresumedLoc PL = fl.getManager ().getPresumedLoc(fl);
    *this << PL.getFilename() << ':' << PL.getLine() << ':' << PL.getColumn()
          << ": ";

    return *this;
  }
  ClangErrorStream &operator << (const char *x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (long x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (unsigned x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (int x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (short x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (char x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (double x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (const Puma::ErrorSeverity & x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (Puma::Location x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (const Puma::Printable &x) {
    ErrorStream::operator << (x);
    return *this;
  }
  ClangErrorStream &operator << (void (*f)(Puma::ErrorSink &)) {
    ErrorStream::operator << (f);
    return *this;
  }
};

#endif // __ClangErrorStream_h__
