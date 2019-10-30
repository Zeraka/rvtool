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

#include "ModelBuilder.h"
#include "IntroductionUnit.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "ACConfig.h"
#include "ACPreprocessor.h"
#include "ACModel/Utils.h"

// Puma library includes
#include "Puma/SysCall.h"

#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"
#include <clang/AST/ExprCXX.h>
#include "clang/Frontend/CompilerInstance.h"
#include "ClangAnnotation.h"

// start phase 1
void ClangModelBuilder::setup_phase1 (const string &tunit_name, int tunit_len) {
  _tunit_file = newTUnit();
  _tunit_file->set_filename(model_filename (tunit_name.c_str()));
  _tunit_file->set_len(tunit_len);
  _tunit_file->set_time(modification_time (tunit_name));
  get_files().insert(_tunit_file);
  file_map().insert(
      ClangModelBuilder::FileMapPair(model_filename(tunit_name.c_str()), _tunit_file));
  set_root(0); // TODO: to be done by libacmodel

  register_namespace1(0, "::");

}


void ClangModelBuilder::advice_infos (ACM_Aspect *jpl_aspect) {
  // collect the advice nodes
  // TODO: change of semantics: the previous version created entries
  // for the advice code in both phases. What does that mean?
  clang::CXXRecordDecl *cls =
    llvm::cast<clang::CXXRecordDecl>(TI_Aspect::of(*jpl_aspect)->decl());

  clang::CXXRecordDecl::decl_iterator DI = cls->decls_begin(),
    DE = cls->decls_end();
  typedef ACM_Container<ACM_AdviceCode, true> Container;
  Container &advices = jpl_aspect->get_advices();
  for (Container::iterator i = advices.begin (); i != advices.end (); ++i) {
    ACM_AdviceCode *advice_code = *i;

    while (DI != DE) {
      if (clang::NamedDecl *ND = llvm::dyn_cast<clang::NamedDecl>(*DI))
        if (llvm::StringRef(ND->getNameAsString()).startswith("__a"))
          break;
      ++DI;
    }

    if (DI == DE) {
      // TODO: this should be a fatal error message
      cout << "advice cannot be associated with advice function in aspect " << cls->getQualifiedNameAsString() << endl;
      return;
    }

    clang::Decl *D = *DI++;
    // Look through templates.
    if (clang::FunctionTemplateDecl *FTD =
        llvm::dyn_cast<clang::FunctionTemplateDecl>(D))
      D = FTD->getTemplatedDecl();

    clang::FunctionDecl *FD = llvm::cast<clang::FunctionDecl>(D);
    unsigned first_context_arg = 0;
    if (FD->getNumParams() > 0 && FD->getParamDecl(0)->getName() == "tjp")
      first_context_arg = 1;

    for (unsigned a = first_context_arg; a < FD->getNumParams(); a++) {
      advice_code->get_pointcut()->get_args().insert(register_arg(
          FD->getParamDecl(a)->getType(), FD->getParamDecl(a)->getName()));
    }

    // tree is the tree of the function definition
    TI_AdviceCode::of(*advice_code)->decl (FD);
  }
}

static string absolutePath (const char *name) {
  Puma::Filename result;
  if (!Puma::SysCall::canonical (name, result))
    return string();

  return result.name ();
}

string ClangModelBuilder::model_filename (const char *name) {
  string fname = absolutePath (name);
  if (fname.empty ())
    return fname;
  ACProject &prj = _conf.project ();
  // if the file does not belong to the project return the absolute path
  if (!prj.isBelow (fname.c_str ()))
    return fname;
  // iterate over all project paths
  for (long p = 0; p < prj.numPaths (); p++) {
    Puma::Filename dir_abs;
    if (!Puma::SysCall::canonical (prj.src (p), dir_abs)) {
      assert (false);
      return fname;
    }
    int dir_len = strlen (dir_abs.name ());
    if (strncmp (dir_abs.name (), fname.c_str (), dir_len) == 0) {
      return fname.substr(dir_len + 1);
    }
  }
  // the file has to be below any of the directories => fatal error here
  // assert (false); acgen.c does not belong to the project with this test
  return fname;
}


// get the modification time of a file (UNIX Epoch value)
long ClangModelBuilder::modification_time (const string &filename) {
  string abs_name = absolutePath (filename.c_str ());
  if (abs_name.empty ())
    return -1; // File does not exists or other error
  Puma::FileInfo fileinfo;
  if (! Puma::SysCall::stat (abs_name.c_str (), fileinfo))
    return -1; // File does not exists.
  return fileinfo.modi_time ();
}


bool ClangModelBuilder::inside_template (clang::DeclContext *scope) const {
  if (llvm::isa<clang::TranslationUnitDecl>(scope))
    return false;

  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(scope))
    if (FD->getTemplatedKind() != clang::FunctionDecl::TK_NonTemplate)
      return true;

  //if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(scope))
  //  if (VD->getTemplatedKind() != clang::TK_NonTemplate)
  //    return true;

  if (clang::CXXRecordDecl *RD = llvm::dyn_cast<clang::CXXRecordDecl>(scope))
    if (RD->getDescribedClassTemplate() ||
        RD->getTemplateSpecializationKind() ==
            clang::TSK_ExplicitSpecialization)
      return true;

  return inside_template(scope->getParent());
}

template <typename T>
static bool isTemplateInstantiation(T node) {
  return (node->getTemplateSpecializationKind() ==
              clang::TSK_ImplicitInstantiation ||
          node->getTemplateSpecializationKind() ==
              clang::TSK_ExplicitInstantiationDefinition);
}

bool ClangModelBuilder::inside_template_instance (clang::DeclContext *scope) {
  if (llvm::isa<clang::TranslationUnitDecl>(scope))
    return false;

  if (clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>(scope))
    if (isTemplateInstantiation(FD))
      return true;

  if (clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>(scope))
    if (isTemplateInstantiation(VD))
      return true;

  if (clang::CXXRecordDecl *RD = llvm::dyn_cast<clang::CXXRecordDecl>(scope))
    if (isTemplateInstantiation(RD))
      return true;

  return inside_template_instance(scope->getParent());
}

bool ClangModelBuilder::is_valid_model_class (clang::RecordDecl *ci) const {
  // Don't consider
  // * anonymous classes like template instances(?)
  // * the generated class JoinPoint
  // * classes defined in the special namespace AC
  llvm::StringRef Name = ci->getName();
  std::string QualName = ci->getQualifiedNameAsString();
  if (Name.empty() ||
      Name.startswith("%") || Name == "JoinPoint" ||
      Name.startswith("__TJP") || Name == "__TI" ||
      llvm::StringRef(QualName).startswith("AC::"))
    return false;

  // Templates and classes nested in template class are not considered for
  // matching, only instances
  if (inside_template (ci))
    return false;

  return true;
}

