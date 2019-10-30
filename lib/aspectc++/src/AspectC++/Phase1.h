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

#ifndef __Phase1_h__
#define __Phase1_h__

#include <string>
#include <vector>
#include <set>

#include "ACModel/Elements.h"

#include "IncludeGraph.h"
#include "ModelBuilder.h"
#include "CodeWeaver.h"
#include "PointCutSearcher.h"
#include "ACPreprocessor.h"

class CodeWeaver;

namespace Puma {
  class CProject;
  class Unit;
  class FileUnit;
  class TokenProvider;
  class Token;
}

// This class performs preliminary parsing of a translation unit. Its
// purpose is to find all aspect-oriented constructs in the code,
// store information about it in the translation unit's model, and
// remove or replace the aspect-oriented code. Thereby, the main
// parsing can be performed by an ordinary C++ parser without
// token and grammar extensions.

class Phase1 : public PointCutSearcher, private ACPreprocessor {

  // Exception type
  struct ParseError {};

  // Helper classes to create a simple syntax tree while parsing
  struct Node {
    enum { ELEMENT_POINTCUT, ELEMENT_CLASS_ALIAS, ELEMENT_CXX11Attr,
      SCOPE_NAMESPACE, SCOPE_CLASS, SCOPE_STRUCT, SCOPE_ASPECT,
      SCOPE_UNION } _kind;
    bool is_scope() const { return _kind >= SCOPE_NAMESPACE; }
    bool is_aspect() const { return _kind == SCOPE_ASPECT; }
    bool is_class() const { return _kind == SCOPE_CLASS; }
    bool is_struct() const { return _kind == SCOPE_STRUCT; }
    bool is_union() const { return _kind == SCOPE_UNION; }
    bool is_namespace() const { return _kind == SCOPE_NAMESPACE; }
    bool is_class_or_struct() const { return is_class() || is_struct(); }
    bool is_cxx11attr() const { return _kind == ELEMENT_CXX11Attr; }
    std::string _name;
    ACM_Name *_jpm_link;
    bool operator < (const Node &compared) const { return _name < compared._name; }
  };
  struct Element : Node {
    Node *_referred; // for class aliases
  };
  struct Scope : Node {
    Scope *_parent_scope;
    bool _is_slice;
    std::set<Scope> _sub_scopes;
    std::set<Element> _elements;
    std::list<Scope*> _search_scopes; // base classes for classes/aspects
                                      // namespaces for namespaces
    void dump (int indent = 0) const {
      for (int i = indent; i >= 0; i--) { cout << "  "; }
      if (_is_slice)
        cout << "slice ";
      switch (_kind) {
      case SCOPE_NAMESPACE: cout << "namespace"; break;
      case SCOPE_CLASS:     cout << "class"; break;
      case SCOPE_ASPECT:    cout << "aspect"; break;
      case SCOPE_UNION:     cout << "union"; break;
      default:              cout << "unknown"; break;
      }
      cout << " " << _name << ": ";
      for (std::list<Scope*>::const_iterator i = _search_scopes.begin ();
          i != _search_scopes.end (); ++i)
        cout << " " << (*i)->_name;
      for (std::set<Element>::const_iterator i = _elements.begin ();
          i != _elements.end (); ++i)
        cout << " elem " << (*i)._name;
      cout << endl;
      for (std::set<Scope>::const_iterator i = _sub_scopes.begin ();
          i != _sub_scopes.end (); ++i)
        (*i).dump (indent + 1);
    }
  };

  Scope _root_scope;
  Scope *_curr_scope;
  ModelBuilder &_jpm;
  CodeWeaver &_code_weaver;
  bool _in_template;
  string _tunit_name;

  ACConfig &_conf;

  void skip_block (int open, int close, bool inclusive = true);
  void skip_round_block (bool inclusive = true);
  void skip_curly_block (bool inclusive = true);
  void skip_square_block (bool inclusive = true);
  void skip_template_params ();
  bool is_attribute_token (const ACToken &token, const ACToken &next) const;
  bool is_cxx11_attribute_token (const ACToken &token, const ACToken &next) const;
  bool is_gnu_attribute_token (const ACToken &token) const;
  void skip_attributes (bool inclusive = true);
  void skip_cxx11_attributes (bool inclusive = true);
  void skip_gnu_attributes (bool inclusive = true);

