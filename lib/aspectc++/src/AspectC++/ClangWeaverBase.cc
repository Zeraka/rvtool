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
// MA  02111-1307  USA

#include "ClangWeaverBase.h"
#include "ACIntroducer.h"
#include "IntroductionUnit.h"
#include "LineDirectiveMgr.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "llvm/Support/MemoryBuffer.h"
#include "ACModel/Utils.h"

// Returns the location one past the end of the token.
static clang::SourceLocation getLocAfterToken(ClangToken tok) {
  unsigned len = tok.get ().getLength ();
  return tok.location ().getLocWithOffset (len);
}

const WeavePos &WeaverBase::get_pos_after_loc (clang::SourceLocation loc,
    WeavePos::Pos pos) {
  clang::SourceLocation result;
  if (loc.isValid() && loc.isMacroID ()) {
    clang::SourceLocation macro_end_loc;
    // See if we're at the end of a macro and get the location for the last
    // token if we are.
    bool is_at_end = clang::Lexer::isAtEndOfMacroExpansion(loc,
        getRewriter().getSourceMgr(), getRewriter().getLangOpts (), &macro_end_loc);
    if (is_at_end) {
      loc = macro_end_loc;
      // Now go on and get the location after this token.
    }
    clang::SourceLocation spell_loc = getRewriter().getSourceMgr().getSpellingLoc(loc);
    unsigned len = clang::Lexer::MeasureTokenLength(spell_loc, getRewriter().getSourceMgr(),
        getRewriter().getLangOpts ());
    result = loc.getLocWithOffset(len);
  }
  else {
    result = clang::Lexer::getLocForEndOfToken (loc, 0,
        getRewriter().getSourceMgr(), getRewriter().getLangOpts ());
  }

  assert (!result.isInvalid ());
  return weave_pos (result, pos);
}

const WeavePos &WeaverBase::weave_pos (ClangToken t, int p) {
  if (p == (int)WeavePos::WP_AFTER)
    return weave_pos (getLocAfterToken(t), p);
  else
    return weave_pos (t.location(), p);
}

const WeavePos &WeaverBase::weave_pos (clang::SourceLocation loc, int p) {
  assert(loc.isValid() && "Invalid SourceLocation!");

  // Optimization: If a location is directly before or after a macro we can turn
  // it into a non-macro location and avoid the extra work for macro expansion.
  // TODO: This is currently disabled because clang returns wrong locations for
  // some macro arguments.
  /*if (loc.isValid() && loc.isMacroID()) {
    clang::SourceManager &SM = _rewriter->getSourceMgr();
    clang::SourceLocation fixed_loc;
    if (SM.isAtStartOfImmediateMacroExpansion (loc, &fixed_loc))
      loc = fixed_loc;
    else if (SM.isAtEndOfImmediateMacroExpansion (loc, &fixed_loc))
      loc = fixed_loc;
  }*/

  // check whether the weaving position has already been created
  WPSet::iterator i = _positions.find (WeavePos (loc, (WeavePos::Pos)p));
  if (i != _positions.end ())
    return *i;

  // insert the position as a new one
  pair<WPSet::iterator, bool> result =
      _positions.insert (WeavePos (loc, (WeavePos::Pos)p));

  // the resulting iterator points to the new entry
  return *result.first;
}

const WeavePos &WeaverBase::header_pos () {
  clang::FileID main_fid = _rewriter->getSourceMgr().getMainFileID();
  return header_pos (_rewriter->getSourceMgr().getFileEntryForID (main_fid));
}

const WeavePos &WeaverBase::header_pos (ACFileID acfid) {
  const clang::FileEntry *fentry = acfid.file_entry ();
  clang::FileID fid = _rewriter->getSourceMgr().translateFile (fentry);
  clang::SourceLocation first =
      _rewriter->getSourceMgr().getLocForStartOfFile(fid);
  return weave_pos (first, WeavePos::WP_HEADER);
}

const WeavePos &WeaverBase::footer_pos () {
  clang::FileID main_fid = _rewriter->getSourceMgr().getMainFileID();
  return footer_pos (_rewriter->getSourceMgr().getFileEntryForID (main_fid));
}