bool ClangModelBuilder::is_valid_model_function (clang::FunctionDecl *fi) const {
  // Don't consider
  // * member functions of an invalid class
  // * ac++ generated functions
  // * pointcuts

  clang::DeclContext *cls = fi->getDeclContext();
  clang::RecordDecl *RD = llvm::dyn_cast<clang::RecordDecl>(cls);
  if (RD && !is_valid_model_class(RD))
    return false;

  if (!fi->isTemplateInstantiation () && inside_template (fi))
    return false;

  std::string S = fi->getNameAsString();
  llvm::StringRef Name = S;
  if (Name.startswith("%a") || Name.startswith("__a") || Name == "aspectof" ||
      Name == "aspectOf" || Name.startswith("__builtin_"))
    return false;

  // Explicitly filter the operators added by phase 1.
  if ((Name == "operator new" || Name == "operator delete") &&
      fi->getNumParams() == 2) {
    std::string arg_ty = TI_Type::get_type_sig_text(fi->getParamDecl(1)->getType(), &fi->getASTContext(), 0, false);
    if (arg_ty == "AC::AnyResultBuffer *")
      return false;
  }

  // filter-out built-in functions that are neither constructor nor destructor
  //
  // (We filter out the copy-assignment-operators, because we
  // currently do not get all calls of these. Example:
  //
  //   class A {};
  //   class B : public A {};
  //
  //   B b1, b2;
  //   b1 = b2; // Internally A::operator= gets called here, but currently there
  //            // is no join-point and therefore no advice for that call.)
  //
  clang::CXXMethodDecl *method = llvm::dyn_cast<clang::CXXMethodDecl>(fi);
  if (method && !method->isUserProvided() &&
      /*!method->isCopyAssignmentOperator() &&*/
      !llvm::dyn_cast<clang::CXXConstructorDecl>(fi) &&
      !llvm::dyn_cast<clang::CXXDestructorDecl>(fi))
    return false;

  return true;
}

bool ClangModelBuilder::is_valid_model_variable( clang::DeclaratorDecl *vi ) const {
  // Don't consider
  // * variables in invalid places
  // * template instantiations
  // * dummy declarations with empty identifier

  clang::DeclContext *ctx = vi->getDeclContext();
  clang::NamespaceDecl *ND = llvm::dyn_cast<clang::NamespaceDecl>( ctx );
  clang::RecordDecl *RD = llvm::dyn_cast<clang::RecordDecl>( ctx );
  clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>( ctx );
  if(    ( ND && ! is_valid_model_namespace( ND ) )
      || ( RD && ! is_valid_model_class( RD ) )
      || ( FD && ! is_valid_model_function( FD ) )
    )
    return false;

  if( inside_template( ctx ) )
    return false;

  // dummy declaration
  llvm::StringRef Name = vi->getName();
  if( Name.empty() || Name == "<anonymous>" ) //TODO: is "<anonymous>" still correct in clang >= 3.6.2?
    return false;

  return true;
}

bool ClangModelBuilder::is_valid_model_namespace (clang::NamespaceDecl *ni) const {
  llvm::StringRef Name = ni->getName();
  // no template instance namespace, but anonymous namespaces!
  if ((ni->isAnonymousNamespace() || Name.find('<') == llvm::StringRef::npos) &&
      Name != "AC" && !Name.startswith("__puma"))
    return true;
  return false;
}

bool ClangModelBuilder::is_valid_model_type( const clang::QualType& qual_type ) const {
  // Don't consider
  // * anonymous/unnamed types
  // * local types
  // * types with no linkage
  // because the C++ Standard (2003) states in chapter 14.3 point 1 and 2:
  // "A local type, a type with no linkage, an unnamed type or a type compounded from any of
  // these types shall not be used as a template-argument for a template type-parameter."

  return !qual_type->hasUnnamedOrLocalType()
      && qual_type->getLinkage() != clang::Linkage::NoLinkage
      && qual_type->getLinkage() != clang::Linkage::VisibleNoLinkage; // TODO: really?

  // TODO: An enum type could be treated as a suitable integer type like the following:
  // Get the suitable integer type from the operator expression and the argument expressions with:
  //    clang::QualType qual_type = expression->getType().getCanonicalType();
  //    if (const clang::EnumType* enum_type = clang::dyn_cast<clang::EnumType>(qual_type)) {
  //      clang::QualType promotion_type = enum_type->getDecl()->getPromotionType();
  //      if(!promotion_type.isNull()) {
  //        assert(promotion_type->isIntegerType());
  //        if(qual_type->isPointerType()) {
  //          promotion_type = context.getPointerType(promotion_type);
  //        }
  //        // reference type?
  // Set the type as the new type of the respective expression:
  //        expression->setType(promotion_type);
  //      }
  //    }
  // Make sure that the result type is not deduced and that the non reference version of the
  // result type will be inserted in the code
}

TU_Type *ClangModelBuilder::register_type (clang::QualType ti) {
  TU_Type *new_elem = newType();
  new_elem->set_signature(
      TI_Type::name(_project.get_compiler_instance()->getASTContext(), ti));
  new_elem->type (ti);
  return new_elem;
}


TU_Arg *ClangModelBuilder::register_arg (clang::QualType ti, llvm::StringRef name) {
  TU_Arg *new_elem = newArg();
  new_elem->set_type(
      TI_Type::name(_project.get_compiler_instance()->getASTContext(), ti));
  new_elem->set_name(name);
  new_elem->type (ti);
  return new_elem;
}

TU_Attribute *ClangModelBuilder::register_attrdecl1 (ACM_Namespace *parent, const std::string &name)
{
  TU_Attribute *elem = 0;
  if (parent)
    elem = (TU_Attribute*)map_lookup(*parent, name);
  if (!elem || elem->type_val() != JPT_Attribute) {
    elem = newAttribute();
    elem->set_name(name);
    if(parent->get_name() == "clang" || parent->get_name() == "gnu")
      elem->set_builtin(true);
    else
      elem->set_builtin(false);
    if (parent) {
      map_insert(*parent, *elem, name);
      parent->get_children().insert(elem);
    }
  }
  else {
    //for already registered attributes?
    elem = 0;
  }

  return elem;
}

// find an attribute declaration
TU_Attribute *ClangModelBuilder::find_attrdecl (const std::vector<std::string> &qual_name) {
  ACM_Name *scope = get_root();
  ACM_Name *found = nullptr;
  for (string name : qual_name) {
    found = map_lookup(*scope, name);
    if (!found)
      return nullptr;
    scope = found;
  }
  if (found->type_val() != JPT_Attribute)
    return nullptr;
  return (TU_Attribute*)found;
}

ACM_Pointcut *ClangModelBuilder::register_pointcut1 (ACM_Name *parent, const string &name,
    bool is_virtual, const string& expr) {

  TU_Pointcut *elem = 0;
  if (parent)
    elem = (TU_Pointcut*)map_lookup(*parent, name);
  if (!elem || elem->type_val() != JPT_Pointcut) {
    elem = newPointcut();
    elem->set_name(name);
    elem->set_expr (expr);
    elem->set_builtin(false);
    elem->set_kind(PT_NORMAL);
    if (parent) {
      if (is_virtual && expr == "0")
        elem->set_kind(PT_PURE_VIRTUAL);
      else if (is_virtual || overrides_virtual_pointcut(parent, name))
        elem->set_kind(PT_VIRTUAL);
      map_insert(*parent, *elem, name);
      parent->get_children().insert(elem);
    }
  }
  else {
    elem = 0;
  }

  return elem;
}

bool ClangModelBuilder::overrides_virtual_pointcut (ACM_Name *parent, const string &name) {
  if (parent->type_val() != JPT_Class && parent->type_val() != JPT_Aspect)
    return false;
  ACM_Class *cls = (ACM_Class*)parent;
  typedef ACM_Container<ACM_Class, false> BList;
  const BList &blist = cls->get_bases();
  for (BList::const_iterator i = blist.begin (); i != blist.end(); ++i) {
    ACM_Class *base = (ACM_Class*)*i;
    ACM_Name *elem = map_lookup(*base, name);
    if (!elem || elem->type_val() != JPT_Pointcut)
      continue;
    ACM_Pointcut *base_pct = (ACM_Pointcut*)elem;
    if (base_pct->get_kind() == PT_PURE_VIRTUAL ||
        base_pct->get_kind() == PT_VIRTUAL ||
        overrides_virtual_pointcut (base, name))
      return true;
  }
  return false;
}


