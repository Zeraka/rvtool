// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2003  The 'ac++' developers (see aspectc.org)
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

#ifndef __IntroductionUnit_h__
#define __IntroductionUnit_h__

#include <string.h>

class ACM_Introduction;

#ifdef FRONTEND_PUMA
#include "ACUnit.h"

class IntroductionUnit : public ACUnit {
  Unit *_target_unit;
  int _nesting_level;
  int _precedence;
  bool _jp_needed;
  ACM_Introduction *_intro;

  void update_nesting_level () {
    IntroductionUnit *outer = cast (_target_unit);
    if (outer)
      _nesting_level = outer->_nesting_level + 1;
  }
public:
  IntroductionUnit (ACErrorStream &e, Unit *t = 0) : ACUnit (e), _target_unit (t),
    _nesting_level (0), _precedence (0), _jp_needed (false) {
    name ("intro");
    update_nesting_level ();
  }
  void target_unit (Unit *t) { _target_unit = t; update_nesting_level (); }
  Unit *target_unit () const { return _target_unit; }
  Unit *final_target_unit () const {
    Unit *result = _target_unit;
    const IntroductionUnit *intro_unit = 0;
    do {
      intro_unit = IntroductionUnit::cast (result);
      if (intro_unit)
        result = intro_unit->target_unit ();
    } while (intro_unit);
    return result;
  }
  int nesting_level () const { return _nesting_level; }
  int precedence () const { return _precedence; }
  void precedence (int p) { _precedence = p; }
  bool jp_needed () const { return _jp_needed; }
  void jp_needed (bool jpn) { _jp_needed = jpn; }
  void intro (ACM_Introduction *i) { _intro = i; }
  ACM_Introduction *intro () const { return _intro; }
  static IntroductionUnit *cast (const Unit* u) {
    return (u->name () && strcmp (u->name (), "intro") == 0) ? (IntroductionUnit*)u : 0;
  }
};

#else

#include "llvm/Support/MemoryBuffer.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/SourceLocation.h"

class IntroductionUnit {
  const llvm::MemoryBuffer *_target_unit; // FIXME: Could get this from _loc.
  clang::SourceLocation _target_loc;
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  const llvm::MemoryBuffer *_buffer;
#endif
  clang::FileID _fid;
  int _nesting_level;
  int _precedence;
  bool _jp_needed;
  ACM_Introduction *_intro;
  string _content;
  int _kind; // IU_OTHER, IU_BASE, IU_MEMBERS, or 0-N for non-inline member 'kind'

  void update_nesting_level () {
    IntroductionUnit *outer = cast (_target_unit);
    if (outer)
      _nesting_level = outer->_nesting_level + 1;
  }
public:
  enum { IU_OTHER = -3, IU_BASE = -2, IU_MEMBERS = -1 };
  IntroductionUnit (int kind) :
    _target_unit (0),
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    _buffer (0),
#endif
    _nesting_level (0), _precedence (0),
    _jp_needed (false), _intro (0), _kind(kind) {
  }
  bool is_base_intro () const { return _kind == IU_BASE; }
  bool is_members_intro () const { return _kind == IU_MEMBERS; }
  bool is_other_intro () const { return _kind == IU_OTHER; }
  int non_inline_member_no () const { return _kind; } // < 0: error
  void target_unit(const llvm::MemoryBuffer *t) {
    _target_unit = t;
    update_nesting_level();
  }
  const llvm::MemoryBuffer *target_unit () const { return _target_unit; }
  const llvm::MemoryBuffer *final_target_unit () const {
    const llvm::MemoryBuffer *result = _target_unit;
    const IntroductionUnit *intro_unit = 0;
    do {
      intro_unit = IntroductionUnit::cast (result);
      if (intro_unit)
        result = intro_unit->target_unit ();
    } while (intro_unit);
    return result;
  }
  void location (clang::SourceLocation t) { _target_loc = t; }
  clang::SourceLocation location () const { return _target_loc; }
  void content (const string &str) { _content = str; }
  const string &content () const { return _content; }
  // create memory buffer on demand from _content string
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  const llvm::MemoryBuffer *buffer () {
    if (!_buffer) {
      std::ostringstream name;
      name << "<intro:" << (void *)this << '>';
      _buffer = llvm::MemoryBuffer::getMemBufferCopy(_content, name.str());
    }
    return _buffer;
#else // C++ 11 interface
  std::unique_ptr<llvm::MemoryBuffer> buffer () {
    std::ostringstream name;
    name << "<intro:" << (void *)this << '>';
//    cout << "name: " << name.str () << endl << _content << endl << "--" << endl;
    return llvm::MemoryBuffer::getMemBufferCopy(_content, name.str());
#endif
  }
  void file_id(clang::FileID t) { _fid = t; }
  clang::FileID file_id () const { return _fid; }
  int nesting_level () const { return _nesting_level; }
  int precedence () const { return _precedence; }
  void precedence (int p) { _precedence = p; }
  bool jp_needed () const { return _jp_needed; }
  void jp_needed (bool jpn) { _jp_needed = jpn; }
  void intro (ACM_Introduction *i) { _intro = i; }
  ACM_Introduction *intro () const { return _intro; }
  static IntroductionUnit *cast (const llvm::MemoryBuffer *buf) {
    const char *name = buf->getBufferIdentifier();
    if (!strncmp(name, "<intro:", 7)) {
      // We can't use istringstream here because libc++'s implementation is
      // buggy http://llvm.org/PR19740.
      void *ptr;
      sscanf(name + 7, "%p", &ptr);
      return (IntroductionUnit *)ptr;
    }
    return 0;
  }
};
#endif

#endif // __IntroductionUnit_h__