const WeavePos &WeaverBase::footer_pos (ACFileID acfid) {
  const clang::FileEntry *fentry = acfid.file_entry ();
  clang::FileID fid = _rewriter->getSourceMgr().translateFile (fentry);
  clang::SourceLocation last =
      _rewriter->getSourceMgr().getLocForEndOfFile(fid);
  return weave_pos (last, WeavePos::WP_FOOTER);
}

// insert a generated string a given position
void WeaverBase::insert (const WeavePos &pos, const string &str,
                              bool before) {
  // return immediately if the string to paste is empty
  if (str == "")
    return;

  // If this is a bulk code insert wrap it in line directives.
  if (str.find('\n') != string::npos) {
    clang::PresumedLoc loc = _rewriter->getSourceMgr().getPresumedLoc(pos._loc);

    // <ac> directives are fixed up later on.
    std::ostringstream os;
    _line_mgr.directive (os, clang::PresumedLoc ());
    os << str;
    _line_mgr.directive (os, loc);

    if (before)
      pos._strings.push_front(os.str());
    else
      pos._strings.push_back(os.str());
    return;
  }

  // FIXME: handle before

  // store the unit in the right order for insertion (later when committing)
  if (before)
    pos._strings.push_front(str);
  else
    pos._strings.push_back(str);
}

// replace the text between two positions with some new text
void WeaverBase::replace (const WeavePos &from, const WeavePos &to,
    const string &str) {
  insert (from, str);
  kill (from, to);
}

void WeaverBase::macro_end () {
  clang::SourceManager &SM = _rewriter->getSourceMgr();

  // build the full string
  string expansion;
  for (list<TokenDescription>::iterator i = _token_descriptions.begin();
      i != _token_descriptions.end(); ++i)
    expansion += i->first + " ";
//  cout << "macro " << expansion << endl;

  // create a memory buffer for the macro instance and register it
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  llvm::MemoryBuffer *mb =
      llvm::MemoryBuffer::getMemBufferCopy(expansion, "<macro-expansion>");
  clang::FileID fid =
      SM.createFileIDForMemBuffer(mb, clang::SrcMgr::C_User, 0, 0, _last_expansion_loc);
#else // C++ 11 interface
  std::unique_ptr<llvm::MemoryBuffer> mb (llvm::MemoryBuffer::getMemBufferCopy(expansion, "<macro-expansion>"));
  clang::FileID fid =
      SM.createFileID(std::move (mb), clang::SrcMgr::C_User, 0, 0, _last_expansion_loc);
#endif

  // calculate the actual position mapping and update it
  unsigned offset = 0;
  clang::SourceLocation start = SM.getLocForStartOfFile(fid);
  clang::SourceLocation last = start;
  for (list<TokenDescription>::iterator i = _token_descriptions.begin();
      i != _token_descriptions.end(); ++i) {
    clang::SourceLocation mapped_loc = start.getLocWithOffset(offset);
    LocMap::iterator map_entry = _loc_map.find (i->second);
    if (map_entry == _loc_map.end ())
      cout << "fatal error: location of macro token not found" << endl;
    else
      map_entry->second = mapped_loc;
    offset += i->first.size();
    clang::SourceLocation end_loc = (i->second).getLocWithOffset(i->first.size());
    mapped_loc = start.getLocWithOffset(offset);
    last = mapped_loc;
    _loc_map.insert(pair<clang::SourceLocation, clang::SourceLocation>(end_loc, mapped_loc));
    offset += 1; // " for the whitespace after each token in the string
  }

  // store general infos about this 'seen' macro
  MacroInfo mi = { fid, _token_descriptions.front().second, start, last };
  _seen_macros.insert (pair<clang::SourceLocation, MacroInfo> (_last_expansion_loc, mi));
}

extern "C" void __real__ZN5clang12Preprocessor3LexERNS_5TokenE(clang::Preprocessor &that, clang::Token &Result);