  // internal registration functions
  ACM_Name *register_scope (Scope &scope);
  void register_base_classes (Scope &cls);

  // internal parse functions
  void parse_scope (Scope &scope, std::string *anon_member_class_copy = 0);
  void parse_qual_name (std::vector<std::string> &names,
      bool &root_qualified, bool &contains_template_id);
  void parse_base_clause (Scope &class_scope);
  void parse_base_spec (Scope &class_scope);
  void parse_aspectof_function (const string &prot);
  void parse_using_directive (Scope &scope);
  void parse_pointcut_def (Scope &scope);
#ifdef FRONTEND_CLANG
  void parse_attribute_decl (Scope &scope);
#endif
  void parse_advice_def (Scope &scope, int &advice_no, int &intro_no, int &order_no,
      int &anon_slice_no, const std::string &prot);
  void parse_advice_body (AdviceCodeContext &context, int open, int close, CodeWeaver::TypeUse &uses_type );

  // internal name lookup functions
  Node *lookup_name (Scope &scope, bool root_qualified, const std::vector<std::string> &names);
  Node *lookup_name_in_scope (Scope &scope, const std::vector<std::string> &names,
      std::set<Scope*> &visited_scopes, int depth = 0);
  Scope *lookup_scope (Scope &scope, bool root_qualified, std::vector<std::string> &names);
#ifdef FRONTEND_PUMA
  Scope *find_slice (Scope &scope, Puma::Unit *rec);
#else
  Scope *find_slice (Scope &scope, ACPreprocessor::TokenVector &rec);
#endif
  ACM_Pointcut *lookup_pct_func (bool root_qualified, std::vector<std::string> &qual_name);
  ACM_Attribute *lookup_pct_attr(bool root_qualified, std::vector<string> &qual_name);

  // internal helper functions
  PointCutExpr *create_pct_expr_tree(const string &pct, ACM_Name *scope, ACToken pos);
#ifdef FRONTEND_PUMA
  void handle_slice_member (Scope &scope, Puma::Unit *rec, ACToken from, ACToken to);
  std::string recording_to_string (Puma::Unit *);
#else
  void handle_slice_member (Scope &scope, ACPreprocessor::TokenVector &rec, ACToken from, ACToken to);
  std::string recording_to_string (const ACPreprocessor::TokenVector &);
#endif
  bool is_visible_scope (Scope &scope, const std::string &name);
  std::string full_anon_class_name (Scope &scope) const;

#ifdef FRONTEND_CLANG
  TU_ClassSlice::SliceBody
  format_non_inline_member(ACPreprocessor::TokenVector &, const std::string &);
  void whitespace (unsigned &line, const ACToken &token, std::string &);
  void set_slice_tokens (TU_ClassSlice *, ACPreprocessor::TokenVector &);
#endif

public:
#ifdef FRONTEND_PUMA
  Phase1 (ModelBuilder &jpm, const string &tunit_name, Puma::CProject &project, ACConfig &conf,
      CodeWeaver &code_weaver) :
        ACPreprocessor(tunit_name, conf, project), _curr_scope (0), _jpm (jpm),
        _code_weaver (code_weaver), _in_template (false), _tunit_name (tunit_name),
        _conf( conf ) {}
  // run the analysis and transformation
  // result: -1 error; 0 empty file; 1 OK
  int run (IncludeGraph &include_graph);
  Puma::Unit *unit() const { return ACPreprocessor::unit (); }

#else
  Phase1 (ModelBuilder &jpm, const string &tunit_name, ACProject &project, ACConfig &conf,
          CodeWeaver &code_weaver, IncludeGraph &include_graph) :
  ACPreprocessor(tunit_name, conf, include_graph, project), _curr_scope (0),
        _jpm (jpm), _code_weaver (code_weaver), _in_template (false),
        _tunit_name (tunit_name), _conf( conf ) {
  }
  // run the analysis and transformation
  // result: -1 error; 0 empty file; 1 OK
  int run ();
#endif
};

#endif // __Phase1_h__