TU_Pointcut *ClangModelBuilder::register_pointcut (clang::FunctionDecl *fi,
    ACM_Name *parent) {

  clang::FunctionDecl *def = fi;

  // find the parent model element
  if (!parent)
    return 0;

  // build the name of the function
  string name = fi->getNameAsString();

  TU_Pointcut *elem = (TU_Pointcut*)map_lookup(*parent, name);
  if (!elem || elem->type_val() != JPT_Pointcut)
    return 0;

  elem->get_args().clear();
  // argument types are the types of the function declaration
  for (clang::FunctionDecl::param_iterator i = fi->param_begin(),
       e = fi->param_end(); i != e; ++i) {
    elem->get_args().insert(register_arg((*i)->getType(), (*i)->getName()));
  }
  elem->decl (def);
  add_source_loc (elem, def);

  return elem;
}

TU_Function *ClangModelBuilder::register_function (clang::FunctionDecl *fi,
  ACM_Name *parent) {
  if (!is_valid_model_function (fi))
    return 0;

  clang::FunctionDecl *def = fi;

  // find the parent model element
  if( !parent && ! ( parent = register_scope( static_cast<clang::DeclContext *>( def ) ) ) ) // explicit cast necessary to chose function (both should be equivalent, using old codepath)
    return 0;

  // build the name of the function for lookup
  std::string sig = TI_Function::signature(def);

  clang::CXXMethodDecl *method = llvm::dyn_cast<clang::CXXMethodDecl>(def);
  bool isStatic = fi->getStorageClass() == clang::SC_Static;
  bool isStaticMethod = method && method->isStatic();
  
  // register the element
  TU_Function *elem = (TU_Function*)map_lookup(*parent, sig);
  bool hidden_function = (elem && elem->type_val() == JPT_Function &&
                          ((isStatic && !isStaticMethod) &&
                           (!elem->has_static_in () || elem->get_static_in () != _tunit_file)));
  if (!elem || hidden_function) {
    elem = newFunction();
    elem->set_name(TI_Function::name(fi));
    if (isStatic && !isStaticMethod)
      elem->set_static_in (_tunit_file);
    map_insert(*parent, *elem, sig);
    parent->get_children().insert(elem);
    FunctionType ft = FT_NON_MEMBER;
    if (llvm::isa<clang::CXXConstructorDecl>(fi))
      ft = FT_CONSTRUCTOR;
    else if (llvm::isa<clang::CXXDestructorDecl>(fi)) {
      if (method->isVirtual() && method->isPure())
        ft = FT_PURE_VIRTUAL_DESTRUCTOR;
      else if (method->isVirtual())
        ft = FT_VIRTUAL_DESTRUCTOR;
      else
        ft = FT_DESTRUCTOR;
    }
    else if (method) {
      if (isStaticMethod)
        ft = FT_STATIC_MEMBER;
      else if (method->isVirtual() && method->isPure())
        ft = FT_PURE_VIRTUAL_MEMBER;
      else if (method->isVirtual())
        ft = FT_VIRTUAL_MEMBER;
      else
        ft = FT_MEMBER;
    }
    else {
      if (isStatic)
        ft = FT_STATIC_NON_MEMBER;
    }
    elem->set_kind (ft);
    elem->set_builtin (method && !method->isUserProvided());
    CVQualifiers cvq = CVQ_NONE;
    if (method && method->isConst())
      cvq = (CVQualifiers)(cvq | CVQ_CONST);
    if (method && method->isVolatile())
      cvq = (CVQualifiers)(cvq | CVQ_VOLATILE);
    elem->set_cv_qualifiers(cvq);
    elem->decl (def);

    // TODO: We forget the arg type from arg 0. They will be deleted by jpm.
    elem->get_arg_types().clear();
    if (!llvm::isa<clang::CXXConstructorDecl>(def) &&
        !llvm::isa<clang::CXXDestructorDecl>(def)) {
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      clang::QualType rtype = fi->getResultType();
    // CTypeInfo *rtype = (fi->isConversion ()) ? fi->ConversionType() : fi->TypeInfo ()->BaseType ();
#else // C++ 11 interface
      clang::QualType rtype = fi->getReturnType();
#endif
      elem->set_result_type(register_type(rtype));
    }
    elem->set_variadic_args(fi->isVariadic());
    for (clang::FunctionDecl::param_iterator i = fi->param_begin(),
         e = fi->param_end(); i != e; ++i) {
      clang::QualType ty =
          fi->getASTContext().getSignatureParameterType((*i)->getType());
      elem->get_arg_types().insert(register_type(ty));
    }
    clang::SourceManager &sm = _project.get_compiler_instance()->getSourceManager();
    clang::PresumedLoc PL = sm.getPresumedLoc(fi->getLocation());
    llvm::StringRef Name = PL.getFilename();
    llvm::StringRef BufferName = sm.getBufferName(fi->getLocation());
    if (!elem->get_builtin() &&
        (!method || !(method->isVirtual() && method->isPure())) &&
        !(inside_template_instance (fi)) &&
        (BufferName.startswith("<intro") ||
         (!Name.empty() && _project.isBelow(Name.str().c_str())))) {
      if (llvm::isa<clang::CXXConstructorDecl>(def))
        register_construction (elem);
      else if (llvm::isa<clang::CXXDestructorDecl>(def))
        register_destruction (elem);
      else
        register_execution (elem);

      // constructors and and destructors cannot be called
      if (!llvm::isa<clang::CXXConstructorDecl>(def) &&
          !llvm::isa<clang::CXXDestructorDecl>(def)) {
        // register a 'pseudo call join point'
        register_call (def, 0, 0, 0);
      }
    }
  }
  else {
    // make sure that elem->func_info() will be the DefObject
    //if (fi == def)
      //elem->func_info(def);
  }

  // update the list of source locations
  add_source_loc (elem, fi,
                  fi->isThisDeclarationADefinition () ? SLK_DEF : SLK_DECL);
  return elem;
}

TU_Variable *ClangModelBuilder::register_variable( clang::DeclaratorDecl *oi, ACM_Name *parent ) {
  if( ! is_valid_model_variable( oi ) )
    return 0;

  clang::DeclaratorDecl *def = oi;
  clang::VarDecl *VD = llvm::dyn_cast<clang::VarDecl>( oi );
  clang::FieldDecl * FD = llvm::dyn_cast<clang::FieldDecl>( oi );

  // check if we have a valid definition, might be another type of entity used in member exprs
  if( ! VD && ! FD )
    return 0;

  // find the parent model element
  if (!parent && !(parent = register_scope (def)))
    return 0;

  // ignore bitfields
  if( FD && FD->isBitField() )
    return 0;

  // ignore incomplete arrays
  if ( oi->getType().getTypePtr()->isIncompleteArrayType() )
    return 0;

  // get the name of the variable
  string sig = def->getNameAsString();//TI_Variable::name (def);

  // register the element
  // TODO: classes and variables in the same scope may have the same name!
  // This might lead to invalid results here.
  TU_Variable *elem = (TU_Variable*)map_lookup(*parent, sig);
  if (!elem || elem->type_val() != JPT_Variable) {
    elem = newVariable ();

    elem->set_name(sig);
    elem->set_builtin(false);

    TU_Type *var_type = register_type(oi->getType ());
    elem->set_type(var_type);

    bool isGlobal = VD && ( VD->isFileVarDecl() && ! VD->isStaticDataMember() );
    bool isMember = FD || ( VD && VD->isStaticDataMember() );
    bool isStatic = VD && ( VD->getStorageClass() == clang::SC_Static );
    if( isGlobal ) {
      if( isStatic )
        elem->set_kind( VT_STATIC_NON_MEMBER );
      else
       elem->set_kind( VT_NON_MEMBER );
    }
    else if( isMember )
      if( isStatic )
        elem->set_kind( VT_STATIC_MEMBER );
      else
        elem->set_kind( VT_MEMBER );
    else
      elem->set_kind( VT_UNKNOWN );

    elem->decl(def);
    map_insert(*parent, *elem, sig);
    parent->get_children().insert(elem);
  }

  bool isDef = ( VD && VD->isThisDeclarationADefinition() ) || FD;
  add_source_loc( elem, oi, isDef ? SLK_DEF : SLK_DECL );

  return elem;
}