// make sure that Clang parses attributes, even if we are not in C++11-mode
void WeaverBase::lex_and_filter_attr(clang::Preprocessor &PP, clang::Token &result) {

  static clang::LangOptions &lang_options = _project.get_compiler_instance()->getLangOpts ();
  static clang::LangOptions original_lang_options = lang_options;
  static clang::SourceLocation prev_location;
  static std::set<clang::SourceLocation> hidden;
  static bool in_attr = false;

  do {
    __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, result);
    if (result.is(clang::tok::eof) || !_annotation_map)
      return;
  } while (hidden.find(result.getLocation()) != hidden.end());

//  cout << "Got token: " << PP.getSpelling(result) << endl;
  clang::SourceManager &SM = _rewriter->getSourceMgr();
  if (prev_location.isValid() && !SM.isBeforeInTranslationUnit (prev_location, result.getLocation()))
    return;
  prev_location = result.getLocation();

//  cout << "New token: " << PP.getSpelling(result) << result.getLocation().printToString(SM) << endl;
  if ((!in_attr && result.isNot(clang::tok::l_square)) || (in_attr && result.isNot(clang::tok::r_square)))
    return;

  PP.EnableBacktrackAtThisPos();
  clang::Token tok;
  __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
  if (!in_attr && tok.is(clang::tok::l_square)) {
    // here we have seen "[[": an attribute sequence starts
    lang_options.CPlusPlus11 = 1;
    in_attr = true;
    hidden.clear();
    // parse the attribute list
    std::list<AnnotationMap::iterator> annotation_iters;
    bool in_seq = true;
    bool is_user_defined_seq = true;
    while (in_seq) {
      // create an initialize an attribute object
      Annotation attr;
      attr.seqBegin = prev_location;
      // parse an attribute, start with the qualified name
      std::vector<clang::Token> tokens;
      while (true) {
        __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
        if (tok.isNot(clang::tok::identifier))
          break;
        tokens.push_back(tok);
        attr.attrNames.push_back(PP.getSpelling(tok));
        __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
        if (tok.isNot(clang::tok::coloncolon))
          break;
        tokens.push_back(tok);
      };
      bool is_user_defined = (attr.attrNames.size() > 1 &&
          attr.attrNames[0] != "gnu" && attr.attrNames[0] != "clang");
      attr.is_user_defined = is_user_defined;
      if (is_user_defined) {
        for (unsigned int i = 1; i < tokens.size() - 2; i++) {
          hidden.insert(tokens[i].getLocation()); // hide the middle part so that clang can parse it
        }
      }
      else
        is_user_defined_seq = false;
      attr.tokBegin = tokens[0].getLocation();
      if (tok.is(clang::tok::l_paren)) {
        unsigned paren_count = 1;
        do {
          __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
          if (tok.is(clang::tok::l_paren))
            paren_count++;
          else if (tok.is(clang::tok::r_paren))
            paren_count--;
          else {
            if (!attr.params.empty())
              attr.params += " ";
            attr.params += PP.getSpelling(tok);
          }
          // TODO: error handling
        } while (paren_count > 0);
        __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
      }
      if (tok.isNot(clang::tok::comma))
        in_seq = false;
      attr.tokEnd = tok.getLocation(); // end is begin of next token => better for deleting the attribute
      std::pair<AnnotationMap::iterator,bool> ret =
          _annotation_map->insert(std::pair<clang::SourceLocation, Annotation>(attr.tokBegin, attr));
      annotation_iters.push_back (ret.first);
    }

    if (tok.is(clang::tok::r_square)) {
      clang::SourceLocation end = tok.getLocation();
      __real__ZN5clang12Preprocessor3LexERNS_5TokenE(PP, tok);
      if (tok.is(clang::tok::r_square)) {
        if (is_user_defined_seq) {
          kill(weave_pos(prev_location, WeavePos::WP_BEFORE),
              weave_pos(tok.getLocation().getLocWithOffset(1), WeavePos::WP_AFTER));
        }
        else {
          clang::SourceLocation begin = end;
          bool have_builtin = false;
          for (std::list<AnnotationMap::iterator>::reverse_iterator i = annotation_iters.rbegin();
              i != annotation_iters.rend(); ++i) {
            if ((*i)->second.is_user_defined) {
              begin = (*i)->second.tokBegin;
            }
            else {
              if (!have_builtin)
                begin = (*i)->second.tokEnd; // remove trailing ','
              if (begin != end)
                kill(weave_pos(begin, WeavePos::WP_BEFORE), weave_pos(end, WeavePos::WP_AFTER));
              begin = end = (*i)->second.tokBegin;
              have_builtin = true;
            }
          }
          if (begin != end)
            kill(weave_pos(begin, WeavePos::WP_BEFORE), weave_pos(end, WeavePos::WP_AFTER));
        }
      }
    }
  }
  else if (in_attr && tok.is(clang::tok::r_square)) {
    lang_options = original_lang_options;
    in_attr = false;
//    cout << "in_attr " << in_attr << " at " << tok.getLocation().printToString(SM) << endl;
  }
  PP.Backtrack();
  return;
}


