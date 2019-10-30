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

#ifndef __ClangWeaverBase_h__
#define __ClangWeaverBase_h__

// This class encapsulates all code manipulation or analysis functions
// that are needed by AspectC++, but which do not depend on AspectC++
// specific classes. They can be seen as a general purpose PUMA extension.

#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/CompilerInstance.h"

#include "ACModel/Elements.h"

#include "ACToken.h"
#include "ACIntroducer.h"
#include "ACProject.h"
#include <map>
#include <set>
#include <list>
#include <string>
#include "ClangAnnotation.h"

class LineDirectiveMgr;

class WeavePos {
public:
  // Attention: the enumerator order is important here ...
  enum Pos { WP_HEADER, WP_AFTER, WP_BEFORE, WP_FOOTER };
  int operator < (const WeavePos& key) const {
    return _loc == key._loc ? (_pos < key._pos) : (_loc < key._loc);
  }
  int operator == (const WeavePos& key) const {
    return _loc == key._loc && _pos == key._pos;
  }
private:
  friend class WeaverBase;
  WeavePos (clang::SourceLocation loc, Pos p) : _loc (loc), _pos (p) {}
  clang::SourceLocation _loc;
  Pos _pos;
  mutable std::list <std::string> _strings;
};


class WeaverBase {

  LineDirectiveMgr &_line_mgr;
  // These data structures are needed to store and find weaving positions that
  // were already used.
  typedef std::set<WeavePos> WPSet;
  WPSet _positions;

  // we need to store all delete (="kill") operations until we reached the commit.
  // Otherwise it wouldn't be possible to expand macros in which code was deleted.
  typedef std::pair<clang::SourceLocation, clang::SourceLocation> LocPair;
  typedef std::list<LocPair> LocPairList;
  LocPairList _deletions;

  typedef std::map<clang::SourceLocation, clang::SourceLocation> LocMap;
  LocMap _loc_map;
  struct MacroInfo {
    clang::FileID _fid;
    clang::SourceLocation _somewhere_inside;
    clang::SourceLocation _start;
    clang::SourceLocation _end;
  };
  typedef std::map<clang::SourceLocation, MacroInfo> MacroInfoMap;
  MacroInfoMap _seen_macros;
  bool _in_macro;
  typedef std::pair<std::string, clang::SourceLocation> TokenDescription;
  std::list<TokenDescription> _token_descriptions;
  clang::SourceLocation _last_expansion_loc;
  std::set<clang::SourceLocation> _macro_expansions;
  clang::Rewriter *_rewriter;
  ACProject &_project;
  AnnotationMap *_annotation_map;

  void commit_internal ();
  void resolve_macros ();
  void commit_kills();
  void commit_kill (clang::SourceLocation from, clang::SourceLocation to);


public:
  WeaverBase (ACProject &prj, LineDirectiveMgr &ldm) :
    _line_mgr (ldm), _in_macro (false), _project (prj), _annotation_map(0) {
    clang::SourceManager &SM = prj.get_compiler_instance ()->getSourceManager ();
    const clang::LangOptions &LO = prj.get_compiler_instance ()->getLangOpts();
    _rewriter = new clang::Rewriter (SM, LO);
  }

  // deal with C++11-style attributes in the token sequence
  // => store location information in a map
  // => transform AspectC++-attributes so that they can be parsed by clang
  // => make sure user-defined attributes are delete on 'commit'
  void set_annotation_map(AnnotationMap *annotation_map) { _annotation_map = annotation_map; }
  AnnotationMap &get_annotation_map() { return *_annotation_map; }
  void lex_and_filter_attr(clang::Preprocessor &PP, clang::Token &tok);

  // get the associate line directive manager
  LineDirectiveMgr &line_directive_mgr () const { return _line_mgr; }

  struct MacroAnalyzerState {
    bool _in_macro;
    std::list<TokenDescription> _token_descriptions;
    clang::SourceLocation _last_expansion_loc;
  };

  MacroAnalyzerState save_macro_analyzer_state () {
    MacroAnalyzerState mas = { _in_macro, _token_descriptions, _last_expansion_loc };
    _in_macro = false; _token_descriptions.clear(); _last_expansion_loc = clang::SourceLocation();
    return mas;
  }

  void restore_macro_analyzer_state (MacroAnalyzerState &mas) {
    _in_macro = mas._in_macro;
    _token_descriptions = mas._token_descriptions;
    _last_expansion_loc = mas._last_expansion_loc;
  }

  void macro_end ();

  void handle_token (clang::Preprocessor &PP, clang::Token &Result);

  ACProject &project () { return _project; }

  clang::Rewriter &getRewriter() { return *_rewriter; }

  const WeavePos &get_pos_after_loc (clang::SourceLocation loc,
      WeavePos::Pos pos = WeavePos::WP_AFTER);

  const WeavePos &weave_pos (ClangToken t, int p);

  const WeavePos &weave_pos (clang::SourceLocation loc, int p);

  const WeavePos &header_pos ();
  const WeavePos &header_pos (ACFileID fid);

  const WeavePos &footer_pos ();
  const WeavePos &footer_pos (ACFileID fid);

  // insert a string at the specified weaving position
  // 'before' decides whether the new text is inserted either before (true)
  // or after (false) any text that has been inserted earlier at this position.
  void insert (const WeavePos &pos, const std::string &str, bool before = false);

  // replace the text between two positions with some new text
  void replace (const WeavePos &from, const WeavePos &to,
      const std::string &str);

  // kill the text between two positions
  void kill (const WeavePos &from, const WeavePos &to);

  // commit a transformation transaction
  bool commit (clang::SourceManager &Target);

  // commit introductionunits into their buffers
  bool resolve_introduction_units(::ACIntroducer &intro);
};

#endif // __ClangWeaverBase_h__
