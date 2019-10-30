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

#ifndef __ClangIntroducer_h__
#define __ClangIntroducer_h__

// Class that is used for introductions during a parser run.
// The semantics object has a reference to an Introducer and uses
// the interface at some well defined (join) points.

//#include "clang/Sema/Sema.h"
#include "ACFileID.h"
#include "ACErrorStream.h"
#include "ACToken.h"

#include "clang/Basic/Version.h"

#include <set>

#include "ClangAnnotation.h"
using std::set;
#include <map>
using std::map;
#include <deque>

namespace clang {
  class CompilerInstance;
  class Parser;
  class Decl;
  class RecordDecl;
  class FunctionDecl;
  class TagDecl;
  class TemplateDecl;
  class TemplateArgument;
  class CXXRecordDecl;
  class ClassTemplatePartialSpecializationDecl;
  class TemplateDecl;
  class DeclaratorDecl;
}

class ACConfig;
class IncludeGraph;
class CodeWeaver;
class ACM_Class;
class ACM_Introduction;
class Plan;
class ModelBuilder;
class ACUnit;
class LineDirectiveMgr;
class IntroductionUnit;
class ClangIntroParser;
class WeavePos;

class ClangIntroducer {
  Plan &_plan;
  CodeWeaver &_code_weaver;
  ModelBuilder &_jpm;
  IncludeGraph &_ig;
  ACConfig &_conf;
  set<ACFileID> _included_aspect_headers;
  typedef map<clang::Decl *, ACM_Class *> ClangTargetMap;
  ClangTargetMap _clangtargets;
    
  // find a member that is/must be link-once (global) code
  const clang::Decl *link_once_object (clang::CXXRecordDecl *ci);

  // Various sets used to avoid duplicated intros.
  std::set<clang::Decl *> _seen_classes;
  std::set<clang::Decl *> _classes_with_introduced_bases;
  std::set<clang::Decl *> _classes_with_introduced_members;
  std::map<clang::Decl *, std::deque<IntroductionUnit*> > _members_to_introduce;
  std::set<clang::Decl *> _classes_with_introduced_non_inlines;

  std::map<clang::Decl *, clang::SourceLocation> _original_class_endings;

  // A map of all generated IntroductionUnits. To be used to resolve intros at
  // the end.
  std::map<IntroductionUnit *, std::vector<IntroductionUnit *> > _intros;

public:
  // Contructor: every Introducer knows the parser
  ClangIntroducer(Plan &pl, CodeWeaver &ccw, ModelBuilder &jpm,
                  IncludeGraph &ig, ACConfig &c)
      : _plan(pl), _code_weaver(ccw), _jpm(jpm), _ig(ig), _conf(c) {}

  // Destructor: release all allocated resources
  ~ClangIntroducer ();
  
  clang::CompilerInstance *_ci;
  ClangIntroParser *_parser;

  ModelBuilder &get_model_builder () const { return _jpm; }

  // Parse an include.
  void parse (clang::Parser *parser, llvm::StringRef include,
              clang::SourceLocation loc, clang::DeclContext *context = 0);
  // Inject a buffer into the parser.
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  clang::FileID inject(const llvm::MemoryBuffer *mb, clang::SourceLocation loc,
      bool add_eof = false);
#else // C++ 11 interface
  clang::FileID inject(std::unique_ptr<llvm::MemoryBuffer> mb,
      clang::SourceLocation loc, bool add_eof = false);
#endif

  // Clang introducer methods
  void tunit_start ();
  void base_specs_end (clang::Decl *decl);
  void class_start (clang::Decl *decl, clang::SourceLocation lbrac);
  void class_end (clang::Decl *decl, clang::SourceLocation rbrac);
  void function_start (clang::FunctionDecl *decl);
  void declarator_start (clang::DeclaratorDecl *decl);
  void tunit_end ();

  virtual bool end_translation_unit(clang::Parser *parser);

  std::vector<IntroductionUnit *> &
  get_intros_for(IntroductionUnit *unit) {
    return _intros[unit];
  }

private:

  // introduce base class into the class referenced by 'decl'
  // 'lbrac' is the position of the opening brace of the class body
  // iff 'first' is true, the injected code will start with ":"
  void introduce_base_classes (clang::Decl *decl, clang::SourceLocation lbrac,
      bool first);

  // check in the plan if the current class needs an introduction
  ACM_Class *plan_lookup (clang::Decl *ci);

  // create the weaving plan for a given class
  ACM_Class *create_plan (clang::Decl *ci);
  
  // insert introspection code
  //  * at the end of class definitions, after AspectC++ introductions
  //  * before a slice, if "precedence" is given
  //void insert_introspection_code (Puma::CT_ClassDef *cd, int precedence = -1);

  // insert friend declarations for all aspects
  bool insert_aspect_friend_decls (std::string &str, clang::Decl *ci);

  // insert friend declarations for all aspects
  bool insert_aspectof_function (std::string &str, clang::CXXRecordDecl *ci);

  // insert introspection code
  //  * at the end of class definitions, after AspectC++ introductions
  //  * before a slice, if "precedence" is given
  std::string insert_introspection_code(clang::CXXRecordDecl *cd,
                                        int precedence = -1);

  // insert include files from units into the parser
  void handle_includes(set<ACFileID> &units, clang::SourceManager &sm,
                       clang::Parser *parser, clang::Decl *decl,
                       clang::SourceLocation CurLoc, ACErrorStream &err);

  // generate member intros for a given class
  void gen_intros (ACM_Class *jpl,
                   std::deque<IntroductionUnit *> &units,
                   ACErrorStream &err,
                   clang::TagDecl *target, int introduction_depth,
                   bool non_inline = false) const;

  // generate base class intros for a given class
  void gen_base_intros (ACM_Class *jpl,
                        std::deque<IntroductionUnit *> &units,
                        bool commas_only = false) const;

  void gen_base_intro (std::string &unit,
                       ACM_Introduction *ii, bool first) const;

  // update the base classes in the project model for a class that had
  // base class introductions
  void update_base_classes (clang::Decl *decl);
};

#endif /* __ClangIntroducer_h__ */