void WeaverBase::handle_token (clang::Preprocessor &PP, clang::Token &Result) {

  clang::SourceManager &SM = _rewriter->getSourceMgr();

  // ignore introduced tokens: we might still be in the macro
  string loc_str = SM.getBufferName(Result.getLocation ());
//  cout << "tok: " << Result.getName() << endl;
//  cout << "loc: " << loc_str << " " << Result.getLocation().isMacroID() << " "<< PP.getSpelling(Result) << endl;
  if (loc_str.substr(0, 7) == "<intro:" ||
      loc_str == "<tmp>" ||
      loc_str == "<intro-includes>")
    return;

  // full macro found if this is a non-macro token
  if (!Result.getLocation().isMacroID()) {
    if (_in_macro) {
    // end of macro reached
      macro_end ();
      _in_macro = false;
    }
    return;
  }

  // handle macro token
  clang::SourceLocation loc = Result.getLocation();
  clang::SourceLocation expansion_loc = SM.getExpansionLoc(loc);

  // filter out macro tokens in macros that we have already handled
  if (_seen_macros.find (expansion_loc) != _seen_macros.end ())
    return;

  // filter out token that are read a second time because of look ahead
  static bool skipping = false;
  if (_loc_map.find (Result.getLocation()) != _loc_map.end()) {
    // Special handling for stringified macro arguments: Clang always uses
    // uses the same token object for all (different) instances of a
    // stringified argument. This means that the mustn't skip it when the
    // token is seen more than once. However, it's not possible to clearly
    // distinguish between a lookahead/reparsing of these tokens and a repeated
    // use within a macro. Therefore, we have to guess. :-( Iff the token before
    // was skipped, we skip a repeated stringified argument token as well.
    //
    // Function stringifiedInMacro() is not available in Clang 3.4
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    return;
#else
    if (!Result.stringifiedInMacro() || skipping) {
      skipping = true;
      return;
    }
#endif
  }
  skipping = false;

  if (_in_macro && expansion_loc != _last_expansion_loc) {
    // end of macro reached
    macro_end();
    _in_macro = false;
  }

  _last_expansion_loc = expansion_loc;
  if (!_in_macro) {
    // begin of macro reached
    _in_macro = true;
    _token_descriptions.clear ();
  }

  // handle macro token
  _token_descriptions.push_back (TokenDescription(PP.getSpelling(Result), loc));

  clang::SourceLocation new_loc = loc; // for now, replace later
  _loc_map.insert (pair<clang::SourceLocation, clang::SourceLocation>(loc, new_loc));
}


// kill the text between two positions
void WeaverBase::kill (const WeavePos &from, const WeavePos &to) {
  // TODO: this should not matter, the code shoud be able to deal with it
  assert (from._pos == WeavePos::WP_BEFORE);
  assert (to._pos == WeavePos::WP_AFTER);

  _deletions.push_back(LocPair(from._loc, to._loc));
}


void WeaverBase::commit_kills() {
  for (LocPairList::iterator i = _deletions.begin(); i != _deletions.end(); ++i)
    commit_kill (i->first, i->second);
}


