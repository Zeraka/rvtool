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

#include <iostream>
using std::cout;
using std::endl;
#include <set>
using std::set;

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/Sema/Template.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/Parser.h"
#include "clang/AST/Attr.h"
#include "llvm/ADT/StringExtras.h"

#include "ClangIntroducer.h"
#include "ACFileID.h"
#include "Plan.h"
#include "CodeWeaver.h"
#include "LineDirectiveMgr.h"
#include "IncludeGraph.h"
#include "Naming.h"
#include "AspectInfo.h"
#include "OrderInfo.h"
#include "IntroductionInfo.h"
#include "ModelBuilder.h"
#include "ACModel/Utils.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "IntroductionUnit.h"
#include "TransformInfo.h"
#include "ClangIntroParser.h"
#include "ClangIntroSema.h"
#include "ClangAnnotation.h"
#include "version.h"

using namespace Puma;

// Destructor: release all allocated resources
ClangIntroducer::~ClangIntroducer () {
}

static const char *prot_str (clang::AccessSpecifier prot) {
  switch (prot) {
  case clang::AS_private: return "AC::PROT_PRIVATE";
  case clang::AS_protected: return "AC::PROT_PROTECTED";
  case clang::AS_public: return "AC::PROT_PUBLIC";
  case clang::AS_none: return "AC::PROT_NONE";
  }
  return 0;
}

static string spec_str (bool is_static, bool is_mutable, bool is_virtual) {
  int pos = 0;
  string result;
  if (is_static) {
    if (pos++) result += "|";
    result += "AC::SPEC_STATIC";
  }
  if (is_mutable) {
    if (pos++) result += "|";
    result += "AC::SPEC_MUTABLE";
  }
  if (is_virtual) {
    if (pos++) result += "|";
    result += "AC::SPEC_VIRTUAL";
  }
  if (!pos)
    result = "AC::SPEC_NONE";
  return result;
}

// insert friend declarations for all aspects
bool ClangIntroducer::insert_aspectof_function(std::string &str,
                                          clang::CXXRecordDecl *ci) {
  if (!ci)
    return false;

  ACM_Aspect *jpl_aspect = _jpm.register_aspect (ci);
  if (!jpl_aspect)
    return false;

  // iterate over all member functions and find any definitions of
  // 'aspectof' or 'aspectOf'
  clang::NamedDecl *aspectOf_func = 0;
  clang::NamedDecl *aspectof_func = 0;
  for (clang::CXXRecordDecl::decl_iterator di = ci->decls_begin(),
                                           de = ci->decls_end();
       di != de; ++di) {
    clang::NamedDecl *nd = llvm::dyn_cast<clang::NamedDecl> (*di);
    if (!nd)
      continue;
    if (nd->getKind () == clang::Decl::FunctionTemplate ||
        nd->getKind () == clang::Decl::CXXMethod) {
      if (nd->getNameAsString() == "aspectof")
        aspectof_func = nd;
      else if (nd->getNameAsString() == "aspectOf")
        aspectOf_func = nd;
    }

    // Register all members now. Some may not be in the model yet because we're
    // in the middle of parsing classes.
    clang::CXXMethodDecl *md = llvm::dyn_cast<clang::CXXMethodDecl> (nd);
    if (md) {
      if (!_jpm.register_pointcut(md, jpl_aspect))
        _jpm.register_function(md, jpl_aspect);
    }
  }

  // Don't create instances of abstract aspects.
  if (::is_abstract(*jpl_aspect))
    return false;

#if 0
  if (aspectOf_func && !aspectof_func) {
    // rename the function to 'aspectof'
    CT_FctDef *fctdef = (CT_FctDef*)aspectOf_func->Tree ();
    CT_SimpleName *name = ((CT_Declarator*)fctdef->Declarator ())->Name ();
    name = name->Name (); // if it is qualified
    const WeavePos &name_start =
      _code_weaver.weave_pos (ACToken (name->token ()), WeavePos::WP_BEFORE);
    const WeavePos &name_end =
      _code_weaver.weave_pos (ACToken (name->end_token ()), WeavePos::WP_AFTER);
    _code_weaver.replace (name_start, name_end, "aspectof");
    return false; // nothing to introduce
  }
#endif

  if (!aspectOf_func && !aspectof_func) {
    llvm::raw_string_ostream unit(str);
    unit << '\n'
      << "public:" << '\n'
      << "  static " << ci->getName () << " *aspectof () {" << '\n'
      << "    static " << ci->getName () << " __instance;" << '\n'
      << "    return &__instance;" << '\n'
      << "  }" << '\n'
      << "  static " << ci->getName () << " *aspectOf () {" << '\n'
      << "    return aspectof ();" << '\n'
      << "  }" << '\n'
      << "private:" << '\n';
    return true;
  }
  return false;
}

bool ClangIntroducer::insert_aspect_friend_decls(std::string &str,
                                            clang::Decl *decl) {
  if (!decl)
    return false;

  clang::TagDecl *td = llvm::dyn_cast<clang::TagDecl>(decl);
  // introductions into unions require some extra effort (not implemented yet)
  if (td && td->isUnion())
    return false;

  // check whether the insertion is indicated for this class or union
  clang::SourceManager &sm = _code_weaver.getRewriter().getSourceMgr();

  // no need for template instances; the templates already get the friend injection
  if (clang::CXXRecordDecl *r = llvm::dyn_cast<clang::CXXRecordDecl>(decl))
    if (r->getTemplateSpecializationKind() ==
            clang::TSK_ImplicitInstantiation ||
        r->getTemplateSpecializationKind() ==
            clang::TSK_ExplicitInstantiationDefinition)
    return false;

  clang::NamedDecl* nd = llvm::dyn_cast<clang::NamedDecl>(decl);
  if (nd) {
    std::string qual_name(nd->getQualifiedNameAsString());
    llvm::StringRef name(nd->getName());

    // code introduced by phase 1 (in special namespace AC) is not modified here
    if (llvm::StringRef(qual_name).startswith("AC::"))
      return false;

    // introspection code is also not modified
    else if ( ((name == "BaseClass") ||
               (name == "Member") ||
               (name == "Function") ||
               (name == "Constructor") ||
               (name == "Destructor")) &&
               (llvm::StringRef(qual_name).count("::__TI::") ||
                llvm::StringRef(qual_name).count("::__TJP_")))
        return false;
  }

  // nested classes in template instances should also not be modified to avoid double injection
  //if (_jpm.inside_template_instance(decl))
    //return false;
  // the class has to belong to the project
  llvm::StringRef name = sm.getPresumedLoc(decl->getLocation()).getFilename();
  llvm::StringRef buffer_name = sm.getBufferName(decl->getLocation());
  if (!buffer_name.startswith("<intro") &&
      !name.empty() && !_jpm.get_project().isBelow(name.str().c_str()))
    return false;

  // OK, perform the insertion ...

  // get all aspects from the join point model
  ProjectModel::Selection all_aspects;
  _jpm.select (JPT_Aspect, all_aspects);

  // generate the list of aspects
  // also make sure that an aspect is not friend of itself!
  bool result = false;
  for (ProjectModel::Selection::iterator iter = all_aspects.begin ();
       iter != all_aspects.end (); ++iter) {
    ACM_Aspect &jpl = (ACM_Aspect&)**iter;
    if (!(td && td->getQualifiedNameAsString() == jpl.get_name())) {
      str += "  friend class ::";
      str += jpl.get_name();
      str += ";\n";
      result = true;
    }
  }

  return result;
}

