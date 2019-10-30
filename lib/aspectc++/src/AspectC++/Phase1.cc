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

#include "Phase1.h"
#include "ModelBuilder.h"
#include "PointCutExprParser.h"
#include "ACModel/Utils.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"

using namespace std;
using namespace Puma;

#ifdef FRONTEND_PUMA
int Phase1::run (IncludeGraph &include_graph) {
#else
int Phase1::run () {
#endif

  // set the start and end token of the unit
#ifdef FRONTEND_PUMA
  if (!unit ()) {
    err() << sev_error << "can't scan file \"" << _tunit_name.c_str () << "\""
     << endMessage;
    return -1;
  }

  Token *file_first = unit()->first ();
  Token *file_last  = unit()->last ();
  if (!file_first)
    return 0;
  assert (file_last);

  _code_weaver.init ((Token*)file_first, (Token*)file_last);
#endif

  // prepare the project model
#ifdef FRONTEND_PUMA
  _jpm.setup_phase1((FileUnit*)unit (), _code_weaver.primary_len());
#else
  _jpm.setup_phase1(_tunit_name, tunit_len());
#endif

  // prepare the parser's scope hierarchy
  _root_scope._kind         = Scope::SCOPE_NAMESPACE;
  _root_scope._name         = "::";
  _root_scope._is_slice     = false;
  _root_scope._parent_scope = 0;
  _root_scope._jpm_link     = _jpm.get_root();

  // parse the elements of the global scope
  try {
    parse_scope (_root_scope);
  }
  catch (ParseError) {
    // we simply return an error code if there was a parse error, indicated by an exception
    // the message has been printed on the error stream where the error was detected
    return -1;
  }

#ifdef FRONTEND_PUMA  // TODO: check this for Clang as well
  // special check for tokens after eoi
  PumaToken after_program = curr_token();
  if (after_program) { // error, e.g. too many closing brackets
    err() << sev_error << after_program.location()
        << "tokens after end of program (too many closing brackets?)"
        << endMessage;
  }

//  _root_scope.dump ();

  // finally parse the preprocessor syntax tree in order to create an
  // include graph of the original source code
  get_include_graph(include_graph);
#endif

  return (err().severity () < sev_error) ? 1 : -1;
}

void Phase1::skip_block (int open, int close, bool inclusive) {
  int token;
  int depth = 0;
  ACToken open_token = curr_token ();

  while (curr_token ().type () != ACToken::TOK_EOF) {
    token = curr_token ().type ();

    if (token == open)
      depth++;
    else if (token == close)
      depth--;
    else if (depth > 0 && token == ACToken::TOK_OPEN_ROUND) {
      skip_round_block ();
      continue;
    }

    if (depth == 0 && !inclusive)
      break;

    next_token ();

    if (depth == 0)
      break;
  }

  if (curr_token ().type () == ACToken::TOK_EOF) {
    err() << sev_error << open_token.location() << "missing closing bracket" << endMessage;
    throw ParseError();
  }
}

void Phase1::skip_round_block (bool inclusive) {
  skip_block (ACToken::TOK_OPEN_ROUND, ACToken::TOK_CLOSE_ROUND, inclusive);
}

void Phase1::skip_curly_block (bool inclusive) {
  skip_block (ACToken::TOK_OPEN_CURLY, ACToken::TOK_CLOSE_CURLY, inclusive);
}

void Phase1::skip_square_block (bool inclusive) {
  skip_block (ACToken::TOK_OPEN_SQUARE, ACToken::TOK_CLOSE_SQUARE, inclusive);
}

void Phase1::skip_template_params () {
  int depth = 0;
  next_token ();

  bool stop = false;
  while (!stop && curr_token ().type () != ACToken::TOK_EOF) {
    int token = curr_token ().type ();

    switch (token) {
      case ACToken::TOK_SEMI_COLON:
      case ACToken::TOK_CLOSE_ROUND:
      case ACToken::TOK_CLOSE_SQUARE:
      case ACToken::TOK_CLOSE_CURLY:
        stop = true;
        break;
      case ACToken::TOK_OPEN_ROUND:
        skip_round_block ();
        continue;
      case ACToken::TOK_OPEN_CURLY:
        skip_curly_block ();
        continue;
      case ACToken::TOK_OPEN_SQUARE:
        skip_block (ACToken::TOK_OPEN_SQUARE, ACToken::TOK_CLOSE_SQUARE);
        continue;
      case ACToken::TOK_LESS:
        depth++;
        break;
      case ACToken::TOK_GREATER:
        if (depth == 0) {
          next_token ();
          stop = true;
        } else {
          depth--;
        }
        break;
      default:
        break;
    }

    if (!stop)
      next_token ();
  }
}

bool Phase1::is_attribute_token (const ACToken &token, const ACToken &next) const {
  return is_gnu_attribute_token(token) || is_cxx11_attribute_token(token, next);
}

bool Phase1::is_cxx11_attribute_token (const ACToken &token, const ACToken &next) const {
  return token.type() == ACToken::TOK_OPEN_SQUARE &&
      next.type() == ACToken::TOK_OPEN_SQUARE;
}

bool Phase1::is_gnu_attribute_token (const ACToken &token) const {
  return token.type () == ACToken::TOK_ATTRIBUTE ||
      token.type () == ACToken::TOK_ALIGNAS;
}

void Phase1::skip_cxx11_attributes (bool inclusive) {
  while (is_cxx11_attribute_token(curr_token (), look_ahead ())) {
    skip_square_block(false);
    if (!is_cxx11_attribute_token(look_ahead (), look_ahead(2)))
      break;
    next_token ();
  }
  if (inclusive)
    next_token ();
}

void Phase1::skip_attributes (bool inclusive) {
  while (true) {
    if (is_gnu_attribute_token(curr_token()))
      skip_gnu_attributes(false);
    else if (is_cxx11_attribute_token(curr_token(), look_ahead()))
      skip_cxx11_attributes(false);
    else
      return;
    if (!is_attribute_token(look_ahead(1), look_ahead(2))) {
      if (inclusive)
        next_token();
      return;
    }
    next_token();
  }
}

void Phase1::skip_gnu_attributes (bool inclusive) {
  while (is_gnu_attribute_token(curr_token ())) {
    next_token();
    skip_round_block(false);
    if (!is_gnu_attribute_token(look_ahead ()))
      break;
    next_token ();
  }
  if (inclusive)
    next_token ();
}

ACM_Name *Phase1::register_scope (Scope &scope) {
  if (!scope._jpm_link) {
    ACM_Name *parent_scope = register_scope (*scope._parent_scope);
    if (scope.is_namespace()) {
      scope._jpm_link = _jpm.register_namespace1(parent_scope, scope._name, false);
    }
    else if (scope.is_aspect()) {
      scope._jpm_link = _jpm.register_aspect1(parent_scope, scope._name, false);
      register_base_classes (scope);
    }
    else if (scope.is_class_or_struct()) {
      scope._jpm_link = _jpm.register_class1(parent_scope, scope._name, false);
      register_base_classes (scope);
    }
  }
  return scope._jpm_link;
}

void Phase1::register_base_classes (Scope &cls) {
  ACM_Class *elem = (ACM_Class*)cls._jpm_link;
  for (list<Scope*>::iterator i = cls._search_scopes.begin ();
      i != cls._search_scopes.end (); ++i) {
    ACM_Class *base_class = (ACM_Class*)register_scope (*(*i));
    elem->get_bases().insert (base_class);
    base_class->get_derived().insert(elem);
  }
}

bool Phase1::is_visible_scope (Scope &scope, const string &name) {
  if (scope._name == "::")
    return (name != "AC" && name != "JoinPoint");
  return is_visible_scope (*scope._parent_scope, scope._name);
}

string Phase1::full_anon_class_name (Scope &scope) const {
  if (scope._parent_scope->_name.substr (0, 9) == "__ac_anon")
    return full_anon_class_name (*scope._parent_scope) + "::" + scope._name;
  else
    return scope._name;
}

#ifdef FRONTEND_CLANG
void Phase1::set_slice_tokens (TU_ClassSlice *slice,
                               ACPreprocessor::TokenVector &pattern) {

  // Analyze the tokens.
  bool has_base_intro = false;
  bool has_member_intro = false;
  for (unsigned j = 0; j != pattern.size(); ++j) {
    if (pattern[j].type () == ACToken::TOK_SEMI_COLON)
      break;
    if (pattern[j].type () == ACToken::TOK_COLON)
      has_base_intro = true;
    else if (pattern[j].type () == ACToken::TOK_OPEN_CURLY) {
      has_member_intro = true;
      break;
    }
  }

  // Generate the base intro.
  // TODO: What about replacements in base intros?
  //       Example: ": public SomeBase<TargetName>"
  TU_ClassSlice::SliceBody base_intro;
  if (has_base_intro) {
    unsigned i = 1;
    while (i < pattern.size() && pattern[i].type () != ACToken::TOK_COLON)
      ++i;
    ++i; // skip ':'
    while (i < pattern.size() &&
        pattern[i].type () != ACToken::TOK_SEMI_COLON &&
        pattern[i].type () != ACToken::TOK_OPEN_CURLY) {
      base_intro.text += ' ';
      base_intro.text += token_text (pattern[i]);
      if (!base_intro.file.is_valid()) {
        base_intro.file = source_unit(pattern[i]);
        base_intro.line = token_line_number(pattern[i]);
      }
      ++i;
    }
  }

  // Generate the (inline) member intro
  TU_ClassSlice::SliceBody member_intro;
  if (has_member_intro) {
    unsigned i = 0;
    while (pattern[i].type () != ACToken::TOK_OPEN_CURLY)
      ++i;
    ++i;
    member_intro.file = source_unit(pattern[i]);
    member_intro.line = token_line_number(pattern[i]);
    unsigned line = member_intro.line;
    int level = 1;
    while (true) {
      if (pattern[i].type () == ACToken::TOK_CLOSE_CURLY) {
        level--;
        if (level == 0)
          break;
      }
      else if (pattern[i].type () == ACToken::TOK_OPEN_CURLY)
        level++;

      // insert whitespace or newline for proper formatting
      whitespace (line, pattern[i], member_intro.text);

      // TODO: this is a hack - make sure not to replace too often!
      if (level == 1 && pattern[i].type () == ACToken::TOK_ID &&
          slice->get_name() == pattern[i].text ())
        member_intro.positions.push_back(
            std::make_pair(member_intro.text.size(),
                TU_ClassSlice::SliceBody::TARGET_NAME));
      else if (pattern[i].type () == ACToken::TOK_ID &&
          string ("JoinPoint") == pattern[i].text ()) {
        member_intro.positions.push_back(
            std::make_pair(member_intro.text.size(),
                TU_ClassSlice::SliceBody::JP_NAME));
      }
      else
        member_intro.text += token_text (pattern[i]);
      ++i;
    }

    // Sort positions so we can easily do the insertions later.
    // TODO: Aren't those always sorted anyways?
    std::sort(member_intro.positions.begin(), member_intro.positions.end());
  }

  slice->set_tokens (member_intro, base_intro, has_base_intro, has_member_intro);
}
#endif // FRONTEND_CLANG

void replace_string_in_place (string& subject, const string& search, const string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

void Phase1::parse_scope (Scope &scope, string *anon_member_class_copy) {
  static string prot_none("");
  static string prot_private("private");
  static string prot_protected("protected");
  static string prot_public("public");

  Scope *old_scope = _curr_scope;
  _curr_scope = &scope;
  bool old_in_template      = _in_template;
  bool in_friend            = false;
  bool in_slice             = false;
  bool in_slice_initializer = false;
  enum { TD_FLOATING, TD_GOT_ID, TD_GOT_TYPEDEF, TD_GOT_ALL } _td_state = TD_FLOATING;
  Scope *class_alias_target = 0;
  string *prot = &prot_none;
  if (scope.is_class() || scope.is_aspect())
    prot = &prot_private;
  else if (scope.is_struct())
    prot = &prot_public;
  int advice_no = 0;
  int intro_no = 0;
  int order_no = 0;
  int anon_slice_no = 0;
  bool is_anon_class = false;
  int anon_class_no = 0;
  ACToken anon_class_def_token;
  string anon_class_name;  // generated name for anon class
  string anon_class_alias; // as in "typedef struct { ... } alias_name;"
  ACToken start_token;

  // if we are in the definition of an anonymous class/struct/union it might be
  // that we are parsing the definition of an anonymous member. In this case we
  // need to create a modified copy of the code and thus start recording here.
  if (anon_member_class_copy)
    start_recording ();

  ACToken token = curr_token ();
  while (token.type () != ACToken::TOK_EOF &&
      token.type () != ACToken::TOK_CLOSE_CURLY) {

    int tt = token.type ();
    // handle sub-scope
    if (tt == ACToken::TOK_TEMPLATE) {
      // A declaration of a template class or function
      token = next_token ();
      if (token.type () != ACToken::TOK_EOF && token.type () == ACToken::TOK_LESS) {
        skip_template_params();
        token = curr_token();
        _in_template = true;
      }
    }
    else if (tt == ACToken::TOK_ENUM) { // handle C++ 11 enum classes
      token = next_token();
      if (token.type () == ACToken::TOK_CLASS)
        token = next_token(); // simply skip the class keyword if there is one
    }
    else if (tt == ACToken::TOK_PUBLIC) {
      token = next_token();
      if (token.type () == ACToken::TOK_COLON)
        prot = &prot_public;
      token = next_token();
    }
    else if (tt == ACToken::TOK_PROTECTED) {
      token = next_token();
      if (token.type () == ACToken::TOK_COLON)
        prot = &prot_protected;
      token = next_token();
    }
    else if (tt == ACToken::TOK_PRIVATE) {
      token = next_token();
      if (token.type () == ACToken::TOK_COLON)
        prot = &prot_private;
      token = next_token();
    }
    else if (scope.is_aspect() &&
        ((tt == ACToken::TOK_STATIC &&
        look_ahead ().type () == ACToken::TOK_ID &&
        scope._name == look_ahead().text ()) ||
        (tt == ACToken::TOK_ID && scope._name == token.text () &&
        look_ahead().type () == ACToken::TOK_STATIC))) {
      parse_aspectof_function (*prot);
      token = next_token ();
    }
    else if (tt == ACToken::TOK_TYPEDEF) {
      switch (_td_state) {
      case TD_GOT_TYPEDEF:
      case TD_GOT_ALL:
        err () << sev_error << token.location()
            << "'typedef' specifier used twice." << endMessage;
        break;
      case TD_GOT_ID:
        _td_state = TD_GOT_ALL;
        break;
      case TD_FLOATING:
        _td_state = TD_GOT_TYPEDEF;
        break;
      }
      token = next_token();
    }
    else if (_td_state == TD_GOT_ALL && tt == ACToken::TOK_ID) {
      if (look_ahead(1).type () == ACToken::TOK_SEMI_COLON) {
        if (class_alias_target) {
          Element alias;
          alias._kind = Element::ELEMENT_CLASS_ALIAS;
          alias._jpm_link = 0;
          alias._name = token.text();
          alias._referred = class_alias_target;
          scope._elements.insert(alias);
        }
      }
      token = next_token();
    }
    else if (tt == ACToken::TOK_FRIEND) {
      in_friend = true;
      token = next_token();
    }
    else if (tt == ACToken::TOK_POINTCUT) {
      parse_pointcut_def (scope);
      token = next_token ();
    }
    else if (tt == ACToken::TOK_ATTRDECL && _conf.attributes()) {
#ifdef FRONTEND_CLANG
      parse_attribute_decl(scope);
#else
      err() << sev_warning << "attributes are not supported for puma-frontend, yet! Use clang-frontend instead!" << endMessage;
#endif
      token = next_token();
    }
    else if (scope.is_namespace() && tt == ACToken::TOK_USING) {
      parse_using_directive (scope);
      token = next_token ();
    }
    else if (tt == ACToken::TOK_ADVICE) {
      parse_advice_def (scope, advice_no, intro_no, order_no, anon_slice_no, *prot);
      token = next_token ();
    }
    else if (tt == ACToken::TOK_SLICE) {
      in_slice = true;
      start_token = token;
      start_recording ();
      token = next_token ();
    }
    else if (!_in_template && !in_friend &&
        (tt == ACToken::TOK_NAMESPACE ||
       (tt == ACToken::TOK_ASPECT) ||
        tt == ACToken::TOK_CLASS ||
        tt == ACToken::TOK_UNION ||
        tt == ACToken::TOK_STRUCT)) {
      bool is_struct = (tt == ACToken::TOK_STRUCT);
      bool is_class  = (tt == ACToken::TOK_CLASS);
      bool is_aspect = (tt == ACToken::TOK_ASPECT);
      bool is_union  = (tt == ACToken::TOK_UNION);
      anon_class_alias.clear();
      string pre_attr;
      // A namespace sub-scope?
      ACToken start = token;
      if (is_aspect)
        _code_weaver.transform_aspect(start);
      token = next_token ();

      // skip __attribute(...)
      if (is_attribute_token(token, look_ahead())) {
        // stop recording
        if (anon_member_class_copy) {
#ifdef FRONTEND_PUMA
          Puma::Unit *rec = stop_recording ();
          string result = recording_to_string (rec);
          delete rec;
#else
          ACPreprocessor::TokenVector rec = stop_recording ();
          string result = recording_to_string (rec);
#endif
          *anon_member_class_copy += result;
        }
        start_recording();
        skip_attributes(false);
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        pre_attr = recording_to_string (rec);
        delete rec;
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
        pre_attr = recording_to_string (rec);
#endif
        if (anon_member_class_copy) {
          *anon_member_class_copy += pre_attr;
          start_recording();
        }
        token = next_token();
      }

      bool root_qualified, contains_template_id;
      vector<string> names;
      parse_qual_name (names, root_qualified, contains_template_id);
      token = curr_token ();
      bool is_qualified = (root_qualified || names.size () > 1);
      string name;
      if (names.size () > 0)
        name = names[names.size () - 1];
      else {
        if (tt == ACToken::TOK_NAMESPACE) {
          name = "<unnamed>";
        }
        else if (is_struct || is_class || is_union || is_aspect) {
          is_anon_class = false;
          ostringstream name_str;
          if (in_slice) {
            name_str << "<anon-slice-" << anon_slice_no << ">";
            anon_slice_no++;
            err () << sev_error << start.location()
                << "Anonymous slice definition only allowed in advice." << endMessage;
          }
          else {
            if (_curr_scope->_name == "::")
              name_str << "__ac_anon" << anon_class_no;
            else {
              if (_curr_scope->_name.substr (0, 9) != "__ac_anon")
                name_str << "__ac_anon";
              name_str << _curr_scope->_name << "_" << anon_class_no;
            }
            anon_class_no++;
            if (is_aspect)
              err () << sev_error << start.location()
                  << "Anonymous aspects are not allowed." << endMessage;
            else {
              if (is_in_project(start)) {
                is_anon_class = true;
                anon_class_def_token = token;
                anon_class_name = name_str.str ();
              }
            }
          }
          name = name_str.str ();
        }
      }
      Node *found_scope = 0;
      if (is_qualified)
        found_scope = lookup_name(scope, root_qualified, names);
      Scope *qual_scope = lookup_scope(scope, root_qualified, names);
      // TODO: handle other cases here (errors!)

      // check if this scope belongs to our project
      bool skip = (is_qualified && !found_scope && in_slice);
      bool in_model = true;
      bool is_declaration = (token.type () == ACToken::TOK_SEMI_COLON);
      Scope *inserted_scope = 0;
      if (!skip && (token.type () == ACToken::TOK_OPEN_CURLY || token.type () == ACToken::TOK_COLON ||
          is_declaration)) {
        if (!is_in_project(start) || !is_visible_scope(scope, name) ||
            !scope._jpm_link || name.substr(0,6) == "<anon-" ||
            name.substr(0,9) == "__ac_anon") {
          // TODO: shall we really filter-out anonymous classes?
          in_model = false;
        }

        // create new scope object
        Scope new_scope;
        new_scope._name         = name;
        new_scope._is_slice     = (in_slice || scope._is_slice);
        new_scope._parent_scope = (qual_scope ? qual_scope : &scope);
        if (start.type () == ACToken::TOK_NAMESPACE) {
          new_scope._kind         = Scope::SCOPE_NAMESPACE;
          new_scope._jpm_link     = (in_model ? _jpm.register_namespace1 (scope._jpm_link, name) : 0);
        }
        else if (start.type () == ACToken::TOK_ASPECT) {
          new_scope._kind         = Scope::SCOPE_ASPECT;
          new_scope._jpm_link     = (in_model ? _jpm.register_aspect1 (scope._jpm_link, name) : 0);
        }
        else if (start.type () == ACToken::TOK_CLASS || start.type () == ACToken::TOK_STRUCT) {
          new_scope._kind = (is_struct ? Scope::SCOPE_STRUCT : Scope::SCOPE_CLASS);
          if (in_slice)
            new_scope._jpm_link = _jpm.register_class_slice(
                qual_scope ? qual_scope->_jpm_link : scope._jpm_link, name, is_struct);
          else
            new_scope._jpm_link     = (in_model ?
                _jpm.register_class1 (qual_scope ? qual_scope->_jpm_link : scope._jpm_link, name) : 0);
        }
        else if (start.type () == ACToken::TOK_UNION) {
          new_scope._kind         = Scope::SCOPE_UNION;
          new_scope._jpm_link     = 0;
        }
        pair<set<Scope>::const_iterator, bool> res =
            new_scope._parent_scope->_sub_scopes.insert (new_scope);
        inserted_scope = &(Scope&)*res.first;
        // A namespace first introduced as external, might now become part of the project.
        // Therefore, add the project model link here.
        if (!inserted_scope->_jpm_link && new_scope._jpm_link)
          inserted_scope->_jpm_link = new_scope._jpm_link;
      }

      // handle class alias typedef state machine
      if (!qual_scope && inserted_scope &&
          (_td_state == TD_FLOATING || _td_state == TD_GOT_TYPEDEF)) {
        _td_state = (_td_state == TD_FLOATING ? TD_GOT_ID : TD_GOT_ALL);
        class_alias_target = inserted_scope;
      }

      // stop recording after the classname
      if (anon_member_class_copy &&
          (token.type () == ACToken::TOK_COLON || token.type () == ACToken::TOK_OPEN_CURLY)) {
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        string result = recording_to_string (rec);
        delete rec;
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
        string result = recording_to_string (rec);
#endif
        *anon_member_class_copy += result;
      }

      bool is_definition = false;
      if (token.type () != ACToken::TOK_EOF && token.type () == ACToken::TOK_COLON) {
        is_definition = true;
        if (in_slice) {
          // consume tokens until we find '{' or syntax error
          do {
            token = next_token();
          } while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_OPEN_CURLY
              && token.type () != ACToken::TOK_SEMI_COLON);
        }
        else if (inserted_scope &&
            (inserted_scope->is_class_or_struct() || inserted_scope->is_aspect())) {
          // list of base classes
          parse_base_clause (*inserted_scope);
          if (in_model) register_base_classes (*inserted_scope);
          token = curr_token();
        }
      }

      if (token.type () != ACToken::TOK_EOF && token.type () == ACToken::TOK_OPEN_CURLY) {
        is_definition = true;
        if (in_slice) {
          skip_curly_block ();
          token = curr_token ();
        }
        else {
          // parse the nested scope
          next_token ();
          if (is_anon_class) {
            string nested_anon_member_class_copy;
            parse_scope (*inserted_scope, &nested_anon_member_class_copy);

            if (curr_token().type () != ACToken::TOK_EOF && (is_struct || is_class || is_union)) {
              // skip __attribute(...)
              string post_attr;
              if (is_gnu_attribute_token(look_ahead ())) {
                next_token ();
                start_recording (); // record attribute while being lexed
                skip_gnu_attributes(false);
#ifdef FRONTEND_PUMA
                Puma::Unit *rec = stop_recording ();
                post_attr = recording_to_string (rec);
                delete rec;
#else
                ACPreprocessor::TokenVector rec = stop_recording ();
                post_attr = recording_to_string (rec);
#endif
              }

              // Check whether we have found the definition of an unnamed strcut/union field, such as in
              // class C { struct { int i; }; } c; <-- here 'i' is visible in 'C'!
              // => check whether ';' follows '}' ...
              if (look_ahead().type () == ACToken::TOK_SEMI_COLON) {
                if (anon_member_class_copy)
                  *anon_member_class_copy += nested_anon_member_class_copy;

                // transform the code:
                //   insert an anonymous class with equivalent members behind
                //   the de-anonymized class
                string tag;
                if (is_struct)
                  tag = "struct";
                else if (is_union)
                  tag = "union";
                else if (is_class)
                  tag = "class";

                replace_string_in_place (nested_anon_member_class_copy,
                    full_anon_class_name(scope) + "::", "");
                _code_weaver.transform_anon_member_class (look_ahead(),
                    tag + pre_attr + " { " + nested_anon_member_class_copy + " }" + post_attr + ";");
              }
              else {
                // check for the definition of a class alias
//                cout << _td_state << ":" << token_text(look_ahead()) << ":" << token_text(look_ahead(2)) << endl;
                if (_td_state == TD_GOT_ALL &&
                    look_ahead().type() == ACToken::TOK_ID &&
                    look_ahead(2).type() == ACToken::TOK_SEMI_COLON) {
                  anon_class_alias = look_ahead().text ();
                  // TODO: handle more (exotic) cases
                }
                if (anon_member_class_copy) {
                  anon_member_class_copy->resize (anon_member_class_copy->size () - 1);
                  *anon_member_class_copy += full_anon_class_name(*inserted_scope);
                  next_token ();
                }
              }
            }
          }
          else
            parse_scope (*inserted_scope);
          // and start recording again
          if (anon_member_class_copy)
            start_recording ();
        }
        if (!skip && (in_model || in_slice)) {
          if (curr_token ().type() == ACToken::TOK_EOF)
            err () << sev_error << start.location()
            << "Scope '" << inserted_scope->_name.c_str() << "' not closed properly" << endMessage;
          else {
            if (inserted_scope->_jpm_link)
              _jpm.add_source_loc(inserted_scope->_jpm_link, start, curr_token (),
                  (inserted_scope->is_namespace() ? SLK_NONE : SLK_DEF));
          }
        }
      }
      else {
        if (inserted_scope && inserted_scope->_jpm_link && (in_model || in_slice)) {
          _jpm.add_source_loc(inserted_scope->_jpm_link, start, curr_token (), SLK_DECL);
        }
      }

      if (!skip && is_definition && qual_scope && !found_scope)
        err () << sev_error << start.location()
          << "Definition of undeclared scope '" << name.c_str() << "'." << endMessage;

#ifdef FRONTEND_PUMA
      Puma::Unit *rec = 0;
#else
      ACPreprocessor::TokenVector rec;
#endif
      if (in_slice) {
        in_slice = false;
        rec = stop_recording ();
      }
      if (inserted_scope && inserted_scope->_is_slice) {
        if (is_definition) {
          TU_ClassSlice *cs = (TU_ClassSlice*)inserted_scope->_jpm_link;
          cs->slice_unit(source_unit(start));
#ifdef FRONTEND_PUMA
          if (cs->get_tokens ())
            cout << "tokens OVERWRITTEN" << endl;
          cs->set_tokens(rec);
#else
          if (!cs->get_tokens ().text.empty())
            cout << "tokens OVERWRITTEN" << endl;
          set_slice_tokens(cs, rec);
#endif
        }
        _code_weaver.transform_delete(start_token, curr_token ());
      }
#ifdef FRONTEND_PUMA
      else if (rec) {
#else
      else if (!rec.empty ()) {
#endif
        handle_slice_member(scope, rec, start_token, curr_token());
      }
      token = next_token ();
    }
    else if (in_slice && tt == ACToken::TOK_ASSIGN) {
      in_slice_initializer = true;
      token = next_token ();
    }
    else if (tt == ACToken::TOK_OPEN_CURLY) {
      skip_curly_block (false);
      if (in_slice && !in_slice_initializer) {
        in_slice = false;
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        if (rec)
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
        if (!rec.empty ())
#endif
          handle_slice_member(scope, rec, start_token, curr_token());
      }
      _in_template = old_in_template;
      in_friend = false;
      token = next_token ();
    }
    else if (tt == ACToken::TOK_SEMI_COLON) {
      if (in_slice) {
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        if (rec)
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
        if (!rec.empty ())
#endif
          handle_slice_member(scope, rec, start_token, curr_token());
        in_slice = false;
        in_slice_initializer = false;
      }
      _in_template = old_in_template;
      token = next_token ();
      in_friend = false;
      if (is_anon_class) {
        if (anon_class_alias.empty())
          _code_weaver.transform_anon_class(anon_class_def_token, anon_class_name);
        else
          _code_weaver.transform_anon_class(anon_class_def_token, anon_class_alias);
        anon_class_alias.clear();
      }
      is_anon_class = false;
      _td_state = TD_FLOATING;
    }
    else if (tt == ACToken::TOK_COLON_COLON || tt == ACToken::TOK_ID) {
      bool root_qualified, contains_template_id;
      vector<string> names;
      parse_qual_name (names, root_qualified, contains_template_id);
      token = curr_token();
      if (_td_state == TD_GOT_ID || _td_state == TD_GOT_ALL ||
          contains_template_id || names.size () == 0)
        _td_state = TD_FLOATING;
      else {
        Node *found = lookup_name(scope, root_qualified, names);
        if (found && (found->is_class_or_struct() || found->is_aspect())) {
          _td_state = (_td_state == TD_FLOATING ? TD_GOT_ID : TD_GOT_ALL);
          class_alias_target = (Scope*)found;
        }
        else
          _td_state = TD_FLOATING;
      }
    }
    else {
      if (_td_state != TD_FLOATING && tt != ACToken::TOK_CONST && tt != ACToken::TOK_VOLATILE)
        _td_state = TD_FLOATING;
      token = next_token ();
    }
  }

  if (anon_member_class_copy) {
#ifdef FRONTEND_PUMA
    Puma::Unit *rec = stop_recording ();
    string result = recording_to_string (rec);
    delete rec;
#else
    ACPreprocessor::TokenVector rec = stop_recording ();
    string result = recording_to_string (rec);
#endif
    result.resize (result.size () - 1); // delete trailing '}'
    *anon_member_class_copy += result;

  }
  _curr_scope = old_scope;
}

#ifdef FRONTEND_CLANG
string Phase1::recording_to_string (const ACPreprocessor::TokenVector &vec) {
  string result;
  if (vec.size () > 0) {
    unsigned last_line = token_line_number(vec[0]);
    for (unsigned i = 0; i < vec.size(); i++) {
      unsigned line = token_line_number(vec[i]);
      bool new_line = false;
      while (line > last_line) {
        result += "\n";
        last_line++;
        new_line = true;
      }
      if (!new_line) result += " ";
      result += token_text (vec[i]);
    }
  }
  return result;
}
#else
string Phase1::recording_to_string (Puma::Unit *rec) {
  string result;
  Token *curr = (Token*)rec->first ();
  if (curr) {
    int last_line = curr->location ().line ();
    while (curr) {
      int line = curr->location ().line ();
      bool new_line = false;
      while (line > last_line) {
        result += "\n";
        last_line++;
        new_line = true;
      }
      if (!new_line) result += " ";
      result += curr->text ();
      curr = (Token*)rec->next (curr);
    }
  }
  return result;
}
#endif

#ifdef FRONTEND_CLANG
// Create a SliceBody structure out of the tokens in pattern.
TU_ClassSlice::SliceBody
Phase1::format_non_inline_member (ACPreprocessor::TokenVector &pattern,
                                  const std::string &name) {
  TU_ClassSlice::SliceBody res;

  int name_tokens = 0;
  unsigned line = token_line_number(pattern[0]);
  res.line = line;
  res.file = source_unit(pattern[0]);
  // skip 'slice' keyword
  for (unsigned i = 1; i < pattern.size(); ) {
    ACToken token = pattern[i];
    unsigned next = i+1;
    if (name_tokens == 0 && next < pattern.size() &&
        pattern[next].type() == ACToken::TOK_INVALID) {
      name_tokens = 1;
      next = i+2;
    }
    else if (name_tokens > 0 && token.type() == ACToken::TOK_INVALID) {
      if (res.text.size () > 0)
        res.text += ' '; // don't change the line number here
      res.positions.push_back(std::make_pair(res.text.size(),
                                   TU_ClassSlice::SliceBody::TARGET_QUAL_NAME));
      name_tokens = 0;
      if (next < pattern.size() && pattern[next].type () == ACToken::TOK_COLON_COLON) {
        token = pattern[next];
        ++next;
        res.text += token_text (token);
        if (next < pattern.size() && pattern[next].type () == ACToken::TOK_TILDE) {
          token = pattern[next];
          ++next;
          res.text += token_text (token);
        }
        if (next < pattern.size() && pattern[next].type () == ACToken::TOK_ID) {
          token = pattern[next];
          ++next;
          if (name == token.text()) {
            res.positions.push_back(std::make_pair(res.text.size(),
                                        TU_ClassSlice::SliceBody::TARGET_NAME));
          }
          else
            res.text += token_text (token);
        }
      }
    }
    else {
      if (name_tokens == 0) {
        // insert whitespace or newline for proper formatting
        whitespace (line, token, res.text);
        if (token.type () == ACToken::TOK_ID &&
            string ("JoinPoint") == token.text ()) {
          res.positions.push_back(std::make_pair(res.text.size(),
                                            TU_ClassSlice::SliceBody::JP_NAME));
        }
        else {
          res.text += token_text (token);
        }
      }
      else
        name_tokens++;
    }
    i = next;
  }
  res.text += "\n";

  // Sort positions so we can easily do the insertions later.
  // TODO: Aren't those always sorted anyways?
  std::sort(res.positions.begin(), res.positions.end());
  return res;
}

// insert whitespace or newline for proper formatting
void Phase1::whitespace (unsigned &line, const ACToken &token, string &str) {
  unsigned new_line = token_line_number(token);
  if (!token.is_macro_generated () && line != new_line) {
    if (new_line > line)
      str.append(new_line - line, '\n');
    line = new_line;
  }
  else {
    if (str.size () > 0)
      str += ' ';
  }
}
#endif // FRONTEND_CLANG

#ifdef FRONTEND_PUMA
  void Phase1::handle_slice_member (Scope &scope, Puma::Unit *rec, ACToken from, ACToken to) {
#else
  void Phase1::handle_slice_member (Scope &scope, ACPreprocessor::TokenVector &rec, ACToken from, ACToken to) {
#endif
  Scope *slice_scope = find_slice(scope, rec);
  if (slice_scope && slice_scope->_jpm_link) {
    TU_ClassSlice *s = (TU_ClassSlice*)slice_scope->_jpm_link;
    s->non_inline_members().push_back (
#ifdef FRONTEND_PUMA
        rec
#else
        format_non_inline_member (rec, s->get_name ())
#endif
        );
    s->non_inline_member_units().push_back (source_unit(from));
  }
  else {
    err () << sev_error << from.location()
        << "Slice member definition cannot be associated with slice definition" << endMessage;
  }
  _code_weaver.transform_delete(from, to);
}

void Phase1::parse_aspectof_function (const string &prot) {
  ACToken start = curr_token ();
  next_token ();
  next_token ();
  if (curr_token ().type() != ACToken::TOK_MUL)
    return;
  next_token ();
  if (curr_token ().type() != ACToken::TOK_ID)
    return;
  ACToken name = curr_token ();
  string text = name.text ();
  if (text != "aspectOf" && text != "aspectof")
    return;
  ACToken arg_begin = next_token ();
  skip_round_block(false);
  next_token ();
  ACToken body_begin = curr_token ();
  AdviceCodeContext context = ACC_NONE;
  CodeWeaver::TypeUse uses_type;
  parse_advice_body( context, ACToken::TOK_OPEN_CURLY, ACToken::TOK_CLOSE_CURLY, uses_type );
  ACToken body_end = curr_token ();
  _code_weaver.transform_aspectof( context, uses_type, start, name, arg_begin, body_begin, body_end, prot );
}

void Phase1::parse_using_directive(Scope &scope) {
  ACToken token = next_token ();
  if (token.type() != ACToken::TOK_NAMESPACE)
    return;
  token = next_token ();
  ACToken name_start = token;
  bool root_qualified, contains_template_id;
  vector<string> names;
  parse_qual_name (names, root_qualified, contains_template_id);
  if (names.size () == 0) {
    err() << sev_error << name_start.location()
        << "Invalid name in 'using directive'." << endMessage;
    return;
  }
  string name = names[names.size () - 1];
  Node *found_scope = lookup_name(scope, root_qualified, names);
  token = curr_token ();
  if (token.type () != ACToken::TOK_SEMI_COLON) {
    err() << sev_error << token.location() 
        << "Expected ';' after namespace name in 'using directive." << endMessage;
    return;
  }
  if (!found_scope || !found_scope->is_namespace()) {
    err() << sev_error << name_start.location()
        << "'" << name.c_str() << "' is not a known namespace." << endMessage;
    return;
  }
  scope._search_scopes.push_back ((Scope*)found_scope);
}

void Phase1::parse_pointcut_def (Scope &scope) {
  ACToken start = curr_token ();
  ACToken token = next_token ();
  bool is_virtual = false; // neither virtual nor pure virtual
  ACToken virtual_token;
  if (token.type () == ACToken::TOK_VIRTUAL) {
    is_virtual = true; // either virtual or pure virtual
    virtual_token = token;
    token = next_token();
  }
  if (!(token.type () == ACToken::TOK_ID))
    return; // TODO: handle syntax error in pointcut definition
  string name = token.text ();
  token = next_token ();
  if (!(token.type () == ACToken::TOK_OPEN_ROUND))
    return; // TODO: handle syntax error in pointcut definition
  skip_round_block();
  token = curr_token();
  if (!(token.type () == ACToken::TOK_ASSIGN))
    return; // TODO: handle syntax error in pointcut definition
  ACToken assignment_token = token;
  token = next_token();
  string expr;
  ACToken pct_end_token = token;
  while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_SEMI_COLON) {
#ifdef FRONTEND_PUMA
    expr += token.text ();
#else
    expr += token_text (token);
#endif
    pct_end_token = token;
    token = next_token();
  }
  // check whether the definition is semantically correct
  if (expr == "0" && !is_virtual) {
    err () << sev_error << start.location()
        << "Pure virtual pointcut defined without 'virtual' keyword." << endMessage;
    is_virtual = true;
  }
  // register the scope of the named pointcut; should normally not be necessary
  ACM_Name *parent = scope._jpm_link;
  if (!parent)
    parent = register_scope(scope);
  // register the named pointcut itself
  TU_Pointcut *pct = (TU_Pointcut*)_jpm.register_pointcut1 (parent, name, is_virtual, expr);
  if (!pct) {
    err () << sev_error << start.location()
        << "Pointcut '" << name.c_str() << "' already defined." << endMessage;
    return;
  }
  PointCutExpr *pce = 0;
  if (expr != "0")
    pce = create_pct_expr_tree(expr, parent, start);
  pct->get_parsed_expr().set (pce);
#ifdef FRONTEND_PUMA
  pct->set_location(start.location());
#else
  pct->set_location(clang::FullSourceLoc (start.location(), err().get_source_manager ()));
#endif
  _jpm.add_source_loc(pct, start, curr_token ());
  Element pct_elem;
  pct_elem._kind = Element::ELEMENT_POINTCUT;
  pct_elem._name = name;
  pct_elem._jpm_link = pct;
  scope._elements.insert(pct_elem);
  // finally transform the named pointcut definition into a function declaration (for phase 2)
  _code_weaver.transform_pointcut_def(start, virtual_token, assignment_token, pct_end_token);
}

#ifdef FRONTEND_CLANG
void Phase1::parse_attribute_decl(Scope &scope)
{
  ACToken start = curr_token ();

  if(!scope.is_namespace() && !scope.is_class() && !scope.is_aspect()) {
    err() << sev_error << curr_token().location()
          << "attribute declaration not allowed in this scope - daclare in namespace or class" << endMessage;
    return;
  }

  ACToken nameToken = next_token ();

  if (nameToken.type () != ACToken::TOK_ID) {
    err() << sev_error << curr_token().location()
          << "attribute keyword used without identifier." << endMessage;
    return;
  }

  string name = nameToken.text();

  ACToken token = next_token();

  if (token.type() != ACToken::TOK_OPEN_ROUND) {
    err() << sev_error << curr_token().location()
          << "expected '(' after attribute-identifier" << endMessage;
    return;
  }

  token = next_token();
  start_recording();

  int openBracket = 1;

  while (true) {
    if(token.type() == ACToken::TOK_OPEN_ROUND)
      openBracket++;
    else if(token.type() == ACToken::TOK_CLOSE_ROUND) {
      openBracket--;
      if(openBracket == 0)
        break;
    }
    else if(token.type() == ACToken::TOK_EOF) {
      err() << sev_error << curr_token().location()
            << "Missing ';' after attribute-identifier" << endMessage;
      return;
    }

    token = next_token();
  }

#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        string attrParams = recording_to_string (rec);
        rec->remove(rec->last());
#else
        ACPreprocessor::TokenVector paramTokens = stop_recording ();
        paramTokens.pop_back();
        string attrParams = recording_to_string(paramTokens);        
#endif

  //check if any params are declared for attribute - this is not allowed in first stage of implementation!
  if(!attrParams.empty()) {
    err() << sev_error << curr_token().location() << "attributes with parameters are not supported, yet" << endMessage;
  }

  token = next_token();

  if (token.type() != ACToken::TOK_SEMI_COLON) {
    err() << sev_error << curr_token().location()
          << "expected ';' after attribute-declaration" << endMessage;
    return;
  }

  ACM_Namespace *parent = (ACM_Namespace*) scope._jpm_link;
  if (!parent)
    parent = (ACM_Namespace*) register_scope(scope);

  TU_Attribute *attr = _jpm.register_attrdecl1(parent, name);

  if(attr == nullptr) {
    //    err() << sev_error << curr_token().location()
    //          << "redeclaration of attribute '" << name.c_str() << "'" << endMessage;

    //TODO:check whether Arguments fit!
    return;
  }

  attr->setParamStr(attrParams);

#ifdef FRONTEND_PUMA
  attr->set_location(start.location());
#else
  attr->set_location(clang::FullSourceLoc (start.location(), err().get_source_manager ()));
#endif

  _jpm.add_source_loc(attr, start, curr_token ());
  Element attr_elem;
  attr_elem._kind = Element::ELEMENT_CXX11Attr;
  attr_elem._name = name;
  attr_elem._jpm_link = attr;
  scope._elements.insert(attr_elem);

  // finally transform the attribute definition for phase 2:
  // for now we delete it
  _code_weaver.transform_delete(start, token);

  //  _code_weaver.transform_attribute_def(start, token, attr->get_name(), attr->getParamStr());
}
#endif //FRONTEND_CLANG

void Phase1::parse_advice_def (Scope &scope, int &advice_no, int &intro_no, int &order_no, int &anon_slice_no,
     const string &prot) {
  if (!scope.is_aspect()) {
    err() << sev_error << curr_token ().location()
        << "'advice' keyword used outside an aspect." << endMessage;
    return;
  }

  ACToken start = curr_token ();
  ACToken token = next_token ();
  ACToken pct_start = token, pct_end = token;
  string expr;
  while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_COLON) {
    // TODO: handle errors, e.g. ';' or '}' token
#ifdef FRONTEND_PUMA
    expr += token.text ();
#else
    expr += token_text(token);
#endif
    pct_end = token;
    token = next_token();
  }
  // create an anonymous pointcut object, ignore arguments for now
  TU_Pointcut *pct = (TU_Pointcut*)_jpm.register_pointcut1(0, "%anon", false, expr);
  pct->get_parsed_expr().set (create_pct_expr_tree(expr, scope._jpm_link, pct_start));
#ifdef FRONTEND_PUMA
  pct->set_location(pct_start.location());
#else
  pct->set_location(clang::FullSourceLoc (pct_start.location(), err().get_source_manager ()));
#endif
  _jpm.add_source_loc(pct, pct_start, pct_end);

//  cout << "advice on " << expr << endl;
  token = next_token ();
  if (token.type() == ACToken::TOK_EOF) return;
  string text = token.text ();
  if (token.type () == ACToken::TOK_ID &&
      (text == "before" || text == "after" || text == "around")) {
    ACToken advice_kind_token = token;
    next_token ();
    ACToken arg_begin = curr_token();
    ACToken next = look_ahead ();
    if (next.type () == ACToken::TOK_EOF) return; // TODO: error handling
    bool has_args = (next.type() != ACToken::TOK_CLOSE_ROUND);
    skip_round_block(false);
    next_token ();
    ACToken body_begin = curr_token ();
    AdviceCodeContext context = ACC_NONE;
    CodeWeaver::TypeUse uses_type;
    parse_advice_body (context, ACToken::TOK_OPEN_CURLY, ACToken::TOK_CLOSE_CURLY, uses_type );
    if (has_args)
      context = (AdviceCodeContext)(context | ACC_VARS);
    TU_AdviceCode *new_elem = _jpm.newAdviceCode();
    ((ACM_Aspect*)scope._jpm_link)->get_advices().insert(new_elem);
    new_elem->set_kind(text == "before" ? ACT_BEFORE: text == "after" ? ACT_AFTER:ACT_AROUND);
    new_elem->set_pointcut(pct);
    new_elem->set_context(context);
    new_elem->set_lid(advice_no);
    ACToken body_end = curr_token();
    _jpm.add_source_loc(new_elem, start, body_end);
    _code_weaver.transform_delete(start, advice_kind_token);
    _code_weaver.transform_advice_code( advice_no, text, context, has_args, uses_type, arg_begin, body_begin, body_end, prot );
    advice_no++; // increment number of code advice block per aspect
  }
  else if (token.type () == ACToken::TOK_ID && text == "order") {
    ACToken order_start = token;
    token = next_token ();
    if (token.type() != ACToken::TOK_OPEN_ROUND) {
      err() << sev_error << order_start.location ()
          << "Syntax error in order advice, expected '(' after 'order'" << endMessage;
      return;
    }
    TU_Order *order = (TU_Order*)_jpm.newOrder();
    order->set_pointcut(pct);
    order->set_lid(advice_no);
//#ifdef FRONTEND_PUMA
//    order->tree (0);
//#endif
    while (true) {
      token = next_token ();
      ACToken pct_start = token, pct_end = token;
      string expr;
      int bracket_depth = 0;
      while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_COMMA &&
          !(token.type() == ACToken::TOK_CLOSE_ROUND && bracket_depth == 0)) {
        // TODO: handle errors, e.g. ';' or '}' token
        if (token.type () == ACToken::TOK_OPEN_ROUND)
          bracket_depth++;
        else if (token.type () == ACToken::TOK_CLOSE_ROUND)
          bracket_depth--;
#ifdef FRONTEND_PUMA
        expr += token.text ();
#else
        expr += token_text (token);
#endif
        pct_end = token;
        token = next_token();
      }
      TU_Pointcut *pct = (TU_Pointcut*)_jpm.register_pointcut1(0, "%anon", false, expr);
      pct->get_parsed_expr().set (create_pct_expr_tree(expr, scope._jpm_link, pct_start));
#ifdef FRONTEND_PUMA
      pct->set_location(pct_start.location());
#else
      pct->set_location(clang::FullSourceLoc (pct_start.location(), err().get_source_manager ()));
#endif
      _jpm.add_source_loc (pct, pct_start, pct_end);
      order->get_aspect_exprs().insert(pct);
      if (token.type () == ACToken::TOK_CLOSE_ROUND)
        break;
    }
    token = next_token();
    if (token.type () != ACToken::TOK_SEMI_COLON)
      return;
    ((ACM_Aspect*)scope._jpm_link)->get_orders().insert(order);
    _jpm.add_source_loc(order, start, token);
    _code_weaver.transform_delete(start, token);
    order_no++; // increment number of order advice per aspect
  }
  else if (token.type () == ACToken::TOK_SLICE) {
    ACToken slice_start = token;
    TU_ClassSlice *slice_obj = 0;
    ACToken next = look_ahead ();
    if (token.type () == ACToken::TOK_EOF) return; // TODO: error handling
    TU_Introduction *intro = _jpm.newIntroduction();
    intro->set_pointcut(pct);
    intro->set_lid(intro_no);
    intro_no++;
    if (next.type () == ACToken::TOK_CLASS || next.type () == ACToken::TOK_STRUCT) {
      bool is_struct = (next.type () == ACToken::TOK_STRUCT);
      start_recording ();
      token = next_token ();
      token = next_token ();
      bool root_qualified, contains_template_id;
      vector<string> names;
      parse_qual_name (names, root_qualified, contains_template_id);
      token = curr_token ();
      Node *found_scope = 0;
      Scope *qual_scope = 0;
      bool is_qualified = (root_qualified || names.size () > 1);
      if (is_qualified) {
        found_scope = lookup_name(scope, root_qualified, names);
        qual_scope = lookup_scope(scope, root_qualified, names);
      }
      string name;
      if (names.size () > 0) {
        name = names[names.size () - 1];
      }
      else {
        ostringstream name_str;
        name_str << "<anon-slice-" << anon_slice_no << ">";
        name = name_str.str ();
        anon_slice_no++;
      }
      if (token.type () == ACToken::TOK_SEMI_COLON) {
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
        delete rec;
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
#endif
        slice_obj = (TU_ClassSlice*)_jpm.register_class_slice(
            (qual_scope ? qual_scope->_jpm_link : scope._jpm_link), name, is_struct);
        _jpm.add_source_loc(slice_obj, slice_start, curr_token(), SLK_DECL);
      }
      else {
        while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_SEMI_COLON) {
          if (token.type () == ACToken::TOK_OPEN_CURLY) {
            skip_curly_block (false);
            if (look_ahead(1).type() != ACToken::TOK_SEMI_COLON)
              err() << sev_error << curr_token().location()
                  << "Semicolon after class slice introduction missing." << endMessage;
            break;
          }
          token = next_token ();
        }
#ifdef FRONTEND_PUMA
        Puma::Unit *rec = stop_recording ();
#else
        ACPreprocessor::TokenVector rec = stop_recording ();
#endif
        if (token.type () == ACToken::TOK_EOF) return; // TODO: error handling
        if (qual_scope && !found_scope) {
          err () << sev_warning << slice_start.location()
              << "Definition of undeclared slice" << endMessage;
        }
        slice_obj = (TU_ClassSlice*)_jpm.register_class_slice(qual_scope ? qual_scope->_jpm_link : scope._jpm_link, name, is_struct);
        slice_obj->slice_unit(source_unit(slice_start));
        _jpm.add_source_loc(slice_obj, slice_start, curr_token());
#ifdef FRONTEND_PUMA
        if (slice_obj->get_tokens ())
#else
        if (!slice_obj->get_tokens ().text.empty())
#endif
          err() << sev_error << slice_start.location()
            << "Slice '" << name.c_str () << "' was already defined." << endMessage;
#ifdef FRONTEND_PUMA
        slice_obj->set_tokens (rec);
#else
        set_slice_tokens(slice_obj, rec);
#endif
      }
      // create new scope object
      Scope new_scope;
      new_scope._name         = name;
      new_scope._is_slice     = true;
      new_scope._parent_scope = (qual_scope ? qual_scope : &scope);
      new_scope._kind         = (is_struct ? Scope::SCOPE_STRUCT : Scope::SCOPE_CLASS);
      new_scope._jpm_link     = slice_obj;
      new_scope._parent_scope->_sub_scopes.insert (new_scope);
    }
    else {
      token = next_token ();
      ACToken name_start = token;
      bool root_qualified, contains_template_id;
      vector<string> names;
      parse_qual_name (names, root_qualified, contains_template_id);
      if (names.size () == 0) {
        err() << sev_error << name_start.location()
            << "expected slice name instead of '"
#ifdef FRONTEND_PUMA
            << name_start.text()
#else
            << token_text(name_start).c_str()
#endif
            << "' token." << endMessage;
        return;
      }
      Node *found_scope = lookup_name(scope, root_qualified, names);
      // TODO: handle error here
      string name ("<unnamed>");
      if (names.size () > 0)
        name = names[names.size () - 1];
      token = curr_token ();
      if (token.type () != ACToken::TOK_SEMI_COLON) return; // TODO: handle syntax errors
      if (!found_scope || !found_scope->is_class_or_struct() ||
          !((Scope*)found_scope)->_is_slice ||
          !found_scope->_jpm_link) {
        err() << sev_error << name_start.location()
            << "'" << name.c_str() << "' is not a known class slice." << endMessage;
      }
      else
        slice_obj = (TU_ClassSlice*)found_scope->_jpm_link;
    }
    if (slice_obj)
      intro->set_named_slice(slice_obj);
    // intro->intro_info(acii);
    ((ACM_Aspect*)scope._jpm_link)->get_intros().insert(intro);
    _jpm.add_source_loc(intro, start, curr_token(), SLK_NONE);
    _code_weaver.transform_delete(start, curr_token ());
    order_no++; // increment number of order advice per aspect
  }
  else if (token.type () == ACToken::TOK_ID && text == "baseclass") {
    err() << sev_error << token.location()
        << "Deprecated introduction syntax used in aspect, use slice instead." << endMessage;
    while (token.type () != ACToken::TOK_EOF && token.type () != ACToken::TOK_SEMI_COLON)
      token = next_token();
  }
  else {
    err() << sev_error << token.location()
        << "Deprecated introduction syntax used in aspect, use slice instead." << endMessage;
    // skip the code;
    bool objdecl = false;
    while (token.type () != ACToken::TOK_EOF) {
      if (token.type () == ACToken::TOK_SEMI_COLON)
        break;
      if (token.type () == ACToken::TOK_CLASS ||
          token.type () == ACToken::TOK_STRUCT ||
          token.type () == ACToken::TOK_UNION ||
          token.type () == ACToken::TOK_ENUM) {
        token = next_token();
        if (token.type () == ACToken::TOK_ID)
          token = next_token();
        if (token.type () == ACToken::TOK_OPEN_CURLY)
          objdecl = true;
      }
      if (token.type () == ACToken::TOK_OPEN_ROUND) {
        skip_round_block(true);
        token = curr_token();
        continue;
      }
      if (token.type () == ACToken::TOK_OPEN_CURLY) {
        skip_curly_block(false);
        if (!objdecl)
          break;
        token = next_token();
      }
      if (token.type () == ACToken::TOK_ASSIGN)
        objdecl = true;
      token = next_token();
    }
  }
}