// kill the text between two positions
void WeaverBase::commit_kill (clang::SourceLocation from, clang::SourceLocation to) {

  /*if (macro_problem (from) || macro_problem (to))
    return;

  bool has_nl = (strstr (from._token.text (), "\n") != NULL);
  if ((to._token && from._token != to._token) || has_nl)
    _line_mgr.insert ((Unit*)to._token.unit (), to._token, true);*/

  clang::Rewriter::RewriteOptions opts;
  opts.IncludeInsertsAtBeginOfRange = false;
  opts.IncludeInsertsAtEndOfRange   = false;
  clang::SourceManager &SM = _rewriter->getSourceMgr();

  // Handle kills inside a macro.

  if (from.isMacroID() || to.isMacroID()) {

    clang::SourceLocation loc1 = from;
    clang::SourceLocation expansion_loc1 = SM.getExpansionLoc(loc1);

    if (from.isMacroID()) {
      LocMap::iterator i = _loc_map.find(from);
      if (i == _loc_map.end()) {
        cout << "fatal: 'from' location not found during kill" << endl;
        return;
      }
      loc1 = i->second;
    }

    clang::SourceLocation loc2 = to;
    clang::SourceLocation expansion_loc2 = SM.getExpansionLoc(loc2);
    if (to.isMacroID()) {
      LocMap::iterator i = _loc_map.find(to);
      if (i == _loc_map.end()) {
        cout << "fatal: 'to' location not found during kill" << endl;
        return;
      }
      loc2 = i->second;
    }

    if (from.isMacroID() && !to.isMacroID()) {
      MacroInfoMap::iterator macro_info = _seen_macros.find(expansion_loc1);
      if (macro_info == _seen_macros.end ()) {
        cout << "fatal: macro info not found in kill" << endl;
        return;
      }
      clang::SourceLocation macro_end = macro_info->second._end;
      _rewriter->RemoveText(clang::CharSourceRange::getCharRange(loc1, macro_end));

      clang::SourceLocation somewhere_inside = macro_info->second._somewhere_inside;
      std::pair<clang::SourceLocation, clang::SourceLocation> range =
          SM.getExpansionRange(somewhere_inside);
      unsigned len = _rewriter->getRangeSize(clang::SourceRange(range.second, range.second));
      _rewriter->RemoveText(clang::CharSourceRange::getCharRange(
          range.second.getLocWithOffset(len), loc2), opts);
    }
    else if (!from.isMacroID() && to.isMacroID()) {
      MacroInfoMap::iterator macro_info = _seen_macros.find(expansion_loc2);
      if (macro_info == _seen_macros.end ()) {
        cout << "fatal: macro info not found in kill" << endl;
        return;
      }
      clang::SourceLocation macro_start = macro_info->second._start;
      _rewriter->RemoveText(clang::CharSourceRange::getCharRange(macro_start, loc2));

      clang::SourceLocation somewhere_inside = macro_info->second._somewhere_inside;
      std::pair<clang::SourceLocation, clang::SourceLocation> range =
          SM.getExpansionRange(somewhere_inside);
      _rewriter->RemoveText(clang::CharSourceRange::getCharRange(
          loc1, range.first), opts);
    }
    else {
      // both locations are in a macro instance

      // remove from 'from' to the macro end
      MacroInfoMap::iterator macro_info1 = _seen_macros.find(expansion_loc1);
      if (macro_info1 == _seen_macros.end ()) {
        cout << "fatal: macro info not found in kill" << endl;
        return;
      }
      clang::SourceLocation macro_end = macro_info1->second._end;

      // remove from macro start to 'end'
      MacroInfoMap::iterator macro_info2 = _seen_macros.find(expansion_loc2);
      if (macro_info2 == _seen_macros.end ()) {
        cout << "fatal: macro info not found in kill" << endl;
        return;
      }
      clang::SourceLocation macro_start = macro_info2->second._start;

      if (macro_info1 != macro_info2) {
        _rewriter->RemoveText(clang::CharSourceRange::getCharRange(loc1, macro_end));
        _rewriter->RemoveText(clang::CharSourceRange::getCharRange(macro_start, loc2));

        // remove from first macro expansion end to second macro expansion begin
        clang::SourceLocation somewhere_inside1 = macro_info1->second._somewhere_inside;
        std::pair<clang::SourceLocation, clang::SourceLocation> range1 =
            SM.getExpansionRange(somewhere_inside1);
        unsigned len = _rewriter->getRangeSize(clang::SourceRange(range1.second, range1.second));

        clang::SourceLocation somewhere_inside2 = macro_info2->second._somewhere_inside;
        std::pair<clang::SourceLocation, clang::SourceLocation> range2 =
            SM.getExpansionRange(somewhere_inside2);

        _rewriter->RemoveText(clang::CharSourceRange::getCharRange(
            range1.second.getLocWithOffset(len), range2.first), opts);
      }
      else {
        _rewriter->RemoveText(clang::CharSourceRange::getCharRange(loc1, loc2));
      }
    }
    return;
  }

  if (!from.isValid () || from.isMacroID ()||
      !to.isValid () || to.isMacroID ()) {
    cout << "Invalid or macro source loc in kill" << endl;
    return;
  }
  _rewriter->RemoveText (clang::CharSourceRange::getCharRange (from, to), opts);
}