ACM_ClassSlice *ClangModelBuilder::register_class_slice (ACM_Name *scope, string name,
    bool is_struct) {

  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_ClassSlice)
    return (ACM_ClassSlice*)found;

  ACM_ClassSlice *new_elem = (ACM_ClassSlice*)newClassSlice();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  new_elem->set_is_struct(is_struct);
  map_insert(*scope, *new_elem, name);
  scope->get_children().insert(new_elem);
  return new_elem;
}


ACM_Class * ClangModelBuilder::register_class1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_Class)
    return (TU_Class*)found;
  ACM_Class *new_elem = newClass();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  if (!in_project) new_elem->get_tunits().insert (_tunit_file);
  new_elem->set_intro_target(in_project);
  scope->get_children().insert(new_elem);
  map_insert(*scope, *new_elem, name);
  return new_elem;
}

void ClangModelBuilder::register_constructor_and_destructor (clang::RecordDecl *ci,
    ACM_Class *elem) {

  if (!ci->isThisDeclarationADefinition())
    return;

  // For Clang we create fake constructor and destructor decls in case they
  // aren't already there. Clang only adds implicitly defined members to the AST
  // but we need all implicitly declared ones in the model
  if (clang::CXXRecordDecl *rec = llvm::dyn_cast<clang::CXXRecordDecl>(ci)) {
    std::string name = rec->getNameAsString();
    if (rec->hasSimpleDestructor()) {
      std::string full_name = '~' + name + "()";
      if (!map_lookup(*elem, full_name)) {
        TU_Function *new_func = newFunction();
        new_func->set_name('~' + name);
        map_insert(*elem, *new_func, full_name);
        elem->get_children().insert(new_func);
        new_func->set_variadic_args(false);
        new_func->set_kind(FT_DESTRUCTOR);
        new_func->set_builtin(true);
        new_func->set_cv_qualifiers(CVQ_NONE);
        new_func->decl(0);
        if (!isTemplateInstantiation(rec))
          register_destruction(new_func);
      }
    }
    if (!rec->hasUserDeclaredConstructor()) {
      std::string full_name = name + "()";
      if (!map_lookup(*elem, full_name)) {
        TU_Function *new_func = newFunction();
        new_func->set_name(name);
        map_insert(*elem, *new_func, full_name);
        elem->get_children().insert(new_func);
        new_func->set_variadic_args(false);
        new_func->set_kind(FT_CONSTRUCTOR);
        new_func->set_builtin(true);
        new_func->set_cv_qualifiers(CVQ_NONE);
        new_func->decl(0);
        if (!isTemplateInstantiation(rec))
          register_construction(new_func);
      }
    }
    if (!rec->hasUserDeclaredCopyConstructor()) {
      clang::ASTContext &Context = ci->getASTContext();
      clang::QualType ClassType = Context.getTypeDeclType(rec);
      clang::QualType ArgType = ClassType;
      if (rec->hasCopyConstructorWithConstParam())
        ArgType = ArgType.withConst();
      ArgType = Context.getLValueReferenceType(ArgType);
      std::string full_name =
          name + "(" + TI_Type::get_type_sig_text(ArgType, &Context, 0, true) + ")";
      if (!map_lookup(*elem, full_name)) {
        TU_Function *new_func = newFunction();
        new_func->set_name(name);
        map_insert(*elem, *new_func, full_name);
        elem->get_children().insert(new_func);
        new_func->get_arg_types().insert(register_type(ArgType));
        new_func->set_variadic_args(false);
        new_func->set_kind(FT_CONSTRUCTOR);
        new_func->set_builtin(true);
        new_func->set_cv_qualifiers(CVQ_NONE);
        new_func->decl(0);
        if (!isTemplateInstantiation(rec))
          register_construction(new_func);
      }
    }
  }
}


TU_Class *ClangModelBuilder::register_class (clang::RecordDecl *ci,
                                        ACM_Name *parent, bool set_source) {

  // only classes are relevant
  if (!is_valid_model_class (ci))
    return 0;

  // find the parent model element
  if (!parent && !(parent = register_scope (ci)))
    return 0;

  string name_with_template_args = TI_Class::name (ci);
  assert(!name_with_template_args.empty());

  TU_Class *elem = 0;
  ACM_Name *found = map_lookup(*parent, name_with_template_args);
  if (!found || found->type_val() != JPT_Class) {
    elem = (TU_Class*)newClass();
    elem->set_name(name_with_template_args);
    elem->set_builtin(false);
    parent->get_children().insert(elem);
    map_insert(*parent, *elem, name_with_template_args);
    bool intro_target = is_intro_target (ci);
    elem->set_intro_target (intro_target);
    if (!intro_target) elem->get_tunits().insert (_tunit_file);
  }
  else
    elem = (TU_Class*)found;
  // set the class attributes
  if (ci->isThisDeclarationADefinition ())
    elem->decl (ci);
  if (set_source && elem->type_val () != JPT_Aspect) {
    // FIXME: Is this right?
    if (!inside_template_instance(ci))
      add_source_loc (elem, ci,
                      ci->isThisDeclarationADefinition () ? SLK_DEF : SLK_DECL);
  }

  register_constructor_and_destructor (ci, elem);
  return elem;
}

ACM_Aspect * ClangModelBuilder::register_aspect1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_Aspect)
    return (TU_Aspect*)found;
  ACM_Aspect *new_elem = newAspect();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  if (!in_project) new_elem->get_tunits().insert (_tunit_file);
  new_elem->set_intro_target(in_project);
  scope->get_children().insert(new_elem);
  map_insert(*scope, *new_elem, name);
  return new_elem;
}


TU_Aspect *ClangModelBuilder::register_aspect (clang::RecordDecl *ci, ACM_Name *parent) {
  // only classes are relevant
  if (!is_valid_model_class (ci))
    return 0;

  // find the parent model element
  if (!parent && !(parent = register_scope (ci)))
    return 0;

  string name = ci->getName();
  TU_Aspect *elem = 0;
  ACM_Name *found = map_lookup(*parent, name);
  if (!found || found->type_val() != JPT_Aspect)
    return 0;
  elem = (TU_Aspect*)found;

  // set the aspect attributes
  elem->decl (ci);

  register_constructor_and_destructor (ci, elem);
  return elem;
}


bool ClangModelBuilder::is_intro_target (clang::Decl *decl) const {
  clang::SourceManager &SM = _project.get_compiler_instance()->getSourceManager();
  const llvm::MemoryBuffer *buf = SM.getBuffer(SM.getFileID(decl->getLocation()));
  if (IntroductionUnit::cast(buf))
    return true;

  // FIXME: Templates.
  clang::PresumedLoc PL = SM.getPresumedLoc(decl->getLocation());
  llvm::StringRef Name = PL.getFilename();
  return !Name.empty() && get_project().isBelow(Name.str().c_str());
}