void Phase1::parse_advice_body( AdviceCodeContext &context, int open, int close, CodeWeaver::TypeUse &uses_type ) {
  ACToken current;
  int token;
  int depth = 0;

  while (curr_token ().type () != ACToken::TOK_EOF) {
    current = curr_token ();
    token = current.type ();

    if (token == open)
      depth++;
    else if (token == close)
      depth--;
    else if (depth > 0 && token == ACToken::TOK_OPEN_ROUND) {
      parse_advice_body (context, ACToken::TOK_OPEN_ROUND, ACToken::TOK_CLOSE_ROUND, uses_type );
      next_token ();
      continue;
    }

    if (depth == 0)
      break;

    if (current.type () == ACToken::TOK_ID) {
      string text = current.text ();
      if (text == "JoinPoint") {
        context = (AdviceCodeContext)(context | ACC_TYPE);
        ACToken qualname_start = current;
        if (look_ahead ().type () == ACToken::TOK_COLON_COLON) {
          next_token (); // skip "::"
          current = next_token ();
          bool have_template_kw = false;
          if (current.type () == ACToken::TOK_TEMPLATE) {
            current = next_token ();
            have_template_kw = true;
          }
          if (current.type() == ACToken::TOK_ID) {
            string name = current.text ();
            if( name == "That" || name == "Result" || name == "Target" ||
                name == "Entity" || name == "MemberPtr" || name == "Array" ) {
              ostringstream tn;
              Naming::tjp_typedef(tn, name.c_str ());
              _code_weaver.replace (_code_weaver.weave_pos(qualname_start, WeavePos::WP_BEFORE),
                  _code_weaver.weave_pos(current, WeavePos::WP_AFTER), tn.str ());
              if (name == "That")
                uses_type.that = true;
              else if (name == "Target")
                uses_type.target = true;
              else if (name == "Result")
                uses_type.result = true;
              else if (name == "Entity")
                uses_type.entity = true;
              else if (name == "MemberPtr")
                uses_type.memberptr = true;
              else if( name == "Array" )
                uses_type.array = true;
            }
            else if( name == "Arg" || name == "Dim" ) {
              // generate "template" keyword in front of Arg<i> or Dim<i> if the user hasn't provided it
              if (!have_template_kw)
                _code_weaver.insert (_code_weaver.weave_pos(current, WeavePos::WP_BEFORE),
                    "template ");
              current = next_token ();
              skip_template_params();
              current = curr_token ();
              if (current.type () == ACToken::TOK_COLON_COLON) {
                current = next_token ();
                if (current.type () == ACToken::TOK_ID) {
                  current = next_token ();
                  if (current.type () != ACToken::TOK_COLON_COLON)
                    _code_weaver.insert (_code_weaver.weave_pos(qualname_start, WeavePos::WP_BEFORE),
                        "typename ");
                }
              }
            }
          }
        }
      }
      else if (text == "tjp" || text == "thisJoinPoint") {
        context = (AdviceCodeContext)(context | ACC_TYPE_OBJ);
        if (text == "thisJoinPoint") {
          // replace every occurrence of "thisJoinPoint" with "tjp"
          _code_weaver.replace(_code_weaver.weave_pos(current, WeavePos::WP_BEFORE),
              _code_weaver.weave_pos(current, WeavePos::WP_AFTER), "tjp");
        }
        if (look_ahead().type() == ACToken::TOK_PTS) {
          next_token (); // skip "->"
          current = next_token ();
          if (current.type() == ACToken::TOK_ID &&
              ( string( current.text() ) == "arg" || ( string( current.text() ) == "idx" ) ) &&
              look_ahead ().type () == ACToken::TOK_LESS) {
            _code_weaver.insert(_code_weaver.weave_pos(current, WeavePos::WP_BEFORE), "template ");
          }
        }
      }
    }
    next_token ();
  }
}