static bool is_attribute(clang::DeclaratorDecl *dd) {
  if (clang::FieldDecl *fd = llvm::dyn_cast<clang::FieldDecl>(dd))
    return !fd->isBitField();
  return true;
}

// insert introspection code
//  * at the end of class definitions, after AspectC++ introductions
std::string ClangIntroducer::insert_introspection_code(clang::CXXRecordDecl *cd,
                                                       int precedence) {
  std::string str;
  llvm::raw_string_ostream unit(str);

  if (!cd /*|| !cd->Object ()->DefObject ()->ClassInfo()*/)
    return "";

  // return if this class is not an introduction target or has C linkage
  if (!_jpm.is_valid_model_class (cd) || !_jpm.is_intro_target (cd))
    return "";

  // introspection templates cannot be declared inside of a local class
  if (cd->isLocalClass()) // integrate check into _jpm.is_valid_model_class ?
    return "";
  
  // Return if this class is defined in an extern "C" context
  if (TI_Class::is_extern_c(cd))
    return "";
//  if (clang::LinkageSpecDecl *SD =
//          llvm::dyn_cast<clang::LinkageSpecDecl>(cd->getDeclContext()))
//    if (SD->getLanguage() == clang::LinkageSpecDecl::lang_c)
//      return "";

  unit << "public:\n";
  unit << "  struct ";
  if (precedence == -1)
    unit << "__TI";
  else
    unit << "__TJP_" << precedence;
  unit << " {\n";

  // generate the signature (fully qualified target class name)
  unit << "    static const char *signature () { return \"";
  unit << cd->getQualifiedNameAsString();
  unit << "\"; }\n";

  // generate a 32-bit hash code from the fully qualified target class name
  unit << "    enum { HASHCODE = "
       << StrCol::hash(cd->getQualifiedNameAsString().c_str()) << "U };\n";

  // generate a typedef for the target type
  unit << "    typedef " << *cd << " That;\n";

  // generate a list with all base classes
  unit << "    template<int I, int __D=0> struct BaseClass {};\n";
  unsigned b = 0;
  for (clang::CXXRecordDecl::base_class_iterator I = cd->bases_begin(),
                                                 E = cd->bases_end();
       I != E; ++I) {
    unit << "    template <int __D> struct BaseClass<" << b
         << ", __D> { typedef ";
    unit << TI_Type::get_type_text(I->getType(), &cd->getASTContext(), "Type", TSEF_ENABLE, true, TSEF_DONOTCHANGE, false, true, false);
    unit << "; ";
    unit << "static const AC::Protection prot = "
         << prot_str(I->getAccessSpecifierAsWritten()) << "; ";
    unit << "static const AC::Specifiers spec = (AC::Specifiers)("
          << spec_str (false, false, I->isVirtual()) << "); ";
    unit << "};\n";
    b++;
  }
  unit << "    enum { BASECLASSES = " << b << " };\n";

  llvm::StringRef clsname = cd->getName();
  // generate Member<I> => a list with all attribute types
  unit << "    template<int I, int __D=0> struct Member {};\n";
  unsigned e = 0;
  for (clang::DeclContext::decl_iterator I = cd->decls_begin(),
                                         E = cd->decls_end();
       I != E; ++I) {
    clang::DeclaratorDecl *dd = llvm::dyn_cast<clang::DeclaratorDecl>(*I);
    if (!dd || !is_attribute(dd))
      continue;

    // Here anonymous struct/class/union members are ignored
    // TODO: Handle all members of these anonymous constructs. They are visible
    //       in the enclosing scope, i.e. here!
    string name = dd->getNameAsString ();
    if (name.empty ())
      continue;

    // static members are represented as VarDecls, the rest is FieldDecl.
    clang::VarDecl *vd = llvm::dyn_cast<clang::VarDecl>(dd);
    clang::FieldDecl *fd = llvm::dyn_cast<clang::FieldDecl>(dd);
    if (!vd && !fd)
      continue;

    unit << "    template <int __D> struct Member<" << e << ", __D> { typedef ";
    unit << TI_Type::get_type_text(dd->getType(), &dd->getASTContext(), "Type", TSEF_ENABLE, true, TSEF_DONOTCHANGE, false, true, false);
    unit << "; typedef AC::Referred<Type>::type ReferredType; ";
    unit << "static const AC::Protection prot = " << prot_str(dd->getAccess())
         << "; ";
    unit << "static const AC::Specifiers spec = (AC::Specifiers)("
         << spec_str(vd, fd && fd->isMutable(), false) << "); ";

    // generate pointer => the typed pointer to attribute I
    unit << "static ReferredType *pointer (" << clsname
         << " *obj = 0) { return (ReferredType*)&";
    if (vd)
      unit << clsname << "::" << *dd;
    else
      unit << "obj->" << *dd;
    unit << "; } ";
    // generate member_name => the name of attribute i
    unit << "static const char *name () { return \"" << *dd << "\"; }\n";
    unit << " };\n";
    e++;
  }
  unit << "    enum { MEMBERS = " << e << " };\n";

  // generate a list with all member functions
  unit << "    template<int I, int __D=0> struct Function {};\n";
  unit << "    template<int I, int __D=0> struct Constructor {};\n";
  unit << "    template<int I, int __D=0> struct Destructor {};\n";
  unsigned int functions=0, constructors=0, destructors=0;
  for (clang::CXXRecordDecl::method_iterator I = cd->method_begin(),
                                             E = cd->method_end();
       I != E; ++I) {
    clang::CXXMethodDecl *fi = *I;

    if (!fi || !_jpm.is_valid_model_function(fi) || !fi->isUserProvided())
      continue;

    if (llvm::dyn_cast<clang::CXXConstructorDecl>(fi)) {
      unit << "    template <int __D> struct Constructor<" << constructors << ", __D> { ";
      constructors++;
    }
    else if (llvm::dyn_cast<clang::CXXDestructorDecl>(fi)) {
      unit << "    template <int __D> struct Destructor<" << destructors << ", __D> { ";
      destructors++;
    }
    else {
      unit << "    template <int __D> struct Function<" << functions << ", __D> { ";
      functions++;
    }

    unit << "static const AC::Protection prot = " << prot_str (fi->getAccess ()) << "; ";
    unit << "static const AC::Specifiers spec = (AC::Specifiers)("
          << spec_str (false, false, fi->isVirtual()) << "); ";

    unit << "};\n";
  }
  unit << "    enum { FUNCTIONS = " << functions << " };\n";
  unit << "    enum { CONSTRUCTORS = " << constructors << " };\n";
  unit << "    enum { DESTRUCTORS = " << destructors << " };\n";

  unit << "  };\n";
#if 0
  // paste a #line directive
  LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();
  ACUnit *dunit = new ACUnit (err);
  lmgr.directive (*dunit, inspos.unit (), inspos);
  *dunit << endu;
  if (dunit->empty ()) delete dunit; else _code_weaver.insert (pos, dunit);
#endif
  return unit.str();
}