void WeaverBase::commit_internal () {

  // commit all code deletions and clear the operations
  commit_kills();
  _deletions.clear();

  // commit code insertions
  clang::SourceManager &SM = _rewriter->getSourceMgr();
  // handle code that has to be inserted at weaving positions
  for (WPSet::iterator i = _positions.begin (); i != _positions.end (); ++i) {
    const WeavePos &wp = *i;
    if (wp._loc.isInvalid ()) {
      cout << "Invalid source loc" << endl;
      continue;
    }
    bool nl = false;
    bool last_nl = false;

    // Handle macros.
    if (wp._loc.isMacroID()) {
      LocMap::iterator i = _loc_map.find(wp._loc);
      if (i == _loc_map.end()) {
        cout << "location not found during commit" << endl;
        continue;
      }
      clang::SourceLocation loc = i->second;
      // TODO: Same as for non-macro insertions.
      for (list<string>::iterator si = wp._strings.begin();
           si != wp._strings.end(); ++si) {
        const string &str = *si;
        _rewriter->InsertTextAfter(loc, str);
        nl |= (str.find("\n") != string::npos);
        last_nl = (str[str.size() - 1] == '\n');
      }
      clang::SourceLocation expansion_loc = SM.getExpansionLoc(wp._loc);
      _macro_expansions.insert (expansion_loc);
    } else {
      // Non-macro insertions.
      for (list<string>::iterator si = wp._strings.begin();
           si != wp._strings.end(); ++si) {
        const string &str = *si;
        _rewriter->InsertTextAfter(wp._loc, str);
        nl |= (str.find("\n") != string::npos);
        last_nl = (str[str.size() - 1] == '\n');
      }
    }
    if (nl) { // at least one new line character was inserted here
      // generate and insert a #line directive to make sure that debuggers will
      // be able to show the original source locations
      const llvm::MemoryBuffer *u = SM.getBuffer(SM.getFileID(wp._loc));
      if (IntroductionUnit::cast(u)) {
//        ACM_Introduction *intro = IntroductionUnit::cast(u)->intro ();
//        if (intro) {
//          cout << "insert into intro "
//            << filename(*get_slice (*intro)) << ": " << line(*get_slice (*intro)) << endl;
//        }
//        else
//          cout << "no class slice" << endl;
      }
      else {
        clang::PresumedLoc ploc = SM.getPresumedLoc(wp._loc);
        ostringstream directive;
        // TODO: this seems to be unnecessary as the line mgr issues \n
        if (!last_nl) directive << endl;
        _line_mgr.directive (directive, ploc);
        _rewriter->InsertTextAfter (wp._loc, directive.str ());
      }
    }
  }

  // clear the list of weaving positions used for this transaction
  _positions.clear();
}