PointCutExpr *Phase1::create_pct_expr_tree(const string &pct,
    ACM_Name *scope, ACToken pos) {
  // don't call the parser if the input is empty
  if (pct == "") {
    err() << sev_error << pos.location()
        << "Empty pointcut expression." << endMessage;
    return 0;
  }
  PointCutExprParser *pce_parser = PointCutExprParser::instance( _conf );
  try {
    return pce_parser->parse(pct, *this);
  }
  catch (const std::exception &e) {
    err () << sev_error << pos.location()
      << "Invalid pointcut expression: " << e.what() << "." << endMessage;
  }
  return 0;
}

void Phase1::parse_base_clause (Scope &class_scope) {
  ACToken token = curr_token();
  int count = 0;
  while (token.type () != ACToken::TOK_EOF &&
      ((count == 0 && token.type () == ACToken::TOK_COLON) ||
          (count > 0 && token.type () == ACToken::TOK_COMMA))) {
    parse_base_spec (class_scope);
    token = curr_token();
    count++;
  }
}

void Phase1::parse_qual_name (vector<string> &names,
    bool &root_qualified, bool &contains_template_id) {
  contains_template_id = false;
  root_qualified = (curr_token ().type () == ACToken::TOK_COLON_COLON);
  if (root_qualified) next_token();

  while (curr_token ().type () == ACToken::TOK_ID) {
    string name = curr_token().text ();
    names.push_back (name);
    next_token ();
    if (curr_token().type () == ACToken::TOK_LESS) {
      contains_template_id = true;
      skip_template_params ();
    }
    if (curr_token().type() == ACToken::TOK_COLON_COLON)
      next_token ();
    else
      break;
  }
}