ACM_Class *ClangIntroducer::create_plan (clang::Decl *decl) {
  
  clang::RecordDecl *ci = llvm::dyn_cast<clang::RecordDecl>(decl);
  if (!ci)
    return 0;

  // return if this class is not an introduction target
  if (!_jpm.is_intro_target (ci))
    return 0;

  // try to register this class (might be a newly introduced class)
  ACM_Class *jpl = _jpm.register_aspect (ci);
  if (!jpl) jpl = _jpm.register_class (ci);

  // return if this is either not a valid model class or no valid intro target
  if (!jpl || !jpl->get_intro_target ())
    return 0;
  
  // iterate through all introduction advice in the plan
  PointCutContext context (_jpm, _conf);
  const list<IntroductionInfo*> &intros = _plan.introduction_infos ();
  for (list<IntroductionInfo*>::const_iterator i = intros.begin ();
       i != intros.end (); ++i) {
    IntroductionInfo *intro = *i;
    // TODO: consider stand-alone advice here as well in the future (C-mode)
    // something like ... if (!intro->is_activated ()) continue;
    context.concrete_aspect (intro->aspect ());
    Binding binding;     // binding and condition not used for intros
    Condition condition;
    PointCutExpr *pce = (PointCutExpr*)intro->pointcut_expr ().get ();
    if (pce->match (*jpl, context, binding, condition))
      _plan.consider (*jpl, &intro->intro ());
  }

  if (jpl->has_plan ()) {
    // order the advice & check
    _plan.order (jpl);

    // remember the class info and join point location
    _clangtargets.insert (ClangTargetMap::value_type (ci, jpl));
  }
  return jpl;
}


ACM_Class *ClangIntroducer::plan_lookup (clang::Decl *ci) {
  // Lazily create the plan.
  if (_seen_classes.insert(ci).second)
    create_plan (ci);

  ClangTargetMap::iterator i = _clangtargets.find (ci);
  if (i != _clangtargets.end () && i->second->has_plan())
    return i->second;
  return 0;
}

const clang::Decl *ClangIntroducer::link_once_object (clang::CXXRecordDecl *ci) {
  clang::SourceManager &sm = _code_weaver.getRewriter().getSourceMgr();
  // Functions before fields.
  for (clang::CXXRecordDecl::decl_iterator DI = ci->decls_begin(),
                                           DE = ci->decls_end();
       DI != DE; ++DI) {
    // Skip generated code.
    llvm::StringRef name = sm.getBufferName((*DI)->getLocation());
    if (name.startswith("<intro"))
      continue;

    if (clang::CXXMethodDecl *MD = llvm::dyn_cast<clang::CXXMethodDecl>(*DI)) {
      // skip template functions and built-in functions
      // they don't need link-once code
      if (!MD->isUserProvided())
        continue;
      // if a member function is undefined it is link-once code!
      if (!MD->isDefined())
        return MD;
      // if the function is defined, outside the class scope, and is not inline,
      // we found the implementation
      const clang::FunctionDecl *def;
      if (MD->getBody(def) && def->getLexicalParent() != ci &&
          !def->isInlineSpecified())
        return def;
    }
  }

  for (clang::CXXRecordDecl::decl_iterator DI = ci->decls_begin(),
                                           DE = ci->decls_end();
       DI != DE; ++DI) {
    // Skip generated code.
    llvm::StringRef name = sm.getBufferName((*DI)->getLocation());
    if (name.startswith("<intro"))
      continue;

    // Look for uninitialized static members with out of line definition.
    if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(*DI))
      if (VD->isStaticDataMember() && !VD->getInit() &&
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
          VD->getDefinition())
#else
          VD->getOutOfLineDefinition())
#endif
        return VD;
  }
  return 0;
}