ACM_Namespace * ClangModelBuilder::register_namespace1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Namespace *result = 0;
  assert(scope || name == "::");
  ACM_Name *found = (scope ? map_lookup(*scope, name) : get_root());
  if (!found || found->type_val() != JPT_Namespace) {
    result = newNamespace();
    result->set_name(name);
    result->set_builtin(false);
    if (!in_project) result->get_tunits().insert (_tunit_file);
    if (scope) {
      scope->get_children().insert(result);
      map_insert(*scope, *result, name);
    }
    else {
      set_root(result);
    }
  }
  else
    result = (TU_Namespace*)found;
  return result;
}


TU_Namespace *ClangModelBuilder::register_namespace (clang::NamespaceDecl *n,
  ACM_Name *parent) {
    // not all Puma namespaces should be registered
  if (!is_valid_model_namespace (n))
    return 0;

  // find the parent model element
  if (!parent && /*!n->GlobalScope () &&*/ !(parent = register_scope (n)))
    return 0;

  TU_Namespace *new_elem = 0;
  if (!parent) {
    if (get_root())
      return (TU_Namespace*)get_root();
    new_elem = newNamespace();
    new_elem->set_name("::");
    new_elem->set_builtin(true);
    set_root(new_elem);
  }
  else {
    // Use <unnammed> for Puma compatibility.
    string name =
        n->isAnonymousNamespace() ? "<unnamed>" : n->getNameAsString();
    ACM_Name *found = map_lookup(*parent, name);
    if (found && found->type_val() == JPT_Namespace)
      return (TU_Namespace*)found;
    new_elem = newNamespace();
    new_elem->set_name(name);
    new_elem->set_builtin(false);
    parent->get_children().insert(new_elem);
    map_insert(*parent, *new_elem, name);
  }

  // set namespace attributes
  new_elem->decl (n);
  if (new_elem != get_root ())
    add_source_loc (new_elem, n);
  return new_elem;
}

// This method creates a new built-in operator-"pseudo"-function and a call
// of this function and registers both in the AspectC++-join-point-model.
TU_Builtin* ClangModelBuilder::register_builtin_operator_call(clang::Expr* built_in_operator, clang::DeclaratorDecl* lexical_parent_decl, const int local_id) {
  // This should not get called with a null-pointer:
  assert(built_in_operator > 0 && lexical_parent_decl > 0);

  // check if joinpoints for builtin-operators are wanted
  if( ! _conf.builtin_operators() )
    return 0;

  // Check for a built-in operator expression type:
  if( ! TI_Builtin::is_builtin_operator( built_in_operator ) )
    return 0;

  // Check if this is an attempt to insert an operator multiple times.
  // TODO: This check only works, if this member function is called with the same operator consecutively.
  // Maybe something like a SmallPtrSet is necessary.
  static clang::Expr* last_inserted_operator = 0;
  if(last_inserted_operator == built_in_operator) {
    return 0;
  }
  else {
    last_inserted_operator = built_in_operator;
  }


  // Analyze the lexical-parent-decl. If it is valid, then get the appropriate AspectC++-model-element:
  ACM_Name* lexical_model_parent = 0;
  clang::FunctionDecl* lexical_parent_function_decl = clang::dyn_cast<clang::FunctionDecl>(lexical_parent_decl);
  clang::VarDecl* lexical_parent_var_decl = clang::dyn_cast<clang::VarDecl>(lexical_parent_decl);
  if( lexical_parent_function_decl ) {
    // Is this function no template-function(TODO: see TODO in register_call)?
    if( lexical_parent_function_decl->isTemplateInstantiation() )
      return 0;

    lexical_model_parent = register_function( lexical_parent_function_decl ); // register_function checks for valid model function
  }
  else if( lexical_parent_var_decl )
    lexical_model_parent = register_variable( lexical_parent_var_decl );

  // Is there a valid lexical parent?
  if( ! lexical_model_parent )
    return 0;

  // Cache the AST-context:
  clang::ASTContext& AST_context = _project.get_compiler_instance()->getASTContext(); // can not have "const"-qualifier, because of
                                                                                      // Expr::isConstantInitializer(...) (but it seems
                                                                                      // that there is no reason why the ASTContext-argument
                                                                                      // does not have the "const"-qualifier)
  // Cache the operator-kind as std::string:
  const std::string operator_kind_string = TI_Builtin::operator_kind_string( built_in_operator );

  // If the operator is in a constant expression (e.g. "2 + 5" in "static char foo[2 + 5];"), we can not weave because we would destroy the
  // constancy. Do not consider this joinpoint in this case:
  // (This is no big disadvantage, because normally the constant expression will be folded by the compiler and therefore the operator is
  // not available at runtime anyway.)
  // (See C++-standard chapter 5.19 (Constant expressions [expr.const]) and http://lists.cs.uiuc.edu/pipermail/cfe-dev/2012-May/021549.html
  // (Clang Front End for LLVM Developers' List))
  // TODO: In some cases there is no problem with destroying the constancy.
  // TODO: If C++1y or later is used, we could analyze the advice-Code to determine whether the call-wrapper could get the constexpr specifier.
  if( built_in_operator->isConstantInitializer( AST_context, false ) )
    return 0;

  // If the operator refers to a bit-field (e.g. if the first argument of an assignment- or increment-/decrement-operator is a bit-field), we can
  // not get a reference to this bit-field, because the C++-standard forbids it: "The address-of operator & shall not be applied to a bit-field,
  // so there are no pointers to bit-fields. A non-const reference shall not be bound to a bit-field (8.5.3)." (see C++-standard chapter
  // 9.6 (Bit-fields [class.bit]) point 3 for more information)
  // The implementation of Expr::getSourceBitfield in Clang 3.4 misses the case of the unary prefix increment/decrement operator (see
  // https://web.archive.org/web/20140819102610/http://clang.llvm.org/doxygen/Expr_8cpp_source.html#l03247 vs.
  // http://clang.llvm.org/doxygen/Expr_8cpp_source.html#l03351). Therefore we handle this case below together with the other special
  // cases of the unary operator. Additionally Expr::getSourceBitfield in Clang 3.5 misses the case of the unary postfix increment/decrement operator.
  // Only consider this join-point if there are no references to a bitfield:
  if( built_in_operator->getSourceBitField() )
    return 0;

  // Name of the operators as string:
  const std::string operator_name = "operator " + operator_kind_string;

  // Debug-output:
  //cout << "operator-name: " << operator_name << endl;

  // Determine the suitable return-type:
  clang::QualType operator_result_type = TI_Builtin::result_type(built_in_operator, AST_context);

  // Check if the result type is valid (e.g. unnamed types are not valid)
  if(!is_valid_model_type(operator_result_type)) {
    return 0;
  }

  // Special cases:
  if( clang::UnaryOperator* unary_operator = llvm::dyn_cast<clang::UnaryOperator>( built_in_operator ) ) {
    // Unary operator:

    // Because Expr::getSourceBitField() misses min. one of this cases, we check for it here. (see above for more information)
    if(unary_operator->isIncrementDecrementOp() == true && unary_operator->getSubExpr()->getSourceBitField() > 0)
      return 0; // Do not consider this join-point

    // Check whether the operator-kind is address-of and the return-type is a member-(function-)-pointer-type:
    if( operator_kind_string == "&" && operator_result_type->isMemberPointerType() )
      // We can not weave at unary address-of-operators on member-(function-)pointers, because these types do not exist as a
      // non-pointer-type (as type of a variable)
      return 0; // Do not consider this join-point
  }

  // Check whether this is a pointer-to-member-operator that works on member-functions (instead of "normal" data members):
  if( ( operator_kind_string == ".*" || operator_kind_string == "->*" ) &&
      TI_Builtin::arg_type( built_in_operator, AST_context, 1 )->isMemberFunctionPointerType() )
    //   or: if(... && result_type_as_string == "<bound member function type>")
    // The C++-standard states in chapter 5.5 (Pointer-to-member operators [expr.mptr.oper]) point 6:
    // "If the result of .* or ->* is a function, then that result can be used only as the operand for the function call operator ()."
    // Because of that we can not cache the result of the ".*"- oder "->*"-Operator ==> we can not weave at this join-point:
    return 0; // Do not consider this join-point

  // Operator-signature (the resulttype is not necessary):
  std::string operator_signature = operator_name + "(";
  // Check the argument types and if they are valid, then add them to the operator-signature:
  unsigned argnum = TI_Builtin::arg_count( built_in_operator );
  for( unsigned a = 0; a < argnum; a++ ) {
    clang::QualType curr_arg_type = TI_Builtin::arg_type( built_in_operator, AST_context, a );
    if(!is_valid_model_type(curr_arg_type)){
      return 0;
    }
    operator_signature += ( ( a != 0 ) ? "," : "" ) + TI_Type::get_type_sig_text(curr_arg_type, &AST_context, 0, true);
  }
  operator_signature += ")";

  // Debug-output:
  //cout << "operator-full-signature: " << operator_result_type.getAsString(AST_context.getPrintingPolicy()) << " " << operator_signature << endl;

  // Root-namespace:
  ACM_Namespace* root_namespace = get_root();

  // Does the pseudo-operator-function already exist in the AspectC++-model?
  TU_Function* model_operator_function = (TU_Function*)map_lookup(*root_namespace, operator_signature);
  if( ! model_operator_function ) {
    // Create the pseudo-operator-function:
    model_operator_function = newFunction();

    // Name of the function:
    model_operator_function->set_name(operator_name);

    // Insert the function into the lookup-map:
    map_insert(*root_namespace, *model_operator_function, operator_signature);

    // Set attributes of the function:
    model_operator_function->set_variadic_args(false); // No variable argument-count
    model_operator_function->set_kind(FT_NON_MEMBER); // No member, because in global scope
    model_operator_function->set_builtin(true); // Built-in
    model_operator_function->set_cv_qualifiers(CVQ_NONE); // Neither const nor volatile, because built-in operators are no member-functions

    // Add operator-arguments:
    ACM_Container<ACM_Type, true>& model_operator_function_arg_types = model_operator_function->get_arg_types(); // Caching
    model_operator_function_arg_types.clear(); // Delete arguments
    for( unsigned a = 0; a < argnum; a++ )
      model_operator_function_arg_types.insert( register_type( TI_Builtin::arg_type( built_in_operator, AST_context, a ).getCanonicalType() ) );

    model_operator_function->set_result_type(register_type(operator_result_type)); // Set return-type
    model_operator_function->decl(0); // No pointer to operator-declaration, because the operator is built-in

    // Insert the function into the model as children of the root-namespace-node:
    root_namespace->get_children().insert(model_operator_function);

    // Mark the function as 'used' by this translation unit,
    // to (hopefully) avoid garbage collection on xml-model merge
    model_operator_function->get_tunits ().insert (_tunit_file);
  }

  // Create the operator-call:
  TU_Builtin *model_operator_call = newBuiltin();
  model_operator_call->set_target(model_operator_function);
  model_operator_call->set_lid(local_id);
  model_operator_call->tree_node(built_in_operator);
  model_operator_call->origin(lexical_parent_decl);

  // Add the source-location:
  add_source_loc(model_operator_call, built_in_operator->getLocStart(), built_in_operator->getLocEnd(), SLK_NONE);

  // Insert the operator-call into the model as children of the lexical-model-parent-node:
  lexical_model_parent->get_children().insert(model_operator_call);

  // Return the new operator-call:
  return model_operator_call;
}