void WeaverBase::resolve_macros() {
  clang::SourceManager &SM = _rewriter->getSourceMgr();

  for (set<clang::SourceLocation>::iterator i = _macro_expansions.begin ();
      i != _macro_expansions.end (); ++i) {
    MacroInfoMap::iterator macro_info = _seen_macros.find(*i);
    if (macro_info == _seen_macros.end ()) {
      cout << "fatal: macro info not found in resolve_macros" << endl;
      continue;
    }
    clang::FileID fid = macro_info->second._fid;
    clang::SourceLocation somewhere_inside = macro_info->second._somewhere_inside;
    // expand macro;
    std::pair<clang::SourceLocation, clang::SourceLocation> range =
        SM.getExpansionRange(somewhere_inside);
    const clang::RewriteBuffer *RB = _rewriter->getRewriteBufferFor(fid);
    if (!RB) {
      cout << "fatal: Rewrite buffer not found" << endl;
      continue;
    }
    string str(RB->begin(), RB->end());
//    cout << "Replace macro " << range.first.printToString(SM) << ": " << str << endl;
    _rewriter->ReplaceText(clang::SourceRange(range.first, range.second), str);
  }
  _macro_expansions.clear ();
  _seen_macros.clear ();
  _loc_map.clear ();
  _token_descriptions.clear ();
  _in_macro = false;
}

bool WeaverBase::commit (clang::SourceManager &Target) {
  clang::SourceManager &SM = _rewriter->getSourceMgr();

  // Resolve weaving positions.
  commit_internal();

  // Put all macros back into the source.
  resolve_macros();

  for (clang::Rewriter::buffer_iterator i = _rewriter->buffer_begin(),
                                        e = _rewriter->buffer_end();
       i != e; ++i) {
    if (const clang::FileEntry *FE = SM.getFileEntryForID(i->first)) {
      std::string data(i->second.begin(), i->second.end());
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      llvm::MemoryBuffer *Buf =
          llvm::MemoryBuffer::getMemBufferCopy(data, FE->getName());
      Target.overrideFileContents(FE, Buf);
#else // C++ 11 interface
      Target.overrideFileContents(FE, llvm::MemoryBuffer::getMemBufferCopy(data, FE->getName()));
#endif
    }
  }

  // renew rewrite
  const clang::LangOptions &LO = _rewriter->getLangOpts ();
  delete _rewriter;
  _rewriter = new clang::Rewriter (Target, LO);

  return true;
}

// postorder traversal of IntroductionUnits, inserting into their parents.
static void po_intros(WeaverBase &wb, ::ACIntroducer &intro,
                      IntroductionUnit *unit, unsigned depth = 0) {
  std::vector<IntroductionUnit *> &units = intro.get_intros_for(unit);

  for (unsigned i = 0, e = units.size(); i != e; ++i)
    po_intros(wb, intro, units[i], unit ? depth + 1 : depth);

  if (!unit)
    return;

  if (const clang::RewriteBuffer *RB =
          wb.getRewriter().getRewriteBufferFor(unit->file_id())) {
    std::string str(RB->begin(), RB->end());
    if (depth == 0) // Insert the last unit via weave_pos, there may be macros.
      wb.insert(wb.weave_pos(unit->location(), WeavePos::WP_AFTER), str);
    else
      wb.getRewriter().InsertText(unit->location(), str);
  } else {
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    llvm::StringRef str(unit->buffer()->getBuffer());
#else // C++ 11 interface
    std::unique_ptr<llvm::MemoryBuffer> buf (unit->buffer());
    llvm::StringRef str(buf->getBuffer());
#endif
    if (depth == 0) // Insert the last unit via weave_pos, there may be macros.
      wb.insert(wb.weave_pos(unit->location(), WeavePos::WP_AFTER), str);
    else
      wb.getRewriter().InsertText(unit->location(), str);
  }
}

bool WeaverBase::resolve_introduction_units(::ACIntroducer &intro) {
  // Resolve weaving positions.
  commit_internal();
  // Insert introduction units.
  po_intros(*this, intro, NULL);
  commit_internal();
  resolve_macros();
  return true;
}