void ClangIntroducer::parse (clang::Parser *parser, llvm::StringRef include,
                        clang::SourceLocation loc, clang::DeclContext *context) {
//  cout << "parsing '" << include.data() << "'" << endl;
  // if not demanded otherwise, parse the string in the global scope
  if (context == 0)
    context = _ci->getASTContext().getTranslationUnitDecl ();
  WeaverBase::MacroAnalyzerState mas = _code_weaver.save_macro_analyzer_state();
  clang::Token &token = (clang::Token &)parser->getCurToken();
  clang::Token saved_token = token;
  // EXPERIMENT:
  string fname = _ci->getSourceManager().getFilename(_ci->getSourceManager().getExpansionLoc(loc));
  static int num = 0;
  ostringstream name;
  name << fname << "_virt" << num++;
  ACFileID vfid = _conf.project().addVirtualFile(name.str(), include);
  clang::FileID fid = _ci->getSourceManager().createFileID(vfid.file_entry(), loc, clang::SrcMgr::C_User);
  _ci->getPreprocessor()
      .EnterSourceFile(fid, _ci->getPreprocessor().GetCurDirLookup(), loc);
//  inject (llvm::MemoryBuffer::getMemBufferCopy(include, "<intro-includes>"), loc);
  // EXPERIMENT end
  _ci->getPreprocessor ().Lex (token);
  ClangIntroSema &sema = (ClangIntroSema&)_ci->getSema();
  clang::Scope *old_scope = sema.setCurScope(sema.TUScope);
  clang::DeclContext *old_context = sema.CurContext;
  clang::DeclContext *old_lex_context = sema.OriginalLexicalContext;
  _ci->getSema().CurContext = context;
  clang::Parser::DeclGroupPtrTy Result;
  while (token.isNot(clang::tok::r_brace) &&
         !parser->ParseTopLevelDecl(Result)) {
    // Tell the ModelBuilder about the new declarations.
    sema.getASTConsumer().HandleTopLevelDecl(Result.get());
  }
  _code_weaver.restore_macro_analyzer_state(mas);
  sema.CurContext = old_context;
  sema.OriginalLexicalContext = old_lex_context;
  sema.setCurScope(old_scope);
  token = saved_token;
  _conf.project().removeVirtualFile(vfid);
//  cout << "parsing done" << endl;
}