ACM_Access *ClangModelBuilder::register_call (clang::FunctionDecl *called,
    clang::CallExpr *call_node, clang::DeclaratorDecl *caller, int local_id) {

  clang::FunctionDecl *fd_caller = 0;
  clang::VarDecl *vd_caller = 0;

  if (caller) {
    fd_caller = clang::dyn_cast<clang::FunctionDecl>(caller);
    if (fd_caller && fd_caller->isTemplateInstantiation ())
      // TODO: call joinpoint located in template instance silently ignored here
      return 0;
    vd_caller = clang::dyn_cast<clang::VarDecl>(caller);
  }

  // what is the lexical scope of this call?
  ACM_Name *lexical = 0;
  if (!caller) {
    lexical = 0; // a pseudo call join point
  }
  else if (fd_caller) {
    // TODO: better cache the JPL object of the current function
    ACM_Any *loc = register_function (fd_caller, 0);
    if (!loc) {
      // TODO: calls in advice code are silently ignored here at the moment
//      _err << sev_error << call_node->token ()->location ()
//           << "location of function call invalid" << endMessage;
      return 0;
    }
    lexical = (ACM_Name*)loc;
  }
  else {
    lexical = register_variable (vd_caller);
    if (!lexical) {
      _err << Puma::sev_warning << call_node->getLocStart ()
           << "location of function call invalid" << Puma::endMessage;
      return 0;
    }
  }

  // call using function pointer or function reference: add callref node
  if (!called) {
    assert (call_node);

    if (llvm::dyn_cast<clang::CXXOperatorCallExpr>(call_node)) {
      // In a C++ template, this expression node kind will be used whenever any
      // of the arguments are type-dependent.
      // Thus, this expression does not represent a callref node.
      return 0;
    }

    ACM_CallRef *new_elem = newCallRef();
    new_elem->set_lid(local_id);

    add_source_loc (new_elem, call_node);
    // set the parent in the join point model structure
    if (lexical) { // pseudo-calls are invisible
      lexical->get_children().insert(new_elem);
    }

    clang::QualType type( call_node->getCallee()->IgnoreParenImpCasts()->getType().getCanonicalType() );
    if (type->isFunctionPointerType())
      type = type->getPointeeType();

    //TODO: get correct type for member-function pointers,
    //      e.g., by clang::Expr::findBoundMemberType( ... ) or similar
    //TODO: new_elem->set_target_class( ... ) for member member-function pointers

    // additionally set the reference type
    TU_Type *ref_type = register_type( type );
    new_elem->set_type( ref_type );

    return new_elem;
  }

  // find the called function in the join point model
  ACM_Function *called_func = register_function (called);
  if (!called_func) {
//  if a called function is, for instance, a member of a local class, it is
//  perfectly valid that we don't find it in the model -> ignore call join-point
//    _err << sev_error << "called function \'" << called->QualName ()
//         << "\' not found in join point model" << endMessage;
    return 0;
  }

  TU_MethodCall *new_elem = newCall();
  new_elem->set_target(called_func);
  called_func->get_calls().insert(new_elem);
  new_elem->set_lid(local_id);
  new_elem->called (called);
  new_elem->origin (caller);
  new_elem->tree_node( call_node );

  // perform static analyses
  if ( _conf.flow_analysis() ) {
    unsigned int basic_block_id = _flow_analysis.get_block_lid(caller, call_node);
    if (_flow_analysis.valid()) {
      new_elem->set_cfg_block_lid(basic_block_id);
    }
    unsigned int target_obj_lid = _flow_analysis.get_target_obj_lid(caller, call_node);
    if (_flow_analysis.valid() && target_obj_lid != ClangFlowAnalysis::NO_ID) {
      new_elem->set_target_object_lid(target_obj_lid);
    }
  }

  if (call_node)
    add_source_loc (new_elem, call_node);
  // set the parent in the join point model structure
  if (lexical) { // pseudo-calls are invisible
    lexical->get_children().insert(new_elem);
  }

  // For functions with default arguments, not more than the number of args
  // in the call expression is used.
  if (call_node && (new_elem->is_call_op() ||
      !clang::dyn_cast<clang::CXXOperatorCallExpr> (call_node))) {
    // argument types are the types from the target function declaration
    unsigned args = called->getNumParams ();
    unsigned call_args = call_node->getNumArgs ();
    unsigned given;
    for (given = 0; given < call_args; given++) {
      clang::Expr *arg = call_node->getArg (given);
      if (clang::dyn_cast<clang::CXXDefaultArgExpr> (arg))
        break;
    }
    // call operator calls always have the object as argument 0 => substract one
    if (new_elem->is_call_op())
      given--;
    if (called->isVariadic ()) {
      for (unsigned a = args; a < call_args; a++) {
        clang::QualType arg_type = call_node->getArg (a)->getType ();
        new_elem->get_variadic_arg_types().insert(register_type (arg_type));
      }
    }
    else if (given < args) {
      new_elem->set_default_args(args - given);
    }
  }

  // analyze the target object type of this call
  if (call_node) {
    clang::RecordDecl *rd = (clang::RecordDecl *)new_elem->target_class();
    if (rd) {
      ACM_Class *target_class = register_aspect(rd);
      if (!target_class) target_class = register_class (rd);
      assert (target_class);
      new_elem->set_target_class(target_class);
    }
  }

  return new_elem;
}