void Phase1::parse_base_spec (Scope &class_scope) {
  // skip ':' or ','
  next_token();
  int count_public = 0, count_private = 0, count_protected = 0, count_virtual = 0;
  while (true) {
    int token = curr_token().type();
    if (token == ACToken::TOK_PUBLIC) count_public++;
    else if (token == ACToken::TOK_PRIVATE) count_private++;
    else if (token == ACToken::TOK_PROTECTED) count_protected++;
    else if (token == ACToken::TOK_VIRTUAL) count_virtual++;
    else if (token == ACToken::TOK_ID || token == ACToken::TOK_COLON_COLON) break;
    else return;
    next_token ();
  }

  bool contains_template_id, root_qualified;
  vector<string> names;
  parse_qual_name (names, root_qualified, contains_template_id);

//  cout << "base of " << class_scope._name << "(" << root_qualified << " " << contains_template_id << "):";
//  for (vector<string>::const_iterator i = names.begin(); i != names.end(); ++i)
//    cout << " " << *i;
//  cout << endl;

  if (!contains_template_id && names.size () > 0) {
    Node *base = lookup_name (*class_scope._parent_scope, root_qualified, names);
    if (base && (base->is_class_or_struct() || base->is_aspect()))
      class_scope._search_scopes.push_back ((Scope*)base);
  }
}