#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  clang::FileID ClangIntroducer::inject(const llvm::MemoryBuffer *mb, clang::SourceLocation loc,
      bool add_eof) {
    clang::FileID fid = _ci->getSourceManager()
        .createFileIDForMemBuffer(mb, clang::SrcMgr::C_User, 0, 0, loc);
#else // C++ 11 interface
  clang::FileID ClangIntroducer::inject(std::unique_ptr<llvm::MemoryBuffer> mb,
      clang::SourceLocation loc, bool add_eof) {
    clang::FileID fid = _ci->getSourceManager()
        .createFileID(std::move (mb), clang::SrcMgr::C_User, 0, 0, loc);
#endif
//  _ci->getPreprocessor()
//      .EnterSourceFile(fid, _ci->getPreprocessor().GetCurDirLookup(), loc);
  clang::Preprocessor &PP = _ci->getPreprocessor();
  bool Invalid;
  const llvm::MemoryBuffer *buff = _ci->getSourceManager().getBuffer(fid, &Invalid);
  if (!Invalid) { // invalid inclusion
//    if (string(buff->getBufferIdentifier ()) == "<tmp>")
//      return fid;
//    cout << "entered:" << endl;
//    cout << buff->getBufferStart() << endl;
//    cout << "--- Tokens ..." << endl;
    clang::Lexer RawLex(fid, buff, PP.getSourceManager(), PP.getLangOpts());
    RawLex.SetCommentRetentionState(false);
    RawLex.setParsingPreprocessorDirective(false);
    list<clang::Token> tokens;
    clang::Token RawToken;
    RawLex.LexFromRawLexer(RawToken);
    while (RawToken.isNot(clang::tok::eof)) {
      if (RawToken.is(clang::tok::hash) && RawToken.isAtStartOfLine()) {
        RawLex.setParsingPreprocessorDirective(true);
      }
      else if (RawToken.is(clang::tok::eod)) {
        RawLex.setParsingPreprocessorDirective(false);
      }
      else if (RawToken.is(clang::tok::raw_identifier)) {
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
        clang::IdentifierInfo *II = PP.getIdentifierInfo(clang::StringRef(RawToken.getRawIdentifierData(), RawToken.getLength()));
#else
        clang::IdentifierInfo *II = PP.getIdentifierInfo(RawToken.getRawIdentifier());
#endif
        if (II) {
          RawToken.setIdentifierInfo(II);
          RawToken.setKind(II->getTokenID());
        }
      }
      tokens.push_back(RawToken);
      RawLex.LexFromRawLexer(RawToken);
    }
    if (add_eof) {
      tokens.push_back(RawToken);
    }
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    llvm::MutableArrayRef<clang::Token> input(new clang::Token[tokens.size()], tokens.size());
    int pos = 0;
    for (list<clang::Token>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
      input[pos++] = *i;
    PP.EnterTokenStream(input, true);
#else
    clang::Token *input = new clang::Token[tokens.size()];
    int pos = 0;
    for (list<clang::Token>::const_iterator i = tokens.begin(); i != tokens.end(); ++i)
      input[pos++] = *i;
    PP.EnterTokenStream(input, tokens.size(), true, true);
#endif
  }
//  cout << "--- End" << endl;
  return fid;
}

void ClangIntroducer::class_end (clang::Decl *decl, clang::SourceLocation rbrac) {
  clang::TagDecl *td = llvm::dyn_cast<clang::TagDecl>(decl);
  clang::CXXRecordDecl *record_decl =
      llvm::dyn_cast<clang::CXXRecordDecl>(decl);
  clang::SourceManager &sm = _parser->getPreprocessor().getSourceManager();
  clang::SourceLocation CurLoc = rbrac;
  ACErrorStream &err = _conf.project().err();
  bool needs_introspection = _conf.introspection(); // --introspection given?

  if (td) {
    if (td->getName () == "__TI" || td->getName ().startswith("__TJP"))
      return;

    update_base_classes (decl);

    std::deque<IntroductionUnit *> bufs;
    // first check with the plan if there are intros for this class
    ACM_Class *jpl = plan_lookup(decl);
    if (jpl && jpl->has_plan())
      gen_intros(jpl, bufs, err, td, _conf.introduction_depth());

    // When we reach here, we're done with this decl.
    _classes_with_introduced_members.insert(decl);

    if (!bufs.empty()) {
      // Fill in the IntroductionUnit structure.
      clang::FileID fid = sm.getFileID(CurLoc);
      const llvm::MemoryBuffer *this_unit = sm.getBuffer(fid);
      std::vector<IntroductionUnit *> &intro_units =
          get_intros_for(IntroductionUnit::cast(this_unit));
      for (std::deque<IntroductionUnit *>::iterator i = bufs.begin(),
                                                    e = bufs.end();
           i != e; ++i) {
        (*i)->target_unit(this_unit);
        (*i)->location(CurLoc);
        intro_units.push_back(*i);
      }

      // Includes may be handled when introducing bases, don't do it twice.
      // We add all includes even if we don't add the member for it just yet.
      if (!_classes_with_introduced_bases.count(decl)) {
        // determine the units that should be included in front of the class
        set<ACFileID> units;
        ACM_ClassPlan *plan = plan_lookup(decl)->get_plan();
        typedef ACM_Container<ACM_MemberIntro, true> MContainer;
        for (MContainer::iterator i = plan->get_member_intros().begin();
             i != plan->get_member_intros().end(); ++i) {
          ACM_ClassSlice *cs = get_slice(*(*i)->get_intro());
          units.insert(TI_ClassSlice::of(*cs)->slice_unit());
        }
        typedef ACM_Container<ACM_BaseIntro, true> BContainer;
        for (BContainer::iterator i = plan->get_base_intros().begin();
             i != plan->get_base_intros().end(); ++i) {
          ACM_ClassSlice *cs = get_slice(*(*i)->get_intro());
          units.insert(TI_ClassSlice::of(*cs)->slice_unit());
        }

        handle_includes(units, sm, _parser, decl, CurLoc, err);
      }
    }

    for (std::deque<IntroductionUnit *>::iterator i = bufs.begin();
        i != bufs.end (); i++) {
      IntroductionUnit *iu = *i;

      // if needed, generate the introspection code for this introduction
      if (iu->jp_needed()) {
        std::string introspection =
            insert_introspection_code(record_decl, iu->precedence());
        if (!introspection.empty()) {
          iu->content (introspection + iu->content ());
          needs_introspection = true; // also generate the final introspection
        }
      }

      clang::Token &token = (clang::Token &)_parser->getCurToken();
      clang::Token saved_token = token;

      // The closing brace is swallowed by Clang.
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      inject(llvm::MemoryBuffer::getMemBuffer(" }", "<tmp>"), CurLoc);
#else // C++ 11 interface
      inject(std::move (llvm::MemoryBuffer::getMemBuffer(" }", "<tmp>")), CurLoc);
#endif

      // Do the actual injection.
      iu->file_id(inject(iu->buffer(), CurLoc));

      _ci->getPreprocessor ().Lex (token);
      _parser->ParseCXXClassBody(decl);
      token = saved_token;
    }
  }

  // insert introspection code for this class if the configuration option
  // --introspection was given or the Joinpoint type was used in an introduction
  // -> inserted at the end of the class -> all intros are visible
  std::string intro_text;
  if (needs_introspection)
    intro_text = insert_introspection_code(record_decl);

  // If this is the last insertion of a member (or there were no member intros)
  // introduce aspectof functions and friend decls.
  bool has_aspectof = insert_aspectof_function(intro_text, record_decl);
  std::string friends = "\n";
  bool has_friends = insert_aspect_friend_decls(friends, decl);

  if (has_friends)
    intro_text += friends;

  if (!intro_text.empty() || has_aspectof || has_friends) {

    clang::FileID fid = sm.getFileID(CurLoc);
    const llvm::MemoryBuffer *this_unit = sm.getBuffer(fid);

    // create the new unit for all this stuff
    IntroductionUnit *unit = new IntroductionUnit(IntroductionUnit::IU_OTHER);
    unit->target_unit(this_unit);
    unit->location(CurLoc);
    unit->intro (0); // there is no specific introduction that triggered this
    unit->content(intro_text);

    // store a pointer to the new intro unit
    std::vector<IntroductionUnit *> &intro_units =
        get_intros_for(IntroductionUnit::cast(this_unit));
    intro_units.push_back(unit);

    clang::Token &token = (clang::Token &)_parser->getCurToken();
    clang::Token saved_token = token;

    // The closing brace is swallowed by Clang.
    inject(llvm::MemoryBuffer::getMemBuffer(" }", "<tmp>"), CurLoc);

    // Do the actual injection.
    unit->file_id(inject(unit->buffer(), CurLoc));

    _ci->getPreprocessor ().Lex (token);
    _parser->ParseCXXClassBody(decl);
    token = saved_token;
  }
}

void ClangIntroducer::introduce_base_classes(clang::Decl *decl,
    clang::SourceLocation CurLoc, bool first) {

  // first check with the plan if there are intros for this class
  ACM_Class *jpl = plan_lookup (decl);
  if (!jpl || !jpl->has_plan ()) {
    return;
  }

  std::deque<IntroductionUnit *> bufs;
  gen_base_intros (jpl, bufs, !first);

  if (bufs.empty())
    return;

  // Introduce bases only once.
  if (!_classes_with_introduced_bases.insert(decl).second)
    return;

  string signature = ((clang::TagDecl*)decl)->getNameAsString ();
  clang::SourceManager &sm = _parser->getPreprocessor().getSourceManager();

  set<ACFileID> units;
  ACM_ClassPlan *plan = jpl->get_plan ();
  typedef ACM_Container<ACM_MemberIntro, true> MContainer;
  for (MContainer::iterator i = plan->get_member_intros().begin();
       i != plan->get_member_intros().end(); ++i) {
    ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
    units.insert (TI_ClassSlice::of (*cs)->slice_unit ());
    }
  typedef ACM_Container<ACM_BaseIntro, true> BContainer;
  for (BContainer::iterator i = plan->get_base_intros().begin();
       i != plan->get_base_intros().end(); ++i) {
    ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
    units.insert (TI_ClassSlice::of (*cs)->slice_unit ());
    }

  ACErrorStream &err = _conf.project ().err ();
  handle_includes(units, sm, _parser, decl, CurLoc, err);

  clang::Token &token = (clang::Token &)_parser->getCurToken();
  clang::Token saved_token = token;

  inject(llvm::MemoryBuffer::getMemBuffer(" {", "<hack>"), CurLoc);

  const llvm::MemoryBuffer *target = sm.getBuffer(sm.getFileID(CurLoc));
  for (std::deque<IntroductionUnit *>::reverse_iterator ui =
       bufs.rbegin (); ui != bufs.rend (); ++ui) {
    (*ui)->target_unit(target);
    (*ui)->location(CurLoc);
    (*ui)->file_id (inject ((*ui)->buffer(), CurLoc));
  }
  std::vector<IntroductionUnit *> &intro_units =
      get_intros_for(IntroductionUnit::cast(target));
  intro_units.insert(intro_units.end(), bufs.begin(), bufs.end());

  if (!first) {
    // read the comma into the current 'token'. It will be overwritten by ':'
    // after the following code injection and 'Lex' call. -> a bit tricky :-)
    _ci->getPreprocessor ().Lex (token);
    // satisfy ParseBaseClause function: code has to start with ':'
    inject(llvm::MemoryBuffer::getMemBuffer(" :", "<hack>"), CurLoc);
  }

  _ci->getPreprocessor ().Lex (token);
  _parser->ParseBaseClause(decl);

  token = saved_token;
  return;
}

void ClangIntroducer::base_specs_end (clang::Decl *decl) {
  clang::SourceLocation CurLoc = _parser->getCurToken().getLocation();
  introduce_base_classes(decl, CurLoc, false);
}

void ClangIntroducer::class_start (clang::Decl *decl, clang::SourceLocation lbrac) {
  // first check with the plan if there are intros for this class
  clang::RecordDecl *ci = llvm::dyn_cast<clang::RecordDecl>(decl);
  if (!ci)
    return;

  // return if this class is not an introduction target
  if (!_jpm.is_intro_target (ci))
    return;

  // try to register this class (might be a newly introduced class)
  ACM_Class *jpl = _jpm.register_aspect (ci);
  if (!jpl) jpl = _jpm.register_class (ci);
  if (!jpl) return;

  // store the lbrace location for later; Clang has no method to get it.
  TI_Class::of(*jpl)->set_lbrace_loc(lbrac);

  // check and register attributes in the project model
  if(_conf.attributes())
    _jpm.handle_attributes(jpl, decl);

  introduce_base_classes(decl, lbrac, true);
}


// update the base classes in the project model for a class that had
// base class introductions
void ClangIntroducer::update_base_classes (clang::Decl *decl) {
  clang::CXXRecordDecl *rd = llvm::cast<clang::CXXRecordDecl>(decl);
  if (!rd || !_jpm.is_intro_target (decl)) return;
  ACM_Class *jpl = _jpm.register_aspect (rd);
  if (!jpl) jpl = _jpm.register_class (rd);
  if (!jpl) return;
  // FIXME: what about the derived-class relation? Will there be duplicates?
  jpl->get_bases().clear();
  for (clang::CXXRecordDecl::base_class_iterator I = rd->bases_begin(),
                                                 E = rd->bases_end();
       I != E; ++I) {
    clang::CXXRecordDecl *base_decl = I->getType ()->getAsCXXRecordDecl ();
    assert (base_decl);
    // re-register the base class in order to find the model element
    ACM_Class *base_jpl = _jpm.register_aspect (base_decl);
    if (!base_jpl) base_jpl = _jpm.register_class (base_decl);
    if (!base_jpl) continue;
    // insert the class into the model
    jpl->get_bases().insert(base_jpl);
    base_jpl->get_derived().insert(jpl);
    // handle this base class recursively
    update_base_classes(base_decl);
  }
}

void ClangIntroducer::handle_includes(set<ACFileID> &units,
                                 clang::SourceManager &sm,
                                 clang::Parser *parser, clang::Decl *decl,
                                 clang::SourceLocation CurLoc,
                                 ACErrorStream &err) {
  clang::FileID fid = sm.getFileID(CurLoc);
  const llvm::MemoryBuffer *this_unit = sm.getBuffer(fid);

  // TODO: this_unit might be a macro unit!
  // handle introductions into introduced classes (nested introductions)
  const IntroductionUnit *intro_unit = IntroductionUnit::cast (this_unit);
  if (intro_unit) this_unit = intro_unit->final_target_unit ();

//  cout << "included units for " << ci->QualName () << " in "
//       << this_unit->name () << " :" << endl;
  for (set<ACFileID>::iterator iter = units.begin ();
      iter != units.end (); ++iter) {
    ACFileID slice_unit = *iter;
    if (ACFileID(sm.getFileEntryForID(sm.getMainFileID())) == slice_unit) {
#if 0
      if (this_unit != primary)
        err << sev_error << decl->getLocation()
          << "affected by aspect in '" << slice_unit.name ().c_str ()
          << "'. Move the aspect into a separate aspect header." << endMessage;
#endif
    }
    else if (sm.getFileEntryForID(fid) &&
             _ig.includes (slice_unit, sm.getFileEntryForID(fid))) {
      err << sev_warning << decl->getLocation()
          << "can't include '" << slice_unit.name ().c_str ()
          << "' to avoid include cycle" << endMessage;
    }
    else {
//      cout << "new edge from " << this_unit->name () << " to "
//             << slice_unit->name () << endl;

      // handling of nested classes -> search the outermost class
      clang::RecordDecl *inscls = llvm::cast<clang::RecordDecl>(decl);
      while (inscls->getLexicalParent()->isRecord())
        inscls = llvm::cast<clang::RecordDecl>(inscls->getLexicalParent());

      ACFileID insfile = sm.getFileEntryForID(sm.getFileID(sm.getExpansionLoc(inscls->getLocation())));
      _ig.add_edge (insfile, slice_unit);

      // namespace should be closed and re-opened
      ostringstream inc;
      _code_weaver.close_namespace (inc, inscls);

      // inspos should be infront of the class
      clang::SourceLocation inspos = inscls->getLocStart();

      // This is what gets parsed. We don't care about namespaces here.
        ostringstream includes;
        Filename incname = _conf.project ().getRelInclString (insfile.name ().c_str(), slice_unit.name ().c_str());
        includes << endl << "#ifndef ";
      Naming::guard (includes, slice_unit);
        includes << endl << "#define ";
      Naming::guard (includes, slice_unit);
        includes << endl;
        includes << "#include \"" << incname << "\"" << endl;
        includes << "#endif" << endl;

      // re-open the namespace
      inc << includes.str();
      _code_weaver.open_namespace (inc, inscls);

      _code_weaver.insert (_code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE), inc.str ());

      // FIXME: stupid hack.
      includes << "}" << endl;

        if (_included_aspect_headers.find (slice_unit) ==
            _included_aspect_headers.end ()) {
          _included_aspect_headers.insert (slice_unit);
      
	parse(parser, includes.str (), CurLoc);
        }
//        else
//          cout << "'" << slice_unit->name () << "' not included again!" << endl;
      }
    }
}

bool ClangIntroducer::end_translation_unit(clang::Parser *parser) {

  bool injection_done = false;
  for (std::set<clang::Decl *>::iterator
           i = _classes_with_introduced_members.begin(),
           e = _classes_with_introduced_members.end();
       i != e; ++i) {
    clang::Decl *decl = *i;
  
    // Introduce members only once.
    if (!_classes_with_introduced_non_inlines.insert(decl).second)
      continue;
  
    // first check with the plan if there are intros for this class
    ACM_Class *jpl = plan_lookup (decl);
    if (!jpl || !jpl->has_plan ()) {
      continue;
    }
  
    clang::TagDecl *tag_decl = (clang::TagDecl*)decl;
    //const clang::Type* tag_type = tag_decl->getTypeForDecl();
    //string signature = tag_decl->getNameAsString ();
    //cout << "END_TU " << signature << " "
    //  << tag_type->getCanonicalTypeInternal().getAsString() << endl;
  
    std::deque<IntroductionUnit *> bufs;
    ACErrorStream &err = _conf.project ().err ();
    gen_intros (jpl, bufs, err, tag_decl, _conf.introduction_depth (), true);

    if (bufs.empty())
      continue;

    clang::SourceLocation CurLoc = parser->getCurToken().getLocation();
    clang::SourceManager &sm = parser->getPreprocessor().getSourceManager();

    if (sm.getFileID(tag_decl->getLocation()) != sm.getMainFileID()) {
      const clang::Decl *loo =
          link_once_object(llvm::dyn_cast<clang::CXXRecordDecl>(tag_decl));
      if (!loo)
        continue;
      else if (const clang::DeclContext *dc =
                   llvm::dyn_cast<clang::DeclContext>(loo))
        if (dc->getLexicalParent() == tag_decl)
      continue;
      // Returned fields are always link once objects.
    }

    // determine the units that should be included in front of the intros
    set<ACFileID> units;
    ACM_ClassPlan *plan = jpl->get_plan ();
    typedef ACM_Container<ACM_MemberIntro, true> MContainer;
    for (MContainer::iterator i = plan->get_member_intros().begin();
         i != plan->get_member_intros().end(); ++i) {
      ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
      std::vector<ACFileID> &member_units = TI_ClassSlice::of(*cs)->non_inline_member_units();
      for (std::vector<ACFileID>::iterator i = member_units.begin ();
          i != member_units.end (); ++i)
        units.insert (*i);
    }
    
    // parse the aspect headers that are needed by this intro
    for (set<ACFileID>::iterator iter = units.begin ();
      iter != units.end (); ++iter) {
      ACFileID slice_unit = *iter;
      ACFileID primary_unit = ACFileID(sm.getFileEntryForID(sm.getMainFileID()));
      if (primary_unit != slice_unit) {
//        cout << "new edge from " << primary->name () << " to "
//             << slice_unit->name () << endl;
        _ig.add_edge (primary_unit, slice_unit);
        // generate a unit with the include
        ostringstream includes;
        Filename incname = _conf.project().getRelInclString(primary_unit.name().c_str (), slice_unit.name().c_str ());
        includes << endl << "#ifndef ";
        Naming::guard (includes, slice_unit);
        includes << endl << "#define ";
        Naming::guard (includes, slice_unit);
        includes << endl;
        includes << "#include \"" << incname << "\"" << endl;
        includes << "#endif" << endl;
        string inc (includes.str ());
        _code_weaver.insert(
            _code_weaver.weave_pos(CurLoc, WeavePos::WP_BEFORE), inc, true);

        if (_included_aspect_headers.find (slice_unit) ==
            _included_aspect_headers.end ()) {
          _included_aspect_headers.insert (slice_unit);
          parse(parser, inc, CurLoc);
    }
//        else
//          cout << "'" << slice_unit->name () << "' not included again!" << endl;
  }
    }

    clang::FileID fid = sm.getFileID(CurLoc);
    const llvm::MemoryBuffer *this_unit = sm.getBuffer(fid);
    for (std::deque<IntroductionUnit *>::reverse_iterator ui =
             bufs.rbegin();
         ui != bufs.rend(); ++ui) {
      (*ui)->target_unit(this_unit);
      (*ui)->location(CurLoc);
      (*ui)->file_id(inject((*ui)->buffer(), CurLoc, !injection_done));
      injection_done = true;
    }
    std::vector<IntroductionUnit *> &intro_units =
        get_intros_for(IntroductionUnit::cast(this_unit));
    intro_units.insert(intro_units.end(), bufs.begin(), bufs.end());
  }
  return injection_done;
}

void ClangIntroducer::tunit_start () {
  // handle Clang parser quirks
// Clang < 3.9.1 does not support __float128 type ...
#if CLANG_VERSION_NUMBER < VERSION_NUMBER_3_9_1
  // ...  therefore we define the __float128 type by ourself.
  // TODO: This does not behave like the real __float128
  string quirks =
      "namespace AC { struct __float128 {\n"
      "  union {\n"
      "    char _data[128/8];\n"
      "    double _val;\n"
      "  };\n"
      "  __float128 () {}\n"
      "  __float128 (double d) : _val (d) {}\n"
      "  operator double () const { return _val; }\n"
      "  __float128 &operator += (double d) { _val += d; return *this; }\n"
      "  __float128 &operator -= (double d) { _val -= d; return *this; }\n"
      "  __float128 &operator *= (double d) { _val *= d; return *this; }\n"
      "  __float128 &operator /= (double d) { _val /= d; return *this; }\n"
      "}; }\ntypedef AC::__float128 __float128;\n}";

  // parse and inject the code
  clang::SourceManager &SM = _ci->getSourceManager ();
  clang::SourceLocation quirks_loc = SM.getLocForStartOfFile (SM.getMainFileID());
  parse (_parser, quirks, quirks_loc);
#endif
}

void ClangIntroducer::tunit_end () {
  clang::Token &token = (clang::Token &)_parser->getCurToken();
  clang::Token saved_token = token;

  if (!end_translation_unit(_parser)) { // inject code and parse required headers
    return;
  }

  _ci->getPreprocessor ().Lex (token);
  clang::Parser::DeclGroupPtrTy ADecl;
  while (!_parser->ParseTopLevelDecl(ADecl)) {
    if (ADecl && !_ci->getASTConsumer().HandleTopLevelDecl(ADecl.get()))
      break;
  }
  token = saved_token;
}

string substitute_names(const string &text, clang::TagDecl *target, int precedence,
    const std::vector<std::pair<size_t, TI_ClassSlice::SliceBody::InsertType> > &positions,
    bool &jp_needed) {
  string result;
  typedef TI_ClassSlice::SliceBody member_t;

  // generate names to be inserted at the appropriate positions
  std::string target_name = target->getNameAsString();
  std::string target_qual_name = target->getQualifiedNameAsString();
  std::string jp_name = "__TJP_" + llvm::itostr(precedence);

  jp_needed = false;
  size_t last_pos = 0;
  // The positions are sorted ascending.
  for (int pi = 0, pe = positions.size(); pi != pe; ++pi) {
    size_t pos = positions[pi].first;
    member_t::InsertType type = positions[pi].second;

    result += text.substr(last_pos, pos - last_pos);
    switch (type) {
    case member_t::TARGET_NAME:
      result += target_name;
      break;
    case member_t::TARGET_QUAL_NAME:
      result += target_qual_name;
      break;
    case member_t::JP_NAME:
      result += jp_name;
      jp_needed = true;
      break;
    }

    last_pos = pos;
  }
  result += text.substr (last_pos);

  return result;
}

void ClangIntroducer::gen_intros (ACM_Class *jpl,
                             std::deque<IntroductionUnit *> &units,
                             ACErrorStream &err,
                             clang::TagDecl *target, int introduction_depth,
  bool non_inline) const {
  typedef TI_ClassSlice::SliceBody member_t;

  // handle all intros
  typedef ACM_Container<ACM_MemberIntro, true> Container;
  Container &intros = jpl->get_plan()->get_member_intros();
  int i = 0;
  for (Container::iterator iter = intros.begin(); iter != intros.end(); ++iter, ++i) {
    ACM_Introduction *ii = (*iter)->get_intro();

    // TODO: clean up; a lot of duplicated code here
    TI_ClassSlice *ti = TI_ClassSlice::of (*get_slice (*ii));

    // generate non-inline introduction instance
    if (ti->non_inline_members ().size () > 0 && non_inline) {
      // Format the non-inline members. Everything is tokenized with the required
      // elements inserted in the given positions.
      std::vector<member_t> &bodies = ti->non_inline_members ();
      for (unsigned body_no = 0; body_no < bodies.size(); ++body_no) {
        const member_t &body = bodies[body_no];
        std::string str; // = "\n";

        // create the new unit
        IntroductionUnit *unit = new IntroductionUnit(body_no);
        unit->intro (ii);
        unit->precedence (i);

        bool jp_needed;
        str += substitute_names(body.text, target, i, body.positions, jp_needed);
        unit->jp_needed(jp_needed);

        // if there was no introduction, delete the unit -> no result
        if (str.empty() || str.find_first_not_of(" \n") == std::string::npos)
           delete unit;
         // check whether this is a deeply nested introduction
        else if (unit->nesting_level () > introduction_depth) {
          err << sev_error << target->getLocation()
              << "maximum level of nested introductions (" << introduction_depth
              << ") for class '"
              << target->getQualifiedNameAsString().c_str() << "' exceeded" << endMessage;
          err << sev_error
              << Location (Filename (filename (*ii).c_str ()), line (*ii))
              << "invalid introduction defined here" << endMessage;
          delete unit;
        }
        else {
          unit->content (str);
          units.push_back (unit);
        }
      }
      continue;
    }

    // create the new unit
    IntroductionUnit *unit = new IntroductionUnit(IntroductionUnit::IU_MEMBERS);
      //new IntroductionUnit (err, (Unit*)target->Tree ()->token ()->belonging_to ());
    unit->intro (ii);
    unit->precedence (i);
    std::string str; // = "\n";

    // generate inline introduction instance
    if (!ti->get_tokens().text.empty() && !non_inline) {
      // create a unit with the target class name
      std::string target_name = target->getNameAsString();

      std::string slice_start;
      if (get_slice(*ii)->get_is_struct())
        slice_start += "  public: ";
      else
        slice_start += "  private: ";
      // add "  typedef <target-name> <slice-name>;\n"
      if (get_slice(*ii)->get_name()[0] != '<') {
        slice_start += "typedef " + target_name + " "
                    + get_slice(*ii)->get_name() + "; ";
      }
      str += slice_start;

      const member_t &body = ti->get_tokens ();
      bool jp_needed;
      str += substitute_names(body.text, target, i, body.positions, jp_needed);
      unit->jp_needed(jp_needed);
    }

    // if there was no introduction, delete the unit -> no result
    if (str.empty() || str.find_first_not_of(" \n") == std::string::npos)
       delete unit;
     // check whether this is a deeply nested introduction
    else if (unit->nesting_level () > introduction_depth) {
      err << sev_error << target->getLocation()
          << "maximum level of nested introductions (" << introduction_depth
          << ") for class '"
          << target->getQualifiedNameAsString().c_str() << "' exceeded" << endMessage;
      err << sev_error
          << Location (Filename (filename (*ii).c_str ()), line (*ii))
          << "invalid introduction defined here" << endMessage;
      delete unit;
    }
    else {
      unit->content (str);
      units.push_back (unit);
    }
  }
}

void ClangIntroducer::gen_base_intros (ACM_Class *jpl,
                                  std::deque<IntroductionUnit *> &units,
                                  bool commas_only) const {
  typedef ACM_Container<ACM_BaseIntro, true> Container;
  Container &bases = jpl->get_plan()->get_base_intros();

  for (Container::iterator i = bases.begin(); i != bases.end (); ++i) {
    // get the current introduction
    ACM_Introduction *ii = (*i)->get_intro();

    // create the new unit
    IntroductionUnit *unit = new IntroductionUnit(IntroductionUnit::IU_BASE);
      //new IntroductionUnit (err, (Unit*)target->Tree ()->token ()->belonging_to ());
    unit->intro (ii);

    // generate the code for this base class introduction
    // FIXME: Line directives.
    std::string str;
    gen_base_intro (str, ii, (i == bases.begin()) && !commas_only);

    unit->content (str);

    // store the result for the caller
    units.push_back (unit);
  }
}

void ClangIntroducer::gen_base_intro (std::string &unit,
  ACM_Introduction *ii, bool first) const {
  TI_ClassSlice *ti = TI_ClassSlice::of (*get_slice(*ii));
  // generate the code for this entry
  unit += (first ? ": " : ", ");
  unit += ti->base_intro();
}

void ClangIntroducer::function_start (clang::FunctionDecl *decl) {

  // introduce code in front of the function definition
//  ClangSyntacticContext fcontext(decl);
//  _code_weaver.insert(fcontext.wrapper_pos(_code_weaver), "/* some generated stuff */");

  //  if (decl->getNameAsString() == "f") {
//    parse(_parser, "int fparam = 42; }",
//        decl->getLocStart(), decl->getLexicalDeclContext());
//  }
}

void ClangIntroducer::declarator_start (clang::DeclaratorDecl *decl) {

  // introduce code in front of the function definition
//  ClangSyntacticContext fcontext(decl);
//  _code_weaver.insert(fcontext.wrapper_pos(_code_weaver), "/* some generated stuff */");
}