ACM_Access *ClangModelBuilder::register_get( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src ) {
  // check if data-based joinpoints are wanted
  if( ! _conf.data_joinpoints() )
    return 0;

  // regen model information if not given
  if( ! var.element && var.decl )
    var.element = register_variable( var.decl );

  // dont create joinpoint in functions that where left out of the model
  if( ! context.parent )
    return 0;

  // generate node
  if( var.element ) {
    TU_Get *new_elem = newGet();
    fillAccessNode( new_elem, var, context, var_src );

    return new_elem;
  }
  else {
    TU_GetRef *new_elem = newGetRef();
    fillRefAccessNode( new_elem, var, context );

    return new_elem;
  }
}

ACM_Access *ClangModelBuilder::register_set( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src ) {
  // check if data-based joinpoints are wanted
  if( ! _conf.data_joinpoints() )
    return 0;

  // regen model information if not given
  if( ! var.element && var.decl )
    var.element = register_variable( var.decl );

  // dont create joinpoint in functions that where left out of the model
  if( ! context.parent )
    return 0;

  // generate node
  if( var.element ) {
    TU_Set *new_elem = newSet();
    fillAccessNode( new_elem, var, context, var_src );

    return new_elem;
  }
  else {
    TU_SetRef *new_elem = newSetRef();
    fillRefAccessNode( new_elem, var, context );

    return new_elem;
  }
}

TU_Ref *ClangModelBuilder::register_ref( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src ) {
  // check if data-based joinpoints are wanted
  if( ! _conf.data_joinpoints() )
    return 0;

  // regen model information if not given
  if( ! var.element )
    var.element = register_variable( var.decl );

  // dont create joinpoint in functions that where left out of the model
  if( ! context.parent )
    return 0;

  // generate node
  TU_Ref *new_elem = newRef();
  fillAccessNode( new_elem, var, context, var_src );
  // addtionally set the reference type
  TU_Type *ref_type = register_type( new_elem->result_type() );
  new_elem->set_type( ref_type );

  return new_elem;
}

template<class NODE> void ClangModelBuilder::fillAccessNode( NODE *new_elem, VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src ) {
  // fill ACModel information
  new_elem->set_variable( var.element );
  new_elem->set_lid( context.local_id );
  if( var.tree_node )
    add_source_loc( new_elem, var.tree_node );

  assert( context.parent );
  if( context.parent->type_val() == JPT_Builtin )
    static_cast<ACM_Builtin *>( context.parent )->get_implicit_access().insert( new_elem );
  else if( context.parent->type_val() & JPT_Name )
    static_cast<ACM_Name *>( context.parent )->get_children().insert( new_elem );
  else
    assert( false && "Unknown parent type for joinpoint" );

  // fill Transform information
  new_elem->variable( var.decl );
  new_elem->tree_node( var.tree_node );
  new_elem->ref_node( var.ref_node );
  new_elem->origin( context.parent_decl );

  // remember dependency chain
  if( var_src )
    new_elem->entity_src( var_src );

  // analyze the target object type of this call
  if( var.tree_node ) {
    if( clang::RecordDecl *rd = const_cast<clang::RecordDecl *>( new_elem->target_class() ) ) { // FIXME: cast should not be necessary, but most place use non const pointers to AST Elements
      ACM_Class *target_class = register_aspect( rd ); // try aspect first ...
      if( ! target_class ) // ... if not successful, try class
        target_class = register_class( rd );
      assert( target_class );
      new_elem->set_target_class( target_class );
    }
  }
}

template<class NODE> void ClangModelBuilder::fillRefAccessNode( NODE *new_elem, VarAccessInfo &var, JoinpointContext& context ) {
  // fill ACModel information
  new_elem->set_lid( context.local_id );
  if( var.tree_node )
    add_source_loc( new_elem, var.tree_node );

  assert( context.parent );
  if( context.parent->type_val() == JPT_Builtin )
    static_cast<ACM_Builtin *>( context.parent )->get_implicit_access().insert( new_elem );
  else if( context.parent->type_val() & JPT_Name )
    static_cast<ACM_Name *>( context.parent )->get_children().insert( new_elem );
  else
    assert( false && "Unknown parent type for joinpoint" );

  // fill Transform information
  new_elem->tree_node( var.tree_node );
  new_elem->ref_node( var.ref_node );
  new_elem->origin( context.parent_decl );

  // this need the transform infos to avoid duplication
  TU_Type *ent_type = register_type( new_elem->entity_type() );
  new_elem->set_type( ent_type );
}

// create a new execution join point
TU_Method *ClangModelBuilder::register_execution (ACM_Function *ef) {
  clang::FunctionDecl *func = ((TI_Function*)ef->transform_info ())->decl ();
  TU_Method *new_elem = newExecution();
  ef->get_children().insert(new_elem);
  new_elem->decl (func);
  return new_elem;
}

// create a new construction join point
TU_Construction *ClangModelBuilder::register_construction (ACM_Function *cf) {
  assert (cf);
  clang::FunctionDecl *func = ((TI_Function*)cf->transform_info ())->decl ();
  TU_Construction *new_elem = newConstruction();
  cf->get_children().insert(new_elem);
  new_elem->decl (func);
  clang::CXXRecordDecl *d =
      llvm::cast<clang::CXXRecordDecl>(TI_Class::of (*((ACM_Class*)cf->get_parent()))->decl ());
  new_elem->that_decl(d);
  return new_elem;
}

// create a new construction join point
TU_Destruction *ClangModelBuilder::register_destruction (ACM_Function *df) {
  assert (df);
  clang::FunctionDecl *func = ((TI_Function*)df->transform_info ())->decl ();
  TU_Destruction *new_elem = newDestruction();
  df->get_children().insert(new_elem);
  new_elem->decl (func);
  clang::CXXRecordDecl *d =
      llvm::cast<clang::CXXRecordDecl>(TI_Class::of (*((ACM_Class*)df->get_parent()))->decl ());
  new_elem->that_decl(d);
  return new_elem;
}