Phase1::Node *Phase1::lookup_name (Scope &scope, bool root_qualified, const vector<string> &names) {
  if (root_qualified)
    return lookup_name(_root_scope, false, names);
  Scope *search_scope = &scope;
  while (search_scope) {
    set<Scope*> new_visited_scopes;
    Node *result = lookup_name_in_scope (*search_scope, names, new_visited_scopes);
    if (result)
      return result;
    search_scope = search_scope->_parent_scope;
  }
  return 0;
}

Phase1::Node *Phase1::lookup_name_in_scope (Scope &scope, const vector<string> &names,
    std::set<Scope*> &visited_scopes, int depth) {
  Node *result = 0;
  Scope search;
  search._name = names[depth];
  set<Scope>::iterator i = scope._sub_scopes.find (search);
  if (i != scope._sub_scopes.end ())
    result = (Node*)&(*i);

  if (!result) {
    Element search_elem;
    search_elem._name = names[depth];
    set<Element>::iterator i = scope._elements.find (search_elem);
    if (i != scope._elements.end ()) {
      if (i->_kind == Element::ELEMENT_POINTCUT)
        result = (Node*)&(*i);
      else if (i->_kind == Element::ELEMENT_CLASS_ALIAS)
        result = (*i)._referred;
      else if (i->_kind == Element::ELEMENT_CXX11Attr)
        result = (Node*)&(*i);
    }
  }

  // for classes and aspects also lookup the name in base classes/aspects
  // for namespaces search in other namespaces mentioned in using directives
  if (!result) {
    visited_scopes.insert (&scope);
    for (std::list<Scope*>::const_iterator i = scope._search_scopes.begin ();
        i != scope._search_scopes.end (); ++i) {
      // if a search scope was searched alread, skip it silently
      if (visited_scopes.find (*i) != visited_scopes.end()) {
        continue;
      }
      // search the search scope
      if ((result = lookup_name_in_scope(**i, names, visited_scopes, depth)) != 0)
        break;
    }
  }
  if (result && result->_kind == Node::ELEMENT_POINTCUT)
    return (depth == (int)(names.size() - 1)) ? result : 0;
  // if we found the scope, check the remaining parts of the qualified name
  if (result && (depth + 1 < (int)names.size ())) {
    set<Scope*> new_visited_scopes;
    return lookup_name_in_scope(*(Scope*)result, names, new_visited_scopes, depth + 1);
  }
  return result;
}

