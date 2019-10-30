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

// AspectC++ includes
#include "WeaverBase.h"
#include "ACUnit.h"
#include "LineDirectiveMgr.h"

// PUMA includes
#include "Puma/ErrorSink.h"
#include "Puma/CTree.h"
#include "Puma/MacroUnit.h"

using namespace Puma;

const WeavePos &WeaverBase::weave_pos (Token *t, WeavePos::Pos p) {

  // check whether the weaving position has already been created
  WPSet::iterator i = _positions.find (WeavePos (t, p));
  if (i != _positions.end ())
    return *i;

  // create an insert a marker token
  Token *marker_token = new Token; // just a marker token without text etc.
  marker_token->location (t->location ());
  Unit *marker_unit = new Unit;
  marker_unit->append (*marker_token);
  _commander.addBuffer (marker_unit);
  if (p == WeavePos::WP_HEADER) {
    // check whether there is already a marker token in front of the first token
    Token *marker_inspos = t;
    WPSet::iterator i = _positions.find (WeavePos (t, WeavePos::WP_BEFORE));
    if (i != _positions.end ())
      marker_inspos = (*i)._marker;
    _commander.move_before (marker_token, marker_token, marker_inspos);
  }
  else if (p == WeavePos::WP_BEFORE)
    _commander.move_before (marker_token, marker_token, t);
  else if (p == WeavePos::WP_FOOTER) {
    // check whether there is already a marker token behind the last token
    Token *marker_inspos = t;
    WPSet::iterator i = _positions.find (WeavePos (t, WeavePos::WP_AFTER));
    if (i != _positions.end ())
      marker_inspos = (*i)._marker;
    _commander.move (marker_token, marker_token, marker_inspos);
  }
  else
    _commander.move (marker_token, marker_token, t);

  // insert the position as a new one
  pair<WPSet::iterator, bool> result =
      _positions.insert (WeavePos (t, p, marker_token));

  // the resulting iterator points to the new entry
  return *result.first;
}

// get the 'header' weaving position in front of a unit
const WeavePos &WeaverBase::header_pos (ACFileID fid) {
  const Unit *unit = fid.file_entry ();
  Token *first = (Token*)unit->first ();
  if (unit == (Unit*)_primary_start->belonging_to ())
    first = _primary_start;
  return weave_pos (first, WeavePos::WP_HEADER);
}

// get the 'header' weaving position in front of the primary unit
const WeavePos &WeaverBase::header_pos () {
  return weave_pos (_primary_start, WeavePos::WP_HEADER);
}

// get the 'footer' weaving position at the end of an arbitrary unit
const WeavePos &WeaverBase::footer_pos (ACFileID fid) {
  const Unit *unit = fid.file_entry ();
  Token *last = (Token*)unit->last ();
  if (unit == (Unit*)_primary_end->belonging_to ())
    last = _primary_end;
  return weave_pos (last, WeavePos::WP_FOOTER);
}

// get the 'footer' weaving position at the end of the primary unit
const WeavePos &WeaverBase::footer_pos () {
  return weave_pos (_primary_end, WeavePos::WP_FOOTER);
}

// insert a unit at the specified weaving position
// 'before' selects whether the new text is inserted either before (true)
// or after (false) any text that has been inserted earlier at this position.
void WeaverBase::insert (const WeavePos &pos, Unit *unit, bool before) {
  // return immediately if the there is a problem with macro manipulation
  if (macro_problem (pos))
    return;

  // store the unit in the right order for insertion (later when committing)
  if (before)
    pos._units.push_front(unit);
  else
    pos._units.push_back(unit);
}

// the same as the other 'insert' method (above), but converts string to unit
void WeaverBase::insert (const WeavePos &pos, const string &str, bool before) {
  // return immediately if the string to paste is empty
  if (str == "")
    return;

  ACUnit *unit = new ACUnit (_err);
  bool has_nl = (str.find('\n') != string::npos);
  if (has_nl)
    _line_mgr.directive (*unit);
  *unit << str.c_str ();
  if (has_nl)
    _line_mgr.directive (*unit, (Unit*)pos._token->belonging_to (), PumaToken(pos._token));
  *unit << endu;
  insert (pos, unit, before);
}


// replace the text between two positions with some new text
void WeaverBase::replace (const WeavePos &from, const WeavePos &to,
                                             const string &str) {
  insert (from, str);
  kill (from, to);
}


// check whether two tokens are on the same line of the same unit
bool same_line (Token *t1, Token *t2, bool check_end_of_t1) {
  if (t1->belonging_to() != t2->belonging_to())
    return false;
  int l1 = t1->location().line();
  if (check_end_of_t1)
    l1 += t1->line_breaks();
  int l2 = t2->location().line();
  return (l1 == l2);
}


// kill the text between two positions
void WeaverBase::kill (const WeavePos &from, const WeavePos &to) {
  // TODO: this should not matter, the code shoud be able to deal with it
  assert (from._pos == WeavePos::WP_BEFORE);
  assert (to._pos == WeavePos::WP_AFTER);
  if (macro_problem (from) || macro_problem (to))
    return;

  // TODO: why is only the first token checked?
  bool has_nl = (string (from._token->text ()).find('\n') != string::npos);
  if ((to._token && from._token != to._token) || has_nl)
    _line_mgr.insert ((Unit*)to._token->belonging_to (), PumaToken(to._token), true);
  _commander.kill (from._token, to._token);
}

// kill the text of a syntax tree
void WeaverBase::kill (CTree *node) {
  const WeavePos &from = weave_pos (node->token (), WeavePos::WP_BEFORE);
  const WeavePos &to   = weave_pos (node->end_token (), WeavePos::WP_AFTER);
  kill (from, to);
}

bool WeaverBase::commit () {
  // handle code that has to be inserted at weaving positions
  for (WPSet::iterator i = _positions.begin (); i != _positions.end (); ++i) {
    const WeavePos &wp = *i;
    for (list<Unit*>::iterator ui = wp._units.begin (); ui != wp._units.end (); ++ui) {
      Unit *unit = *ui;
      // TODO: handle line directives here (to avoid too many of them)
      _commander.addBuffer (unit);
      Token *first = (Token*)unit->first ();
      Token *last  = (Token*)unit->last ();
      // "move_before" preserves the order
      _commander.move_before (first, last, wp._marker);
    }
  }

  ManipError terror = _commander.valid ();
  if (!terror)
    _commander.commit ();
  else
    _err << terror << endMessage;

  // clear the list of weaving positions used for this transaction
  _positions.clear();

  return (bool)terror;
}

bool WeaverBase::macro_problem (const WeavePos &pos) {
  Token *token = pos._token;
  Unit  *unit  = (Unit*)token->belonging_to ();
  assert (unit);
  if (unit->isMacroExp () &&
      ((pos._pos == WeavePos::WP_BEFORE &&
        unit->isMacroExp () && !((MacroUnit*)unit)->ExpansionBegin (token)) ||
       (pos._pos == WeavePos::WP_AFTER &&
           unit->isMacroExp () && !((MacroUnit*)unit)->ExpansionEnd (token)))) {
      if (_warn_macro_expansion) {
        _err << sev_warning << token->location ()
             << "transformation within macro '"
             << ((MacroUnit*)unit)->MacroBegin ()->text () << "'" << endMessage;
      }
//      return true;
      return false;
  }
  return false;
}

int WeaverBase::primary_len() const {
  return _primary_end->location().line() - _primary_start->location().line() + 1;
}