ACM_Name *ClangModelBuilder::register_scope( clang::DeclaratorDecl *obj ) {
  return register_scope (obj->getDeclContext (), false);
}

ACM_Name *ClangModelBuilder::register_scope (clang::DeclContext *obj, bool parent) {
  ACM_Name *result = 0;
  clang::DeclContext *scope = (parent ? obj->getParent() : obj);
  if (scope) {
    if (clang::NamespaceDecl *ND = llvm::dyn_cast<clang::NamespaceDecl>(scope))
      result = register_namespace (ND);
    else if (clang::CXXRecordDecl *RD =
             llvm::dyn_cast<clang::CXXRecordDecl>(scope)) {
      result = register_aspect (RD);
      if (!result)
        result = register_class (RD);
    } else if (llvm::isa<clang::TranslationUnitDecl>(scope)) {
      result = register_namespace1(0, "::");
    } else if (clang::LinkageSpecDecl *LSD =
               llvm::dyn_cast<clang::LinkageSpecDecl>(scope))
      result = register_scope(LSD);
  }

//  in some cases, e.g. join-points within local classes it can happen that
//  the scope of a join-point is not known in the model -> no error!
//  if (!result) {
//    _err << sev_error << "parent '" << scope_name (obj).c_str ()
//       << "' of model element " << obj->QualName () << " not found"
//       << endMessage;
//  }

  return result;
}


// add the source location to a model element by using the syntax tree node
void ClangModelBuilder::add_source_loc (ACM_Any *name, ClangToken token, ClangToken end_token, SourceLocKind kind) {
  add_source_loc(name, token.location(), end_token.location(), kind);
}

void ClangModelBuilder::add_source_loc (ACM_Any *name, clang::Decl *tree,
                                   SourceLocKind kind) {
  add_source_loc(name, tree->getLocStart(), tree->getLocEnd(), kind);
}

void ClangModelBuilder::add_source_loc (ACM_Any *name, clang::Stmt *tree,
                                   SourceLocKind kind) {
  add_source_loc(name, tree->getLocStart(), tree->getLocEnd(), kind);
}

void ClangModelBuilder::add_source_loc(ACM_Any *name, clang::SourceLocation location,
                                  clang::SourceLocation endlocation,
                                  SourceLocKind kind) {
  assert(name);
  clang::SourceManager &SM = _project.get_compiler_instance()->getSourceManager();

  // check if this file belong to our project
  //ACFileID funit = ACPreprocessor::source_unit (token);
  //if (!funit)
    //return;

  clang::PresumedLoc PL = SM.getPresumedLoc(location);
  clang::PresumedLoc PLEnd = SM.getPresumedLoc(endlocation);
// here the presumed loc must ignore #line directives! => param false
  clang::PresumedLoc PL_no_line = SM.getPresumedLoc(location, false);
//  clang::PresumedLoc PLEnd = SM.getPresumedLoc(endlocation, false);

  if (PL.isInvalid() || PLEnd.isInvalid())
    return;

  if (!get_project ().isBelow (PL_no_line.getFilename())) {
    if (name->type_val () & JPT_Name) {
      ACM_Name *jpl_name = (ACM_Name*)name;
      // TODO: really use linear search here?
      typedef ACM_Container<ACM_TUnit, false> Container;
      const Container &tunits = jpl_name->get_tunits ();
      bool found = false;
      for (Container::const_iterator i = tunits.begin(); i != tunits.end(); ++i) {
        if (*i == _tunit_file) {
          found = true;
          break;
        }
      }
      if (!found)
        jpl_name->get_tunits ().insert (_tunit_file);
    }
    return;
  }

  clang::FileID ID = SM.getFileID(location);
  ClangModelBuilder::FileMap::iterator i =
      file_map().find(model_filename(PL.getFilename()));
  ACM_File *file = 0;
  if (i != file_map ().end ())
    file = i->second;
  else {
    // TODO: temporary hack
    int len = SM.getSpellingLineNumber(SM.getLocForEndOfFile(ID));
    // TODO: in the future, handle aspect headers differently
    ACM_Header *new_file = newHeader();
    get_files().insert(new_file);
    new_file->set_filename(model_filename(PL.getFilename()));
    new_file->set_len(len);
    new_file->get_in().insert(_tunit_file);
    if (const clang::FileEntry *fe = SM.getFileEntryForID(ID))
      new_file->set_time(fe->getModificationTime());
    else
      new_file->set_time(0);
    file = new_file;
    file_map().insert(
        ClangModelBuilder::FileMapPair(model_filename(PL.getFilename()), file));
  }
  int line = PL.getLine();
  int len = PLEnd.getLine() - line + 1;
  // TODO: really use linear search here?
  typedef ACM_Container<ACM_Source, true> Container;
  const Container &sources = name->get_source ();
  bool found = false;
  for (Container::const_iterator i = sources.begin(); i != sources.end(); ++i) {
    if ((*i)->get_line () == line &&
        (*i)->get_file () == file &&
        (*i)->get_kind () == kind &&
        (*i)->get_len () == len) {
//      if ((*i)->get_len () != len)
//        _err << sev_warning << "Internal problem: Length differs for source at same location (file='" << file->get_filename().c_str () << "', line="
//        << line << ")" << endMessage;
      found = true;
      break;
    }
  }
  if (!found) {
    ACM_Source *source = newSource();
    assert(file);
    source->set_file(file);
    source->set_line(line);
    source->set_len(len);
    source->set_kind(kind);
    name->get_source().insert(source);
  }
}


// Analyze a declaration and add found attributes to the model element 'name'
void ClangModelBuilder::handle_attributes (ACM_Name *name, clang::Decl *decl) {

  if (!decl->hasAttrs())
    return;
  const clang::AttrVec &attrs = decl->getAttrs();
  vector<string> attrStructNames;
  for(const clang::Attr *attr_use : attrs) {
    AnnotationMap::const_iterator iter = annotation_map().find(attr_use->getLocation());
    if (iter == annotation_map().end())
      continue;
    const Annotation &annotation = iter->second;
//    cout << signature(*name) << ": " << annotation.to_string() << std::endl;
    if (!annotation.params.empty())
      _err << Puma::sev_warning << attr_use->getLocation() << "attribute parameters of '"
        << annotation.get_qualified_name().c_str() << "' ignored" << Puma::endMessage;
    TU_Attribute *attr = find_attrdecl(annotation.attrNames);
    if (!attr && annotation.is_user_defined)
      _err << Puma::sev_error << attr_use->getLocation() << "undeclared attribute '"
        << annotation.get_qualified_name().c_str() << "' used" << Puma::endMessage;
    if (!attr)
      continue;
    if (decl->isFirstDecl()) {
      // in the first declaration of an entity all attributes must be present
      if (name->get_attributes().get_sorted().find(attr) !=
          name->get_attributes().get_sorted().end()) {
        // the same attribute use more than once! => error
        _err << Puma::sev_warning << attr_use->getLocation() << "attribute '"
          << annotation.get_qualified_name().c_str()
          << "' used more than once on the same entity" << Puma::endMessage;
      }
      else
        name->get_attributes().insert(attr);
    }
    else {
      if (name->get_attributes().get_sorted().find(attr) ==
          name->get_attributes().get_sorted().end()) {
        _err << Puma::sev_error << attr_use->getLocation() << "attribute '"
          << annotation.get_qualified_name().c_str()
          << "' added after first declaration" << Puma::endMessage;
      }
    }
  }
}