Phase1::Scope *Phase1::lookup_scope (Scope &scope, bool root_qualified, vector<string> &names) {
  if (names.size () <= 1)
    return 0;
  vector<string> scope_names;
  for (unsigned i = 0; i < names.size () - 1; i++)
    scope_names.push_back (names[i]);
  Node *result = lookup_name (scope, root_qualified, scope_names);
  return (result->is_scope() ? (Scope*)result : 0);
}

ACM_Pointcut *Phase1::lookup_pct_func (bool root_qualified, vector<string> &qual_name) {
  Node *lookup_result = lookup_name (*_curr_scope, root_qualified, qual_name);
  if (lookup_result && lookup_result->_kind == Element::ELEMENT_POINTCUT &&
      lookup_result->_jpm_link &&
      lookup_result->_jpm_link->type_val() == JPT_Pointcut) {
    return (ACM_Pointcut*)lookup_result->_jpm_link;
  }
  return 0;
}

ACM_Attribute *Phase1::lookup_pct_attr(bool root_qualified, std::vector<string> &qual_name)
{
  Node *lookup_result = lookup_name (*_curr_scope, root_qualified, qual_name);
  if (lookup_result && lookup_result->_kind == Element::ELEMENT_CXX11Attr &&
    lookup_result->_jpm_link &&
      lookup_result->_jpm_link->type_val() == JPT_Attribute) {
    return (ACM_Attribute*)lookup_result->_jpm_link;
  }
  return 0;
}

