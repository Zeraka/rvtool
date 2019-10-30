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

#ifndef __PumaIntroducer_h__
#define __PumaIntroducer_h__

// Class that is used for introductions during a parser run.
// The semantics object has a reference to an Introducer and uses
// the interface at some well defined (join) points.

#include "Puma/ACIntroducer.h"

#include <set>
using std::set;
#include <list>
using std::list;
#include <map>
using std::map;

namespace Puma {
  class CT_ClassDef;
  class CT_Program;
  class CCParser;
  class PreprocessorParser;
  class CClassInfo;
  class TokenProvider;
  class TokenStream;
}

#include "ACErrorStream.h"

class ACConfig;
class IncludeGraph;
class CodeWeaver;
class JPP_Class;
class ACM_Class;
class ACM_Introduction;
class Plan;
class ModelBuilder;
class ACUnit;
class LineDirectiveMgr;
class IntroductionUnit;

class PumaIntroducer : public Puma::ACIntroducer {
  Plan &_plan;
  CodeWeaver &_code_weaver;
  Puma::CCParser *_parser;
  ModelBuilder &_jpm;
  int _intro_level;
  Puma::PreprocessorParser *_cpp;
  IncludeGraph &_ig;
  ACConfig &_conf;
  set<const Puma::Unit*> _included_aspect_headers;
  list<Puma::CTree*> _ah_trees;
  typedef map<Puma::CClassInfo*,ACM_Class*> TargetMap;
  TargetMap _targets;
  list<Puma::TokenProvider*> _token_providers;
  list<Puma::TokenStream*> _token_streams;
    
  // find a member that is/must be link-once (global) code
  static Puma::CObjectInfo *link_once_object (Puma::CClassInfo *ci);

  // check whether an object was introduced
  static bool is_intro (Puma::CObjectInfo *obj);

  // parse code that shall be introduced
  Puma::CTree *parse (list<Puma::Unit*> units, bool (Puma::CCSyntax::*rule)(),
    const char *expected_id, ACErrorStream &err);
    
  // checks if an attribute that us returned by the parser is an attribute
  // in the sense of the AspectC++ introspection mechnism
  bool is_attribute (Puma::CAttributeInfo *obj);

public:
  // Contructor: every Introducer knows the parser
  PumaIntroducer (Plan &pl, CodeWeaver &cw, Puma::CCParser &p, ModelBuilder &jpm,
    IncludeGraph &ig, ACConfig &c) : _plan (pl), _code_weaver (cw),
    _parser (&p), _jpm (jpm), _intro_level (0), _cpp (0), _ig (ig), _conf(c) {}
  
  // Destructor: release all allocated resources
  ~PumaIntroducer ();
  
  // called when a new class/union/struct/aspect is created, current scope
  // is the global scope
  virtual void class_before (Puma::CT_ClassDef*);

  // called when a new class/union/struct/aspect is created
  virtual void class_begin (Puma::CT_ClassDef*);

  // called when a new class/union/struct/aspect definition ends
  // (still in the class scope)
  virtual void class_end (Puma::CT_ClassDef*);
  
  // called after the parser tried to parse a base clause
  virtual void base_clause_end (Puma::CT_ClassDef*, Puma::Token*);

  // called after the program has been parsed completely
  virtual void trans_unit_end (Puma::CT_Program *);

  // get the list of aspect header syntax trees
  list<Puma::CTree*> &ah_trees () { return _ah_trees; }

private:

  // check in the plan if the current class needs an introduction
  ACM_Class *plan_lookup (Puma::CClassInfo *ci);

  // create the weaving plan for a given class and return the class object
  ACM_Class *create_plan (Puma::CClassInfo *ci);
  
  // insert introspection code
  //  * at the end of class definitions, after AspectC++ introductions
  //  * before a slice, if "precedence" is given
  void insert_introspection_code (Puma::CT_ClassDef *cd, int precedence = -1);

  // insert friend declarations for all aspects
  bool insert_aspect_friend_decls (ACUnit &unit, Puma::CRecord *rec);

  // insert friend declarations for all aspects
  bool insert_aspectof_function (ACUnit &unit, Puma::CClassInfo *ci,
      ACErrorStream &err);

  // manage the intro nesting level and the _cpp pointer
  void enter ();
  void leave ();
  
  // generate member intros for a given class
  void gen_intros (ACM_Class *jpl, list<Puma::Unit*> &units, ACErrorStream &err,
      Puma::CStructure *target, LineDirectiveMgr &lmgr, int introduction_depth,
      bool non_inline = false) const;

  // generate base class intros for a given class
  void gen_base_intros (ACM_Class *jpl, list<Puma::Unit*> &units, ACErrorStream &err,
      Puma::CClassInfo *target, LineDirectiveMgr &lmgr) const;

  void gen_base_intro (IntroductionUnit &intro_unit,
    ACM_Introduction *ii, bool first) const;
};

#endif /* __PumaIntroducer_h__ */
