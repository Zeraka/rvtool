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

#ifndef __PumaWeaverBase_h__
#define __PumaWeaverBase_h__

// This class encapsulates all code manipulation or analysis functions
// that are needed by AspectC++, but which do not depend on AspectC++
// specific classes. They can be seen as a general purpose PUMA extension.

#include <set>
#include <string>
using namespace std;

#include "Puma/ManipCommander.h"

#include "ACToken.h"
#include "ACUnit.h"
#include "ACFileID.h"
#include "ACErrorStream.h"
#include "ACProject.h"
#include "BackEndProblems.h"

namespace Puma {
  class CTree;
} // namespace Puma

class WeaverBase;
class LineDirectiveMgr;

class WeavePos {
public:
  enum Pos { WP_HEADER, WP_AFTER, WP_BEFORE, WP_FOOTER };
  int operator < (const WeavePos& key) const {
    return _token == key._token ?
      (_pos < key._pos) : (_token < key._token);
  }
  int operator == (const WeavePos& key) const {
    return _token == key._token && _pos == key._pos;
  }
private:
  friend class WeaverBase;
  WeavePos (Puma::Token *t, Pos p, Puma::Token *m = 0) : _token (t), _pos (p), _marker (m) {}
  Puma::Token *_token;
  Pos _pos;
  Puma::Token *_marker;
  mutable list <Puma::Unit*> _units;
};

class WeaverBase {

  // These data structures are needed to store and find weaving positions that
  // were already used.
  typedef set<WeavePos> WPSet;
  WPSet _positions;

  // This Puma class/object is needed to perform the actual transformations
  Puma::ManipCommander _commander;

  ACProject &_project;

protected:
  ACErrorStream &_err;
  LineDirectiveMgr &_line_mgr;
  bool _warn_macro_expansion;
  Puma::Token *_primary_start;
  Puma::Token *_primary_end;

  // check if a code manipulation at the given position would lead to an
  // automatic macro expansion; warn if configured to do so
  bool macro_problem (const WeavePos &pos);

public:
  WeaverBase (ACProject &prj, LineDirectiveMgr &ldm) :
    _project (prj), _err (prj.err ()), _line_mgr (ldm),
    _warn_macro_expansion(false), _primary_start(0), _primary_end(0) {
    _commander.manip_mode (Puma::MM_EXPAND_MACROS);
  }

  ACProject &project () { return _project; }

  // setup first and last token of the translation unit
  void init (Puma::Token *s, Puma::Token *e) {
    _primary_start = s; _primary_end = e;
  }

  // return the length of the primary translation unit file as defined
  // by the start and end tokens passed to 'init'
  int primary_len () const;

  // get the associate line directive manager
  LineDirectiveMgr &line_directive_mgr () const { return _line_mgr; }

  // functions to retrieve weaving positions:

  // get a weaving position before or after a specific token
  const WeavePos &weave_pos (Puma::Token *t, WeavePos::Pos p);
  const WeavePos &weave_pos (PumaToken t, WeavePos::Pos p) {
    return weave_pos(t.get(), p);
  }
  // get the 'header' weaving position in front of an arbitrary unit
  const WeavePos &header_pos (ACFileID fid);
  // get the 'header' weaving position in front of the primary unit
  const WeavePos &header_pos ();
  // get the 'footer' weaving position at the end of an arbitrary unit
  const WeavePos &footer_pos (ACFileID fid);
  // get the 'footer' weaving position at the end of the primary unit
  const WeavePos &footer_pos ();

  // code manipulation functions:

  // check/ignore manipulations for unbalanced preprocessor directives
  void ignore_unbalanced () { _commander.ignore_mask (Puma::MIM_UNBALANCED); }
  void check_unbalanced () { _commander.ignore_mask (Puma::MIM_NONE); }

  // insert a unit at the specified weaving position
  // 'before' decides whether the new text is inserted either before (true)
  // or after (false) any text that has been inserted earlier at this position.
  void insert (const WeavePos &pos, Puma::Unit *unit, bool before = false);

  // the same as the other 'insert' method (above), but converts string to unit
  void insert (const WeavePos &pos, const string &str, bool before = false);

  // replace the text between two positions with some new text
  void replace (const WeavePos &from, const WeavePos &to, const string &str);

//  // replace the text of a syntax tree with some new text
//  void replace (CTree *node, const string &str);

  // kill the text between two positions
  void kill (const WeavePos &from, const WeavePos &to);

  // kill the text of a syntax tree
  void kill (Puma::CTree *node);

  // commit a transformation transaction
  bool commit ();
};

#endif // __PumaWeaverBase_h__