#ifdef FRONTEND_PUMA
Phase1::Scope *Phase1::find_slice (Scope &scope, Puma::Unit *rec) {
  enum { EXP_SEP, EXP_ID, FLOATING } state = FLOATING;
  vector<string> names;
  bool root_qualified = false;
  Token *name_start = 0;
  Scope *result = 0;
  Token *token = rec->first ();
  while (token) {
    if (state == FLOATING && token->type() == TOK_COLON_COLON) {
      root_qualified = true;
      name_start = token;
      state = EXP_ID;
    }
    else if (state == EXP_SEP && token->type() == TOK_COLON_COLON) {
      state = EXP_ID;
    }
    else if (state != EXP_SEP && token->type() == TOK_ID) {
      if (state == FLOATING)
        name_start = token;
      names.push_back (token->text ());
      Scope *lookup_result = 0;
      Node *lr = lookup_name(scope, root_qualified, names);
      if (lr && lr->is_class_or_struct())
        lookup_result = (Scope*)lr;
      if (lookup_result) {
        if (lookup_result->_is_slice) {
          if (result && lookup_result != result) {
            // TODO: generate real error message
            cout << "ERROR conflicting slice names referenced" << endl;
            cout << result->_name << " <--> " << lookup_result->_name << endl;
            cout << "SLICE: " << *rec << endl;
            cout << "SCOPE: " << scope._name << endl;
            scope.dump();
          }
          else {
            result = lookup_result;
            Token *marker = new Token;
            rec->insert(name_start, *marker);
            marker = new Token;
            rec->insert(token, *marker);
            // TODO: continue to consume tokens while we are in a qualified name
          }
        }
      }
      state = EXP_SEP;
    }
    else {
      state = FLOATING;
      names.clear ();
      root_qualified = false;
      name_start = 0;
      if (token->type () == TOK_ID)
        continue; // this ID could be the start of the (qualified) slice name
    }
    token = rec->next (token);
  }
  return result;
}
#else
Phase1::Scope *Phase1::find_slice (Scope &scope, ACPreprocessor::TokenVector &rec) {
  enum { EXP_SEP, EXP_ID, FLOATING } state = FLOATING;
  vector<string> names;
  bool root_qualified = false;
  unsigned name_start = -1U;
  Scope *result = 0;
  for (unsigned i = 1; i != rec.size(); ) {
    ACToken token = rec[i];
    if (state == FLOATING && token.type() == ACToken::TOK_COLON_COLON) {
      root_qualified = true;
      name_start = i;
      state = EXP_ID;
    }
    else if (state == EXP_SEP && token.type() == ACToken::TOK_COLON_COLON) {
      state = EXP_ID;
    }
    else if (state != EXP_SEP && token.type() == ACToken::TOK_ID) {
      if (state == FLOATING)
        name_start = i;
      names.push_back (token.text ());
      Scope *lookup_result = 0;
      Node *lr = lookup_name(scope, root_qualified, names);
      if (lr && lr->is_class_or_struct())
        lookup_result = (Scope*)lr;
      if (lookup_result) {
        if (lookup_result->_is_slice) {
          if (result && lookup_result != result) {
            // TODO: generate real error message
            cout << "ERROR conflicting slice names referenced" << endl;
            cout << result->_name << " <--> " << lookup_result->_name << endl;
            //cout << "SLICE: " << *rec << endl;
            cout << "SCOPE: " << scope._name << endl;
            scope.dump();
          }
          else {
            result = lookup_result;
            clang::Token tok;
            tok.startToken ();
            ACToken marker(tok);
            rec.insert(rec.begin() + name_start + 1, marker);
            i++;
            rec.insert(rec.begin() + i + 1, marker);
            i++;
            // TODO: continue to consume tokens while we are in a qualified name
          }
        }
      }
      state = EXP_SEP;
    }
    else {
      state = FLOATING;
      names.clear ();
      root_qualified = false;
      name_start = -1U;
      if (token.type () == ACToken::TOK_ID)
        continue; // this ID could be the start of the (qualified) slice name
    }
    ++i;
  }
  return result;
}
#endif
