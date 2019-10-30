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

#ifndef __ClangTransformInfo_h__
#define __ClangTransformInfo_h__

#include "ACModel/Elements.h"
#include "ACToken.h"
#include "ACFileID.h"
#include "ThisJoinPoint.h"
#include "PointCutExpr.h"
#include "SyntacticContext.h"
#include "CFlow.h"
#include "WeaverBase.h"
#include "ClangAdjustedTypePrinter.h"
#include "version.h"

#include "clang/Basic/Version.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"
#if !(CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) && \
  !(CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
#include "clang/AST/ExprCXX.h"
#endif
#include "llvm/Support/raw_ostream.h"
#include "clang/AST/CXXInheritance.h"


class TI_CodeAdvice : public ModelTransformInfo {
  // Condition is located here instead of in the AdviceInfo object because
  // there are cases where the conditions differ but the advice info
  // is the same.
  const Condition *_condition;
  AdviceInfo *_advice_info;
public:
  void set_condition (const Condition *c) { _condition = c; }
  const Condition *get_condition () const { return _condition; }
  void set_advice_info (AdviceInfo *ai) { _advice_info = ai; }
  AdviceInfo *get_advice_info () const { return _advice_info; }

  static TI_CodeAdvice *of (const ACM_CodeAdvice &loc) {
    return static_cast<TI_CodeAdvice*>(loc.transform_info ());
  }
};

class TransformInfo : public ModelTransformInfo {
public:
  virtual ModelNode &jpl () = 0;
  virtual clang::Decl *decl () const = 0;

  static inline const TransformInfo *of (const ModelNode &loc);
  static inline clang::Decl *decl (const ModelNode &loc);
  static inline Puma::Location location (const ModelNode &loc);

  static const WeavePos &get_pos_after_token (clang::SourceLocation loc,
      WeaverBase &wb, WeavePos::Pos pos = WeavePos::WP_AFTER) {
    return wb.get_pos_after_loc(loc, pos);
  }

  // parameter 'is_id': if true, the searched string is only replaced if the
  //                    character before and after the matching substring is
  //                    not in an identifier.
  //                    (example: don't replace 'shortcut' by 'short intcut')
  static void replace_in_string(std::string& subject, const std::string& search,
      const std::string& replace, bool is_id = false) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
      bool id_before = (pos > 0 && (subject[pos - 1] == '_' || std::isalpha(subject[pos - 1])));
      bool id_after  = ((pos + search.length () < subject.length()) &&
          (subject[pos + search.length()] == '_' || std::isalnum(subject[pos + search.length()])));
      if (!is_id || (!id_before && !id_after)) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
      }
      else
        pos += search.length();
    }
  }

  static bool needs_this (clang::FunctionDecl *func) {
    if (clang::CXXMethodDecl *m = llvm::dyn_cast<clang::CXXMethodDecl>(func))
      if (!m->isStatic())
        return true;

    return false;
  }

  // TODO: duplicate of function in ClangModelBuilder.cc
  template <typename T>
  static bool isTemplateInstantiation(T node) {
    return (node->getTemplateSpecializationKind() ==
                clang::TSK_ImplicitInstantiation ||
            node->getTemplateSpecializationKind() ==
                clang::TSK_ExplicitInstantiationDefinition);
  }

  // TODO: duplicate of function in ClangModelBuilder.cc
  static bool inside_template_instance (clang::DeclContext *scope) {
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

};

inline const TransformInfo *TransformInfo::of (const ModelNode &loc) {
  return loc.transform_info () ? (TransformInfo*)loc.transform_info () : 0;
}
//inline CTree *TransformInfo::tree (const ModelNode &loc) {
//  return loc.transform_info () ?
//    ((TransformInfo*)loc.transform_info ())->tree () : 0;
//}
//inline Unit *TransformInfo::unit (const ModelNode &loc) {
//  return loc.transform_info () ?
//    ((TransformInfo*)loc.transform_info ())->unit () : 0;
//}
inline clang::Decl *TransformInfo::decl (const ModelNode &loc) {
  return loc.transform_info () ?
    ((TransformInfo*)loc.transform_info ())->decl () : 0;
}
inline Puma::Location TransformInfo::location (const ModelNode &loc) {
  return Puma::Location (); // FIXME: implement for Clang
}


class TI_Type : public TransformInfo {
  clang::QualType _type;
public:
  void type (clang::QualType ti) { _type = ti; }
  clang::QualType type () const { return _type; }

  virtual clang::Decl *decl () const { return 0; }

  bool is_const () const { return _type.isConstQualified (); }
  bool is_reference () const { return _type.getTypePtr ()->isReferenceType (); }

  // Called from ClangModelBuilder::register_type(...) and ClangModelBuilder::register_arg(...)
  // and used for signature.
  static string name (clang::ASTContext &ctx, clang::QualType type_info) {
    // Get the type as a string, looking through template parameters
    // and typedefs and printing anonymous as unnamed:
    string type_str = TI_Type::get_type_text(type_info, &ctx, "?", TSEF_DONOTCHANGE, false, TSEF_DISABLE, true, true, false);

    int last = type_str.length() - 1;
    if (type_str[last] == '?') last--;
    while(type_str[last] == ' ') last--;

    return type_str.substr (0, last + 1);
  }

private:
  // Returns a printing policy setup according to given flags for adjusted type printing
  static inline clang::PrintingPolicy get_preset_printing_policy(const clang::ASTContext* ctx,
                                                                 bool unnamed) {
    // TODO: always create, setup and use own printing policy?
    clang::PrintingPolicy policy = ctx
        ? ctx->getPrintingPolicy()
        : clang::PrintingPolicy(clang::LangOptions());

    // Ensure some policy properties:
    policy.SuppressSpecifiers = false;
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    policy.IncludeTagDefinition = false;
#else
    policy.SuppressTag = false;
#endif
    // Suppress <anonymous> only if we do not want to replace it with
    // <unnamed> later
    policy.SuppressUnwrittenScope = !unnamed;
    policy.AnonymousTagLocations = unnamed;

    return policy;
  }

  // Unifies the types in the given string according to the given flags.
  static inline string& get_with_unified_types(string& input, bool unnamed) {
    if(unnamed) {
      // If requested, replace occurences of anonymous-variants with "<unnamed>":
      const char* unnamed_namespace_str =
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
          "<anonymous>"
#else // C++ 11 interface
          "(anonymous namespace)"
#endif
      ;
      replace_in_string(input, unnamed_namespace_str, "<unnamed>");
    }

    // Unify some types: (taken from fix_type_in_signature(...)
    // and fixed "long int" -> "long int int" bug)
    replace_in_string(input, "long", "long int", true);
    replace_in_string(input, "long int long int", "long long int", true);
    replace_in_string(input, "long int int", "long int", true);
    replace_in_string(input, "long int double", "long double", true);
    replace_in_string(input, "short", "short int", true);
    replace_in_string(input, "short int int", "short int", true);
    // TODO: Is the following line necessary?
    replace_in_string(input, "<anonymous namespace>::", unnamed ? "<unnamed>::" : "");
    replace_in_string(input, "*restrict", "*");
    replace_in_string(input, " restrict", " ");
    replace_in_string(input, "*__restrict", "*");
    replace_in_string(input, " __restrict", " ");

    return input;
  }

public:

  /** Returns the textual representation of a declaration name.
      *  \param decl The declaration
      *  \param ctx The ASTContext
      *  \param absolute_qualified Add all nested name specifiers as well as the
      *   root qualifier (ENABLE), do not change nested name specifiers (DONOTCHANGE)
      *   or remove all nested name specifiers (DISABLE).
      *  \param keep_typedef If the type is typedef'd, keep the typedef and return
      *   the typedef'd type (and not the underlying type)
      *  \param elaborated_type_spec Add elaborated type specifier before
      *                              class, union, and enumeration types.
      *  \param unnamed Do not suppress printing of unwritten scope and convert printed
      *   namespaces to '<unnamed>'
      *  \param remove_attributes Removes occurrences of attributes
      *    */
  static string get_decl_name_text(const clang::NamedDecl* decl,
                              TriStateEnableFeature absolute_qualified = TSEF_DONOTCHANGE,
                              bool keep_typedef = true,
                              TriStateEnableFeature elaborated_type_spec = TSEF_DONOTCHANGE,
                              bool unnamed = false,
                              bool remove_attributes = false
  ) {
    clang::PrintingPolicy policy = get_preset_printing_policy(&decl->getASTContext(), unnamed);
    string string_buffer;
    llvm::raw_string_ostream stream(string_buffer);

    AdjustedTypePrinter(policy, absolute_qualified, keep_typedef, elaborated_type_spec,
                               remove_attributes, false, 0)
        .adjusted_NamedDecl_printQualifiedName(decl, stream);
    return get_with_unified_types(stream.str(), unnamed);
  }

  /** Returns the textual representation of a list of template arguments.
     *  \param template_args The list of template arguments
     *  \param ctx The ASTContext
     *  \param absolute_qualified Add all nested name specifiers as well as the
     *   root qualifier (ENABLE), do not change nested name specifiers (DONOTCHANGE)
     *   or remove all nested name specifiers (DISABLE).
     *  \param keep_typedef If the type is typedef'd, keep the typedef and return
     *   the typedef'd type (and not the underlying type)
     *  \param elaborated_type_spec Add elaborated type specifier before
     *                              class, union, and enumeration types.
     *  \param unnamed Do not suppress printing of unwritten scope and convert printed
     *   namespaces to '<unnamed>'
     *  \param remove_attributes Removes occurrences of attributes
     *    */
  static string get_templ_arg_list_text(const clang::TemplateArgumentList& template_args,
                                           const clang::ASTContext* ctx,
                                           TriStateEnableFeature absolute_qualified = TSEF_ENABLE,
                                           bool keep_typedef = true,
                                           TriStateEnableFeature elaborated_type_spec = TSEF_DONOTCHANGE,
                                           bool unnamed = false,
                                           bool remove_attributes = false
  ) {
    clang::PrintingPolicy policy = get_preset_printing_policy(ctx, unnamed);
    string string_buffer;
    llvm::raw_string_ostream stream(string_buffer);
    AdjustedTypePrinter(policy, absolute_qualified, keep_typedef, elaborated_type_spec,
                           remove_attributes, false, ctx)
        .adjusted_PrintTemplateArgumentList(stream,
                                            template_args.data(),
                                            template_args.size());
    return get_with_unified_types(stream.str(), unnamed);
  }

  /** Returns the textual representation of template argument.
    *  \param temp_arg The template argument
    *  \param ctx The ASTContext
    *  \param absolute_qualified Add all nested name specifiers as well as the
    *   root qualifier (ENABLE), do not change nested name specifiers (DONOTCHANGE)
    *   or remove all nested name specifiers (DISABLE).
    *  \param keep_typedef If the type is typedef'd, keep the typedef and return
    *   the typedef'd type (and not the underlying type)
    *  \param elaborated_type_spec Add elaborated type specifier before
    *                              class, union, and enumeration types.
    *  \param unnamed Do not suppress printing of unwritten scope and convert printed
    *   namespaces to '<unnamed>'
    *  \param remove_attributes Remove occurrences of attributes
    *    */
  static string get_templ_arg_text(const clang::TemplateArgument& temp_arg,
                       const clang::ASTContext* ctx,
                       TriStateEnableFeature absolute_qualified = TSEF_ENABLE,
                       bool keep_typedef = true,
                       TriStateEnableFeature elaborated_type_spec = TSEF_DONOTCHANGE,
                       bool unnamed = false,
                       bool remove_attributes = false
  ) {
    clang::PrintingPolicy policy = get_preset_printing_policy(ctx, unnamed);

    string temp_arg_string_buffer;
    llvm::raw_string_ostream temp_arg_stream(temp_arg_string_buffer);
    AdjustedTypePrinter(policy, absolute_qualified, keep_typedef, elaborated_type_spec,
                           remove_attributes, false, ctx)
        .adjusted_TemplateArgument_print(temp_arg, temp_arg_stream);
    return get_with_unified_types(temp_arg_stream.str(), unnamed);
  }

  /** Returns the textual representation of a type for the use in woven code.
      *  \param type The type
      *  \param ctx The ASTContext
      *  \param var_name If this is unequal zero, the type is returned
      *   together with the given variable name
      *   (e.g. void(*)(int) is returned as void(*example)(int) if "example" is
      *   provided as variable name.)
      *    */
  static string get_type_code_text(clang::QualType type, const clang::ASTContext* ctx,
       const char *var_name = (const char*)0
  ) {
    return get_type_text(type,
                         ctx,
                         var_name,
                         TSEF_ENABLE, // make type absolute
                         true, // keep typedef if possible (to keep woven code portable)
                         TSEF_DONOTCHANGE,
                         false, // No unwritten scopes
                         true, // Remove __attribute__s (TODO: There are cases where we can keep them?)
                         false); // Do not make signature parameter type adjustments (because the result is not used for signatures)
  }

  /** Returns the textual representation of a type for the use in AspectC++ signatures.
      *  \param type The type
      *  \param ctx The ASTContext
      *  \param var_name If this is unequal zero, the type is returned
      *   together with the given variable name
      *   (e.g. void(*)(int) is returned as void(*example)(int) if "example" is
      *   provided as variable name.)
      *  \param as_parameter_signature_type Return the type as adjusted for use in the
      *   signature of a function, decaying array and function types and removing top-level
      *   cv-qualifiers. This should be true if the type is associated to a parameter of a function.
      *    */
  static string get_type_sig_text(clang::QualType type, const clang::ASTContext* ctx,
      const char *var_name = (const char*)0,
      bool as_parameter_signature_type = false
  ) {
    // TODO: Common question: Always remove elaborated in signature?
    return get_type_text(type,
                         ctx,
                         var_name,
                         TSEF_DONOTCHANGE, // Do not change nested name specifiers
                         false, // Remove typedefs to make matching independent of typedefs
                         TSEF_DISABLE, // AspectC++ signatures do not contain elaborated specifiers. TODO: Is this always true?
                         false, // Do not add unwritten scopes
                         true, // remove __attribute__s
                         as_parameter_signature_type);
  }

  /** Returns the textual representation of a type.
    *  \param type The type
    *  \param ctx The ASTContext
    *  \param var_name If this is unequal zero, the type is returned
    *   together with the given variable name
    *   (e.g. void(*)(int) is returned as void(*example)(int) if "example" is
    *   provided as variable name.)
    *  \param absolute_qualified Add all nested name specifiers as well as the
    *   root qualifier (ENABLE), do not change nested name specifiers (DONOTCHANGE)
    *   or remove all nested name specifiers (DISABLE).
    *  \param keep_typedef If the type is typedef'd, keep the typedef and return
    *   the typedef'd type (and not the underlying type)
    *  \param elaborated_type_spec Add elaborated type specifier before
    *                              class, union, and enumeration types.
    *  \param unnamed Do not suppress printing of unwritten scope and convert printed
    *   namespaces to '<unnamed>'
    *  \param remove_attributes Remove occurrences of attributes
    *  \param as_parameter_signature_type Return the type as adjusted for use in the
    *   signature of a function, decaying array and function types and removing top-level
    *   cv-qualifiers.
    *    */
  static string get_type_text(clang::QualType type, const clang::ASTContext* ctx,
    const char *var_name = (const char*)0,
    TriStateEnableFeature absolute_qualified = TSEF_ENABLE,
    bool keep_typedef = true,
    TriStateEnableFeature elaborated_type_spec = TSEF_DONOTCHANGE,
    bool unnamed = false,
    bool remove_attributes = false,
    bool as_parameter_signature_type = false
  ) {

    // Check invalid combinations:
    assert(!(keep_typedef && elaborated_type_spec == TSEF_ENABLE) && "A typedef type can not be "
           "printed together with an elaborated type specifier.");
    assert(!(as_parameter_signature_type && !ctx) && "as_parameter_signature_type flag needs an ASTContext");
    // TODO: more!?

    clang::PrintingPolicy policy = get_preset_printing_policy(ctx, unnamed);


    // Start printing:
    string resulting_type_text = var_name ? var_name : "";

    string type_string_buffer;
    llvm::raw_string_ostream type_stream(type_string_buffer);
    AdjustedTypePrinter(policy, absolute_qualified, keep_typedef, elaborated_type_spec,
                        remove_attributes, as_parameter_signature_type, ctx)
        .print(type, type_stream, resulting_type_text);
    resulting_type_text = type_stream.str();

    return get_with_unified_types(resulting_type_text, unnamed);
  }

  // This static member-function checks whether the expression is a lvalue or
  // xvalue and if yes returns a appropriate reference-QualType while using
  // the given QualType as guideline. If not it returns a copy of the given type.
  static inline clang::QualType get_reference_type_if_necessary(const clang::QualType& type,
      const clang::Expr* const expression, const clang::ASTContext& context) {
    if(expression->isLValue() == true) {
      return context.getLValueReferenceType(type);
    }
    else if(expression->isXValue() == true) {
      return context.getRValueReferenceType(type);
    }
    else {
      return type;
    }
  }


  static const TI_Type *of (const ACM_Type &loc) {
    return static_cast<TI_Type*>(loc.transform_info ());
  }
};

class TI_Namespace : public TransformInfo {
  // pointer to the Clang namespace object (for transformation)
  clang::NamespaceDecl *_decl;
public:
  void decl (clang::NamespaceDecl *n) { _decl = n; }
  virtual clang::NamespaceDecl *decl () const { return _decl; }
};

class TI_Class : public TransformInfo {
  clang::RecordDecl *_decl;
  clang::SourceLocation _lbrace_loc;
  bool _has_replaced_arrays;

  static void get_member_contexts (const clang::RecordDecl *decl,
      list<ClangSyntacticContext> &member_contexts) {

    for (clang::RecordDecl::field_iterator i = decl->field_begin ();
        i != decl->field_end (); ++i) {

      // Only certain members are delivered.
      // If this is not the right choice for all use case, add filter flags to
      // the argument list of this function
      clang::FieldDecl *attr = *i;
      if (attr->getNameAsString ().empty ()) {
        const clang::RecordType *UT = attr->getType ()->getAsUnionType ();
        if (UT) { // members of anonymous unions in a record are also record members
          get_member_contexts (UT->getDecl (), member_contexts);
        }
        continue;
      }
//      if (attr->isStatic () || attr->isAnonymous () || attr->EnumeratorInfo ())
//        continue;

      member_contexts.push_back (ClangSyntacticContext (attr));
    }
  }

public:
  TI_Class () : _decl (0), _has_replaced_arrays( false ) {}

  bool valid () const { return _decl != 0; }

  void decl (clang::RecordDecl *c) { _decl = c; }
  virtual clang::RecordDecl *decl () const { return _decl; }

  void set_lbrace_loc(clang::SourceLocation loc) { _lbrace_loc = loc; }

  SyntacticContext get_def_context () const { return SyntacticContext (_decl); }

  void get_member_contexts (list<ClangSyntacticContext> &member_contexts) const {
    get_member_contexts (_decl, member_contexts);
  }

  enum SMKind { CONSTRUCTOR, COPY_CONSTRUCTOR, DESTRUCTOR };
  bool may_have_implicit (SMKind kind) {
    return may_have_implicit (kind, _decl);
  }

  bool may_have_implicit (SMKind kind, const clang::RecordDecl *decl) {
    const clang::CXXRecordDecl *d = llvm::cast<clang::CXXRecordDecl>(decl);
    if (!d)
      return false;
    if (kind == CONSTRUCTOR || kind == COPY_CONSTRUCTOR) {
      for (clang::CXXRecordDecl::ctor_iterator i = d->ctor_begin ();
          i != d->ctor_end (); ++i) {
        clang::CXXConstructorDecl *cd = *i;
        if (kind == CONSTRUCTOR && cd->isDefaultConstructor () &&
            cd->getAccess () == clang::AS_private)
          return false;
        if (kind == COPY_CONSTRUCTOR && cd->isCopyConstructor () &&
            cd->getAccess () == clang::AS_private)
          return false;
      }
    }
    else { // destructor
      const clang::CXXDestructorDecl *dd = d->getDestructor ();
      if (dd && kind == DESTRUCTOR && dd->getAccess () == clang::AS_private)
        return false;
    }

    for (clang::CXXRecordDecl::base_class_const_iterator i = d->bases_begin ();
        i != d->bases_end (); ++i) {
      const clang::CXXRecordDecl *bd = (*i).getType ()->getAsCXXRecordDecl ();
      if (bd && !may_have_implicit (kind, bd))
        return false;
    }

    for (clang::CXXRecordDecl::field_iterator i = d->field_begin ();
        i != d->field_end (); ++i) {
      const clang::FieldDecl *attr = *i;
      const clang::CXXRecordDecl *rd = attr->getType ()->getAsCXXRecordDecl ();
      if (rd && !may_have_implicit (kind, rd))
        return false;
    }
    return true;
  }

  // remember if planing found advice for the builtin_copy constructor
  void remember_builtin_copyconstructor_advice() {
    // if the constructor advice can be woven (parallel check to begining of CodeWeaver::gen_special_member_function)
    if( may_have_implicit( TI_Class::COPY_CONSTRUCTOR ) )
      _has_replaced_arrays = true; // remember that replacement will be done, to assure it is considerd when weaving on other joinpoints (array access)
  }
  bool has_replaced_arrays() const { return _has_replaced_arrays; }

  // return the position behind the opening bracket of the class body
  const WeavePos &body_start_pos (WeaverBase &wb) const {
    // TODO: iterating over all decls and finding the one with the smallest
    // location is a terribly comlicated solution. However, I haven't found
    // a way to get the location of the opening bracket.
#if 0
    clang::SourceLocation min_loc;
    for (clang::DeclContext::decl_iterator i = _decl->decls_begin ();
        i != _decl->decls_end (); ++i) {
      clang::SourceLocation cur_loc = (*i)->getLocStart();
      if ((*i)->isImplicit () || !cur_loc.isValid ())
        continue;
      if (!min_loc.isValid () || (cur_loc < min_loc))
          min_loc = (*i)->getLocStart();
    }
    if (min_loc.isValid ())
      return wb.weave_pos(min_loc, WeavePos::WP_BEFORE);
    else
      return wb.weave_pos(_decl->getRBraceLoc(), WeavePos::WP_BEFORE);
#endif
    assert (_lbrace_loc.isValid());
    return wb.weave_pos(_lbrace_loc.getLocWithOffset(1), WeavePos::WP_AFTER);
  }

  // return the position in front of the closing bracket of the class body
  const WeavePos &body_end_pos (WeaverBase &wb) const {
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    return wb.weave_pos(_decl->getBraceRange().getEnd(), WeavePos::WP_BEFORE);
#else
    return wb.weave_pos(_decl->getRBraceLoc(), WeavePos::WP_BEFORE);
#endif
  }

  // return the position of the first token of the class definition
  const WeavePos &objdecl_start_pos (WeaverBase &wb) const {
    return wb.weave_pos (_decl->getLocStart(), WeavePos::WP_BEFORE);
  }

  // return the position after the ";" of the class definition
  const WeavePos &objdecl_end_pos (WeaverBase &wb) const {
    // FIXME: This relies on the lack of spaces between the closing '}' and ';'.
    return wb.weave_pos (_decl->getLocEnd().getLocWithOffset(1), WeavePos::WP_AFTER);
  }

  // check whether this is a class and not a struct
  bool is_class () const { return _decl->isClass(); }

  // check whether this is a struct (more restrictive than 'is_class')
  bool is_struct () const { return _decl->isStruct(); }

  // check whether the class is defined (=has a body) and not only declared
  bool is_defined () const { return _decl->isCompleteDefinition (); }

  // checks whether this class is a template *instance*
  bool is_template_instance () const {
    const clang::CXXRecordDecl *d = llvm::cast<clang::CXXRecordDecl>(_decl);
    return d && isTemplateInstantiation (d);
  }

  // check whether the class is defined in a extern "C" block
  bool is_extern_c () const {
    return is_extern_c (_decl);
  }

  static bool is_extern_c (clang::RecordDecl *d) {
    clang::DeclContext *dc = d;
    while (dc->getDeclKind() != clang::Decl::TranslationUnit) {
      if (dc->getDeclKind() == clang::Decl::LinkageSpec)
        return clang::cast<clang::LinkageSpecDecl>(dc)->getLanguage() ==
            clang::LinkageSpecDecl::lang_c;
      dc = dc->getParent();
    }
    return false;
    // Future clang versions will support this: return _decl->isExternCContext ();
    // TODO: current version support now: return d->isExternCContext();
  }

  // checks whther the class is defined within a template instance
  bool is_in_template_instance () const { return inside_template_instance(_decl); }

  static string name(clang::RecordDecl *ci) {
    return TI_Type::get_decl_name_text(ci, TSEF_DISABLE, false, TSEF_DISABLE, false, true);
  }

  static TI_Class *of (const ACM_Class &loc) {
    return static_cast<TI_Class*>(loc.transform_info ());
  }
};

class TI_Aspect : public TI_Class {
public:

  clang::FunctionDecl *aspectof () const {

    clang::CXXRecordDecl *d = llvm::cast<clang::CXXRecordDecl>(decl());
    for (clang::CXXRecordDecl::decl_iterator di = d->decls_begin(),
                                             de = d->decls_end();
         di != de; ++di) {
      clang::NamedDecl *nd = llvm::dyn_cast<clang::NamedDecl> (*di);
      if (!nd)
        continue;
      std::string name = nd->getNameAsString();
      if (name != "aspectof" && name != "aspectOf")
        continue;
      if (nd->getKind () == clang::Decl::FunctionTemplate)
        return llvm::dyn_cast<clang::FunctionTemplateDecl>(nd)->getTemplatedDecl ();
      else if (nd->getKind () == clang::Decl::CXXMethod)
        return llvm::dyn_cast<clang::CXXMethodDecl>(nd);
    }
    return 0;
  }

  static const TI_Aspect *of (const ACM_Aspect &loc) {
    return static_cast<TI_Aspect*>(loc.transform_info ());
  }

};

class TI_Function : public TransformInfo {
  mutable vector<ClangSyntacticContext> _contexts;
  clang::FunctionDecl *_decl;
public:

  const vector<ClangSyntacticContext> &syntactic_contexts () const {
    if (_contexts.size() == 0) {
      // Store a pointer to each declaration and the definition (if one exists)
      for (clang::FunctionDecl::redecl_iterator ri = _decl->redecls_begin(),
                                                re = _decl->redecls_end();
           ri != re; ++ri)
        _contexts.push_back(ClangSyntacticContext(*ri));
    }
    return _contexts;
  }

  void decl (clang::FunctionDecl *c) { _decl = c; }
  virtual clang::FunctionDecl *decl () const { return _decl; }
  void add_decl (clang::FunctionDecl *c) {
    _contexts.push_back(ClangSyntacticContext(c));
  }

  static string name(clang::FunctionDecl *func_info) {
    string out;

    clang::CXXConversionDecl *conv =
        llvm::dyn_cast_or_null<clang::CXXConversionDecl>(func_info);
    if (conv) {
      clang::ASTContext &ctx = func_info->getASTContext();
      clang::QualType t = conv->getConversionType ();
      out = "operator ";
      out += TI_Type::get_type_sig_text(t, &ctx, 0, true);
      return out;
    }

    out = func_info->getNameAsString();
    // Add a space after "operator" for Puma compatibility.
    if (out.size() > 8 && llvm::StringRef(out).startswith("operator") &&
        out[8] != ' ' && out[8] != '_' && !isalnum(out[8]))
      out.insert(out.begin() + 8, ' ');

    clang::FunctionTemplateSpecializationInfo *ftsi =
        func_info->getTemplateSpecializationInfo ();
    if (ftsi) {
      if (out[out.length() - 1] == '<') // e.g. operator <<
        out += " ";
      out += TI_Type::get_templ_arg_list_text(*ftsi->TemplateArguments,
                                              &func_info->getASTContext(),
                                              TSEF_DONOTCHANGE,
                                              false,
                                              TSEF_DISABLE,
                                              false,
                                              true);
    }
    return out;
  }

  static string signature(clang::FunctionDecl *func_info) {
    std::string str;
    llvm::raw_string_ostream out(str);
    clang::ASTContext &ctx = func_info->getASTContext();

    out << '(';
    for (clang::FunctionDecl::param_iterator i = func_info->param_begin(),
                                             e = func_info->param_end();
         i != e; ++i) {
      if (i != func_info->param_begin())
        out << ',';
      out << TI_Type::get_type_sig_text((*i)->getType(), &ctx, 0, true);
    }
    out << ')';

    // add CV qualifiers
    if (clang::CXXMethodDecl *m =
            llvm::dyn_cast<clang::CXXMethodDecl>(func_info)) {
      if (m->isConst())
        out << " const";
      if (m->isVolatile())
        out << " volatile";
    }

    return name(func_info) + out.str ();
  }

  bool is_const () const {
    clang::CXXMethodDecl *m = llvm::dyn_cast<clang::CXXMethodDecl>(_decl);
    return m && m->isConst ();
  }

  bool is_conversion_operator () const {
    return (llvm::dyn_cast_or_null<clang::CXXConversionDecl>(_decl) != 0);
  }

  static const TI_Function *of (const ACM_Function &loc) {
    return static_cast<TI_Function*>(loc.transform_info ());
  }
};

class TI_Variable : public TransformInfo {
  clang::DeclaratorDecl *_decl;
public:
  TI_Variable () : _decl (0) {}

  void decl (clang::DeclaratorDecl *oi) { _decl = oi; }
  virtual clang::DeclaratorDecl *decl () const { return _decl; }
public:
  static const TI_Variable *of (const ACM_Variable &loc) {
    return static_cast<TI_Variable*>(loc.transform_info ());
  }
};


class TI_Arg : public TransformInfo {
  clang::QualType _type;
public:
  void type (clang::QualType ti) { _type = ti; }
  clang::QualType type () const { return _type; }

  virtual clang::Decl *decl () const { return 0; }

  static const TI_Arg *of (const ACM_Arg &loc) {
    return static_cast<TI_Arg*>(loc.transform_info ());
  }
};

class TI_Code : public TransformInfo {
  bool _is_planned;
  bool _has_implicit;
  CFlowList _triggers;
public:
  TI_Code() : _is_planned( false ), _has_implicit( false ) {};

  // remember if this joinpoint is planned for weaving
  // ( decisions for other joinpoints depend on this info )
  void remember_planned() { _is_planned = true; }
  bool is_planned() { return _is_planned; }

  // remember implicit joinpoints needing to be considered
  void remember_implicit() { _has_implicit = true; }
  bool has_implicit_joinpoints() { return _has_implicit; }

  // consider a necessary cflow trigger at this join point
  bool consider (const CFlow &cflow) {
    _triggers.push_back (cflow);
    return true;
  }

  // return the list of cflows that must be entered/left here
  const CFlowList &cflows () const { return _triggers; }

  // that types (for the JoinPoint-API)
  virtual std::string that_type_string () const { return "void"; }

  // target type (for the JoinPoint-API)
  virtual std::string target_type_string () const { return "void"; }

  // argument type (for the JoinPoint-API)
  virtual std::string arg_type_string (unsigned no) const { return "void"; }

  // entity type (for the JoinPoint-API)
  virtual std::string entity_type_string() const {
    return format_type( decl() );
  }

  // helper functions for derived classes
  static std::string get_type_string (const clang::NamedDecl *obj, bool normalise = false) {

    // if the 'obj' refers to a record or member function, we have to print a record 'r'
    const clang::CXXRecordDecl *r = llvm::dyn_cast<const clang::CXXRecordDecl>(obj);
    const clang::CXXMethodDecl *m = llvm::dyn_cast<const clang::CXXMethodDecl>(obj);
    if (m)
      r = m->getParent ();

    if (!r)
      if (const clang::ParmVarDecl *pd = llvm::dyn_cast<const clang::ParmVarDecl>(obj))
        r = pd->getType()->getAsCXXRecordDecl();

    if (r) {
      string qualifiers;
      if (m) {
        // preserve const volatile qualifiers of member functions
        // we could also print m->getThisType(obj->getASTContext())
        if(m->isConst())
          qualifiers = "const ";
        if (m->isVolatile())
          qualifiers = "volatile ";
      }
      return qualifiers + TI_Type::get_decl_name_text(r, TSEF_ENABLE, true, TSEF_DONOTCHANGE, false, true);
    }
    else if (const clang::ParmVarDecl *pd = llvm::dyn_cast<const clang::ParmVarDecl>(obj)) {
      return TI_Type::get_type_code_text(pd->getType(), &obj->getASTContext(), 0);
    }
    else
      return "void";
  }

  static std::string format_type( clang::Decl *obj ) {
    clang::ValueDecl *typed_obj = llvm::dyn_cast_or_null<clang::ValueDecl>( obj );
    if( ! typed_obj )
      return "void";

    return format_type( typed_obj->getType(), obj->getASTContext() );
  }

  static std::string format_type( clang::QualType type, clang::ASTContext& ctx ) {
    return TI_Type::get_type_code_text(type, &ctx, 0);
  }

  static TI_Code *of (const ACM_Code &loc) {
    return static_cast<TI_Code*>(loc.transform_info ());
  }
};

class TI_Method : public TI_Code {
  clang::FunctionDecl *_decl;

public:
  TI_Method () : _decl (0) {}

  void decl (clang::FunctionDecl *f) { _decl = f; }
  virtual clang::Decl *decl () const { return _decl; }

  // that type (for the JoinPoint-API)
  virtual std::string that_type_string() const {
    return get_type_string(_decl);
  }

  // target type (for the JoinPoint-API)
  virtual std::string target_type_string() const {
    return get_type_string(_decl);
  }

  virtual std::string arg_type_string (unsigned no) const {
    return get_type_string (_decl->getParamDecl (no), true);
  }
};

// forward declaring child class, as we need to reference the pointer type already
// class TI_Builtin : public TI_Access
class TI_Builtin;

class TI_Access : public TI_Code {
private:
  clang::DeclaratorDecl *_entity;

  clang::Expr *_node;
  clang::Expr *_ref_node;

  clang::Decl *_origin;

  clang::Expr *_target_expr;
public:
  TI_Access() : _entity(0), _node(0), _ref_node(0), _origin(0), _target_expr(0) {}

protected: // make setter protected as derived class might need one with different signature
  void entity( clang::DeclaratorDecl *ent ) { _entity = ent; }
public:
  clang::DeclaratorDecl *entity() const { return _entity; }
  virtual clang::Decl *decl() const { return _entity; } // defined in TransformInfo

  bool entity_is_const() const { return false; }

protected: // make setter protected as derived class might need one with different signature
  void tree_node( clang::Expr *n ) { _node = n; };
public:
  clang::Expr *tree_node() const { return _node; };

  void ref_node( clang::Expr *ref ) {
    _ref_node = ref;

    _target_expr = find_target_expr(); // init caching var
  };
  clang::Expr *ref_node() const { return _ref_node; };

  // checks if the original access uses a qualified target entity name
  bool is_qualified () const {
    clang::MemberExpr *me = clang::dyn_cast_or_null<clang::MemberExpr>( _ref_node );
    if (me && me->hasQualifier())
      return true;
    clang::DeclRefExpr *dre = clang::dyn_cast_or_null<clang::DeclRefExpr>( _ref_node );
    if (dre && dre->hasQualifier())
      return true;
    return false;
  }

  void origin( clang::Decl *o ) { _origin = o; }
  clang::Decl *origin() const { return _origin; }

  bool target_is_ptr() const { return _target_expr && _target_expr->getType().getTypePtr()->isPointerType(); }
  bool target_is_implicit() const { return _target_expr && _target_expr->isImplicitCXXThis(); }
  bool target_is_const() const {
    if (!_target_expr)
      return false;
    // in case of calls to const member functions Clang adds an implicit cast that turn the object
    // into a const object even if it was non-const. So we have to ignore implicit casts here.
    clang::Expr *real_target = _target_expr->IgnoreImpCasts ();
    // now check whether the object type was const before any implicit casts
    clang::QualType t = real_target->getType();
    // if the target is a pointer we need check the object's const-ness and ignore the pointer.
    if (t.getTypePtr()->isPointerType())
      t = t.getTypePtr()->getPointeeType();
    return t.isConstQualified ();
  }
  bool has_target_expr() const { return _target_expr && ! target_is_implicit(); }
  clang::Expr *target_expr() const {
    if( ! target_is_implicit() ) // by convention only explicit expr are returned
      return _target_expr;
    else
      return 0;
  }
  // no setter as it is done internally
  virtual clang::Expr *find_target_expr() const {
    // analog to clang::CXXMemberCallExpr::getImplicitObjectArgument()
    if( const clang::MemberExpr *member = llvm::dyn_cast<clang::MemberExpr>( _ref_node ) )
      return member->getBase()->IgnoreImpCasts();
    else if( const clang::BinaryOperator *op = llvm::dyn_cast<clang::BinaryOperator>( _ref_node ) )
      if( op->getOpcode() == clang::BO_PtrMemD || op->getOpcode() == clang::BO_PtrMemI )
        return op->getLHS();

    return 0;
  }

  // target type (for the JoinPoint-API)
  virtual const clang::RecordDecl *target_class () const = 0;
  // type of target expr (for qualifing names, might differ from target_class)
  virtual const clang::RecordDecl *targetexpr_class() const = 0;
  const clang::RecordDecl *defining_class() const { return clang::dyn_cast<clang::RecordDecl>( _entity->getDeclContext() ); }

  const bool entity_from_baseclass() const {
    const clang::CXXRecordDecl *defining = llvm::dyn_cast_or_null<clang::CXXRecordDecl>( defining_class() );
    const clang::CXXRecordDecl *target = llvm::dyn_cast_or_null<clang::CXXRecordDecl>( target_class() );
    return defining && target && target->isDerivedFrom( defining );
  }

  static TI_Access *of( const ACM_Access &loc ) {
    return static_cast<TI_Access *>( loc.transform_info() );
  }

  const SyntacticContext access_context () const { return SyntacticContext( _origin ); }
  const SyntacticContext entity_context () const { return SyntacticContext( _entity ); }

  // that type (for the JoinPoint-API)
  virtual std::string that_type_string() const {
    if( clang::CXXMethodDecl *m = llvm::dyn_cast<clang::CXXMethodDecl>( _origin ) )
      return get_type_string( m->getParent() );
    else if( clang::VarDecl *vd = llvm::dyn_cast<clang::VarDecl>( _origin ) )
      if( clang::CXXRecordDecl *r = llvm::dyn_cast<clang::CXXRecordDecl>( vd->getDeclContext() ) )
        return get_type_string( r );
    return "void";
  }

  // target type (for the JoinPoint-API)
  virtual std::string target_type_string() const {
    const clang::NamedDecl *tc = target_class();
    return ( tc ? get_type_string( tc ) : string( "void" ));
  }

  // type of target expr (for qualifing names, might differ from target_class)
  virtual std::string targetexpr_type_string() const {
    const clang::NamedDecl *tc = targetexpr_class();
    return ( tc ? get_type_string( tc ) : string( "void" ));
  }

  virtual bool has_result() const = 0;
  virtual clang::QualType result_type() const = 0;
  virtual std::string result_type_string() const { return format_type( result_type(), _origin->getASTContext() ); }

  virtual unsigned int entity_index_count() const { return 0; } // derived override if they have indices
  virtual unsigned long int entity_index_dimension( unsigned int i ) const { return 0; } // derived override if they have indices
  virtual std::string entity_index_type( unsigned int i ) const { return "void"; } // derived override if they have indices
  virtual TI_Builtin *entity_src() const { return 0; }
  enum SpliceMode { SpliceMode_None, SpliceMode_Packed, SpliceMode_Pack, SpliceMode_FullSplice };
  virtual SpliceMode entity_src_splice_mode() const { return SpliceMode_None; }

  static const clang::Expr * skipTransparent( const clang::Expr * start ) {
    const clang::Expr *oldE = start;
    const clang::Expr *newE = start->IgnoreImplicit()->IgnoreParens();
    while( oldE != newE ) {
      oldE = newE;
      if( const clang::BinaryOperator *bo = llvm::dyn_cast<clang::BinaryOperator>( newE ) ) {
        if( bo->getOpcode() == clang::BO_Comma ) // jump inside right part as this is the relevant part for the joinpoint in the parent
          newE = bo->getRHS();
      }
      newE = newE->IgnoreImplicit()->IgnoreParens();
    }
    return newE;
  }

  virtual const WeavePos &before_pos (WeaverBase &wb) {
    // jump into parenthesis and ignore exprs that don't change anything
    // cant do this early (or _node in general as ImplicitCast can be skipped here but not for type compuatations)
    return wb.weave_pos( skipTransparent( _node )->getLocStart(), WeavePos::WP_BEFORE );
  }
  virtual const WeavePos &after_pos (WeaverBase &wb) {
    // jump into parenthesis and ignore exprs that don't change anything
    // cant do this early (or _node in general as ImplicitCast can be skipped here but not for type compuatations)
    return get_pos_after_token( skipTransparent( _node )->getLocEnd(), wb );
  }
  const WeavePos &entity_before_pos( WeaverBase &wb ) {
    return wb.weave_pos( ref_node()->getLocStart(), WeavePos::WP_BEFORE );
  }
  const WeavePos &entity_after_pos( WeaverBase &wb ) {
    return get_pos_after_token( ref_node()->getLocEnd(), wb, WeavePos::WP_AFTER );
  }
  const WeavePos &entity_op_before_pos( WeaverBase &wb ) {
    assert( clang::isa<clang::MemberExpr>( ref_node() ) );
    clang::MemberExpr *me = clang::dyn_cast<clang::MemberExpr>( ref_node() );
    // clang3.4 seems to miss getOperatorLoc()
    //return wb.weave_pos( me->getOperatorLoc(), WeavePos::WP_BEFORE );
    return get_pos_after_token( me->getBase()->getLocEnd(), wb, WeavePos::WP_BEFORE );
  }
  const WeavePos &entity_op_after_pos( WeaverBase &wb ) {
    assert( clang::isa<clang::MemberExpr>( ref_node() ) );
    clang::MemberExpr *me = clang::dyn_cast<clang::MemberExpr>( ref_node() );
    if( me->hasQualifier() )
      return wb.weave_pos( me->getQualifierLoc().getBeginLoc(), WeavePos::WP_AFTER );
    else
      return wb.weave_pos( me->getMemberLoc(), WeavePos::WP_AFTER );
  }

  struct PH : public clang::PrinterHelper {
    virtual bool  handledStmt (clang::Stmt *node, llvm::raw_ostream &os) {
      clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> (node);
      clang::FunctionDecl *fd = (ce ? ce->getDirectCallee () : 0);
      if (ce && ce->getNumArgs () == 1 &&
          fd->getNameAsString () == "operator->") {
        clang::ASTContext &ctx = fd->getASTContext();
        os << "(";
        ce->getArg (0)->printPretty(os, this, ctx.getPrintingPolicy(), 0);
        os << ").operator->()";
        return true;
      }
      // WORKAROUND for Clang 3.4 problem:
      // Implicit calls to conversion operators are printed explicitly, but
      // the object argument is not put into brackets. For example:
      // "a & b" might become "a & b.operator int()"
      // This fix generates the brackets: "(a & b).operator int()"
      clang::CXXMemberCallExpr *mce = clang::dyn_cast<clang::CXXMemberCallExpr> (node);
      fd = (mce ? mce->getDirectCallee () : 0);
      if (fd && clang::dyn_cast<clang::CXXConversionDecl>(fd)) {
        clang::MemberExpr *me = clang::dyn_cast<clang::MemberExpr>(mce->getCallee ());
        if (me) {
          os << "(";
          clang::ASTContext &ctx = fd->getASTContext();
          me->getBase ()->printPretty(os, this, ctx.getPrintingPolicy(), 0);
          os << ")" << (me->isArrow () ? "->" : ".");
          os << TI_Function::name(fd) << "()";
          return true;
        }
      }
      return false;
    }
  };

  virtual string code () const {
    PH ph;
    clang::ASTContext &ctx = _origin->getASTContext();
    std::string buf;
    llvm::raw_string_ostream out (buf);
    _node->printPretty(out, &ph, ctx.getPrintingPolicy(), 0);
    return buf;
  }

  // returns true if the access needs special rights
  bool needs_rights () const {
    // no member function => no accessibility problem
    if( ! target_class() )
      return false;

    // static member => no problem only if public
    clang::CXXMethodDecl *md = clang::dyn_cast<clang::CXXMethodDecl>(_entity);
    clang::VarDecl *vd = clang::dyn_cast<clang::VarDecl>(_entity);
    if ((md && md->isStatic ()) || (vd && vd->isStaticDataMember ()))
      return (_entity->getAccess () != clang::AS_public);

    // normal member function => look up the accessibility
    const clang::CXXRecordDecl *base = clang::dyn_cast<clang::CXXRecordDecl>( defining_class() );
    const clang::CXXRecordDecl *target = clang::dyn_cast<clang::CXXRecordDecl>(target_class ());
    clang::CXXBasePaths paths;
    if (target->isDerivedFrom (base, paths)) {
      for (clang::CXXBasePaths::const_paths_iterator i = paths.begin (); i != paths.end (); ++i)
        if (i->Access != clang::AS_public)
          return true;
    }

    if( _entity->getAccess () == clang::AS_public )
      return false;

    return true;
  }
};

// This abstract class represents a call in general and exists just as interface for
// actions that are (mostly) the same between calls of user-defined (member-)functions
// and calls of built-in operators.
class TI_CommonCall : public TI_Access {
protected:
  TI_CommonCall () {}
public:
  virtual string operator_kind_string() const = 0;

  virtual const clang::Expr* arg(unsigned int index) const = 0;
  virtual unsigned int arg_count() const = 0;

  virtual bool is_unary_expr() const = 0;
  virtual bool is_binary_expr() const = 0;
  virtual bool is_ternary_expr() const = 0;
  virtual bool is_postfix_expr() const = 0;
  virtual bool is_index_expr() const = 0;
  virtual bool is_arrow_class_member_access_expr() const = 0;
  virtual bool is_implicit_conversion() const = 0;
  virtual bool is_implicit_dereference() const = 0;

  virtual clang::SourceLocation get_operator_location() const = 0;

  // returns the position directly in front of the operator
  virtual const WeavePos &op_before_pos (WeaverBase &wb) {
    return wb.weave_pos(get_operator_location(), WeavePos::WP_BEFORE);
  }

  // returns the position directly behind the operator
  virtual const WeavePos &op_after_pos (WeaverBase &wb) {
    return get_pos_after_token(get_operator_location(), wb);
  }

  // The following member-functions may be called if is_index_expr() is true
  // returns the position directly in front of the opening bracket [
  virtual const WeavePos &index_open_before_pos (WeaverBase &wb) {
    assert (is_index_expr ());
    return get_pos_after_token(arg(0)->getLocEnd(), wb, WeavePos::WP_BEFORE);
  }
  // returns the position directly behind the opening bracket [
  virtual const WeavePos &index_open_after_pos (WeaverBase &wb) {
    assert (is_index_expr ());
    return wb.weave_pos(arg(1)->getLocStart (), WeavePos::WP_AFTER);
  }
  // returns the position directly in front of the closing bracket ]
  virtual const WeavePos &index_close_before_pos (WeaverBase &wb) {
    assert (is_index_expr ());
    return get_pos_after_token(arg(1)->getLocEnd (), wb, WeavePos::WP_BEFORE);
  }
  // returns the position directly behind the closing bracket ]
  virtual const WeavePos &index_close_after_pos (WeaverBase &wb) {
    assert (is_index_expr ());
    return get_pos_after_token(TI_Access::tree_node()->getLocEnd (), wb, WeavePos::WP_AFTER);
  }

  // The following member-functions return the corresponding weave-positions of ternary operators
  // (first delimiter = "?", second delimiter = ":")
  virtual const WeavePos& ternary_op_first_delim_before_pos(WeaverBase &wb) {
    assert (is_ternary_expr());
    return get_pos_after_token(arg(0)->getLocEnd(), wb, WeavePos::WP_BEFORE);
  }
  virtual const WeavePos& ternary_op_first_delim_after_pos(WeaverBase &wb) {
    assert (is_ternary_expr());
    return wb.weave_pos(arg(1)->getLocStart(), WeavePos::WP_AFTER);
  }
  virtual const WeavePos& ternary_op_second_delim_before_pos(WeaverBase &wb) {
    assert (is_ternary_expr());
    return get_pos_after_token(arg(1)->getLocEnd(), wb, WeavePos::WP_BEFORE);
  }
  virtual const WeavePos& ternary_op_second_delim_after_pos(WeaverBase &wb) {
    assert (is_ternary_expr());
    return wb.weave_pos(arg(2)->getLocStart(), WeavePos::WP_AFTER);
  }
};

class TI_MethodCall : public TI_CommonCall {
  clang::FunctionDecl *_called_func;
public:
  TI_MethodCall() : _called_func( 0 ) {}

  void called (clang::FunctionDecl *f) { _called_func = f; entity( f ); }
  clang::FunctionDecl *called () const { return _called_func; }

  void tree_node( clang::Expr *n ) {
    // In this class the node-object always has the type clang::CallExpr*
    assert(!n || llvm::isa<clang::CallExpr>(n));
    TI_Access::tree_node( n );
    if( n ) {
      ref_node( static_cast<clang::CallExpr*>(n)->getCallee()->IgnoreParenImpCasts() );
    }
  }

  // In this class the node-object always has the type clang::CallExpr*
  const clang::CallExpr* tree_node() const {
    assert(llvm::isa<clang::CallExpr>(TI_Access::tree_node()));
    return static_cast<clang::CallExpr*>(TI_Access::tree_node());
  }

  static TI_MethodCall *of (const ACM_Call &loc) {
    return static_cast<TI_MethodCall*>(loc.transform_info ());
  }

  bool uses_ADN_lookup () const {
    return (_called_func &&
        _called_func->getLexicalDeclContext () != _called_func->getDeclContext () &&
        _called_func->getFriendObjectKind () != clang::Decl::FOK_None);
  }

  virtual string operator_kind_string() const {
    if( clang::CXXOperatorCallExpr *oc =llvm::dyn_cast<clang::CXXOperatorCallExpr>( TI_Access::tree_node() ) )
      return oc->getDirectCallee ()->getNameAsString ().substr (8);

    assert( false && "Unknown expr type" );
    return "<?>"; // dummy
  }

  static unsigned int arg_count( const clang::CallExpr *node ) {
    unsigned int args = node->getNumArgs();
    // for calls to % T::operator() (...) on an object t of type T,
    // e.g t(42), Clang says that the number of args is 2. In AspectC++ we
    // regard t is the target pointer and only 42 as an argument.
    if (is_call_op(node))
      args--;
    return args;
  }

  virtual unsigned int arg_count() const {
    return arg_count( tree_node() );
  }

  // This method takes an argument-index as unsigned int and returns the correspondent argument-clang::Expr-pointer.
  static const clang::Expr* arg( const clang::CallExpr *node, unsigned int index ) {
    // for calls to % T::operator() (...) on an object t of type T,
    // e.g t(42), Clang says that the number of args is 2. In AspectC++ we
    // regard t is the target pointer and only 42 as an argument.
    if (is_call_op(node))
      index++;
    return node->getArg(index);
  }

  // This method takes an argument-index as unsigned int and returns the correspondent argument-clang::Expr-pointer.
  virtual const clang::Expr* arg(unsigned int index) const {
    return arg( tree_node(), index );
  }

  static std::string arg_type_string( const clang::CallExpr *node, clang::ASTContext &ctx, unsigned no ) {
    return TI_Type::get_type_code_text(arg( node, no )->getType(), &ctx, 0);
  }

  virtual std::string arg_type_string (unsigned no) const {
    // If the requested argument is no variadic argument we take it
    // from the function declaration:
    if( no < _called_func->getNumParams () )
      return get_type_string (_called_func->getParamDecl (no), true);

    assert(_called_func->isVariadic());
    // Otherwise we take it from the call expression:
    clang::ASTContext &ctx = origin()->getASTContext();
    return arg_type_string( tree_node(), ctx, no );
  }

  bool is_operator_call() const {
    return llvm::isa<clang::CXXOperatorCallExpr>( TI_Access::tree_node() );
  }

  virtual bool is_unary_expr () const {
    clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> (TI_Access::tree_node());
    return (ce && ce->getNumArgs () == 1 && !is_call_op());
  }

  virtual bool is_binary_expr () const {
    clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> (TI_Access::tree_node());
    return (ce && ce->getNumArgs () == 2 && !is_index_expr () && !is_postfix_expr () && !is_call_op());
  }

  virtual bool is_index_expr () const {
    clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> (TI_Access::tree_node());
    return (ce && ce->getNumArgs () == 2 && _called_func->getNameAsString () == "operator[]");
  }

  virtual bool is_postfix_expr () const {
    clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> (TI_Access::tree_node());
    return (ce && ce->getNumArgs () == 2 &&
        (_called_func->getNameAsString () == "operator++" ||
            _called_func->getNameAsString () == "operator--"));
  }

  virtual bool is_arrow_class_member_access_expr() const {
    return _called_func->getNameAsString () == "operator->";
  }

  virtual bool is_ternary_expr() const {
    return false;
  }

  virtual bool is_implicit_conversion () const {
    if (!llvm::isa<clang::CXXConversionDecl> (_called_func))
      return false;
    clang::CXXMemberCallExpr *mce = clang::dyn_cast<clang::CXXMemberCallExpr>( TI_Access::tree_node() );
    if (!mce)
      return false;
    // TODO: is there a better way to distinguish 'c' from 'c.operator int*()'?
    return (mce->getCallee ()->getLocEnd () == TI_Access::tree_node()->getLocEnd ());
  }
  
  virtual bool is_implicit_dereference() const {
    return false;
  }

  static bool is_call_op (const clang::CallExpr *node) {
    return (clang::dyn_cast<clang::CXXOperatorCallExpr>(node) &&
        node->getDirectCallee ()->getNameAsString () == "operator()");
  }

  bool is_call_op () const {
    return is_call_op (tree_node());
  }

  // This method returns the clang::SourceLocation of the operator according to the operator-type.
  virtual clang::SourceLocation get_operator_location() const {
    if( clang::CXXOperatorCallExpr *ce = clang::dyn_cast<clang::CXXOperatorCallExpr> ( TI_Access::tree_node() ) )
      return ce->getOperatorLoc ();

    assert( false && "This is no operator." );
    return TI_Access::tree_node()->getLocStart(); // dummy
  }

  const WeavePos &args_open_before_pos (WeaverBase &wb) {
    if (is_call_op ())
      return wb.weave_pos (tree_node()->getCallee ()->getLocStart (), WeavePos::WP_BEFORE);
    else
      return get_pos_after_token(tree_node()->getCallee ()->getLocEnd (), wb, WeavePos::WP_BEFORE);
  }

  const WeavePos &args_open_after_pos (WeaverBase &wb) {
    if (call_args () > 0)
      return wb.weave_pos (arg (0)->getLocStart (), WeavePos::WP_AFTER);
    else
      return wb.weave_pos (TI_Access::tree_node()->getLocEnd (), WeavePos::WP_AFTER);
  }

  const WeavePos &args_close_before_pos (WeaverBase &wb) {
    return wb.weave_pos(TI_Access::tree_node()->getLocEnd (), WeavePos::WP_BEFORE);
  }

  const WeavePos &args_close_after_pos (WeaverBase &wb) {
    return get_pos_after_token(TI_Access::tree_node()->getLocEnd (), wb, WeavePos::WP_AFTER);
  }

  const WeavePos &callee_before_pos (WeaverBase &wb) {
    if (has_target_expr()) {
      const clang::Expr *callee = tree_node()->getCallee ();
      while (clang::dyn_cast<clang::ImplicitCastExpr> (callee))
        callee = clang::dyn_cast<clang::ImplicitCastExpr> (callee)->getSubExpr ();
      while (clang::dyn_cast<clang::ParenExpr> (callee))
        callee = clang::dyn_cast<clang::ParenExpr> (callee)->getSubExpr ();
      const clang::MemberExpr *me = clang::dyn_cast<clang::MemberExpr> (callee);
      assert (me);
      return get_pos_after_token(me->getBase ()->getLocEnd (), wb, WeavePos::WP_BEFORE);
    }
    else {
      if (is_call_op ())
        return wb.weave_pos (TI_Access::tree_node()->getLocStart (), WeavePos::WP_BEFORE);
      else
        return wb.weave_pos (tree_node()->getCallee ()->getLocStart (), WeavePos::WP_BEFORE);
    }
  }

  const WeavePos &callee_after_pos (WeaverBase &wb) {
    if (is_call_op ())
      return wb.weave_pos (tree_node()->getCallee ()->getLocStart (), WeavePos::WP_AFTER);
    else {
      const clang::Expr *callee = tree_node()->getCallee ();
      while (clang::dyn_cast<clang::ParenExpr> (callee))
        callee = clang::dyn_cast<clang::ParenExpr> (callee)->getSubExpr ();
      return get_pos_after_token(callee->getLocEnd (), wb, WeavePos::WP_AFTER);
    }
  }

  // return the number of arguments, not including the object in case of
  // member function calls and not including implicitly passed default arguments
  unsigned call_args () const {
    unsigned args = 0;
    const clang::CallExpr* call_node = tree_node();
    while (args < call_node->getNumArgs ()) {
      if( clang::dyn_cast<clang::CXXDefaultArgExpr>( call_node->getArg( args ) ) )
        break;
      args++;
    }
    // in case of calls to operator() the number has to be decremented once
    if (is_call_op())
      args--;
    return args;
  }

  virtual bool has_result () const {
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    return !_called_func->getResultType()->isVoidType ();
#else // C++ 11 interface
    return !_called_func->getReturnType()->isVoidType ();
#endif
  }

  virtual clang::QualType result_type() const {
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
    return _called_func->getResultType ();
#else // C++ 11 interface
    return _called_func->getReturnType ();
#endif
  }

  // target type (for the JoinPoint-API)
  virtual const clang::RecordDecl *target_class () const {
    return targetexpr_class();
  }

  virtual const clang::RecordDecl *targetexpr_class () const {
    const clang::RecordDecl *result = 0;

    const clang::CXXMethodDecl *md = clang::dyn_cast_or_null<clang::CXXMethodDecl> (_called_func);
    if( has_target_expr() ) {
      const clang::Type *type = target_expr()->getType ().getTypePtr ();
      result = type->getPointeeCXXRecordDecl();
      if (!result)
        result = type->getAsCXXRecordDecl ();
    }
    else if (md) {
      result = md->getParent();
      if (clang::CXXMethodDecl *caller = llvm::dyn_cast<clang::CXXMethodDecl>(origin())) {
        if (!md->isStatic())
          result = caller->getParent ();
      }
    }
    return result;
  }

  // the target object of the call or NULL
  virtual clang::Expr *find_target_expr() const {

    // check if this call has a target object
    clang::CXXMethodDecl *md = clang::dyn_cast_or_null<clang::CXXMethodDecl> (_called_func);
    if (!md /* || md->isStatic ()*/)
      return 0;

    clang::Expr *result = 0;

    // an ordinary member function call, e.g. foo->bar()
    clang::CXXMemberCallExpr *mce =
        clang::dyn_cast<clang::CXXMemberCallExpr> (TI_Access::tree_node());
    // .. or an operator call, e.g. !foo or foo+bar
    clang::CXXOperatorCallExpr *oce =
        clang::dyn_cast<clang::CXXOperatorCallExpr> (TI_Access::tree_node());
    if (mce)
      result = mce->getImplicitObjectArgument ();
    else if (oce && md->getParent ())
      result = oce->getArg (0);
    else {
      // it might still be a static member function call with unused target expr,
      // e.g. foo->static_bar()
      const clang::Expr *callee = tree_node()->getCallee();
      if (clang::dyn_cast<clang::ImplicitCastExpr> (callee)) {
        callee = clang::dyn_cast<clang::ImplicitCastExpr> (callee)->getSubExpr ();
        if (clang::dyn_cast<clang::MemberExpr> (callee)) {
          result = clang::dyn_cast<clang::MemberExpr> (callee)->getBase();
        }
      }
    }

    // TODO: check if implicit calls are handled correctly here
    return result;
  }

  // checks whether the call uses explicit template parameters
  bool has_explicit_template_params () const {
    if( clang::DeclRefExpr *dre = llvm::dyn_cast<clang::DeclRefExpr>( ref_node() ) )
      return dre->hasExplicitTemplateArgs();
    else if( clang::MemberExpr *me = llvm::dyn_cast<clang::MemberExpr>( ref_node() ) )
      return me->hasExplicitTemplateArgs();
    else
      return false;
  }

  const clang::TemplateArgumentLoc *get_explicit_template_params() const {
    if( clang::DeclRefExpr *dre = llvm::dyn_cast<clang::DeclRefExpr>( ref_node() ) )
      return dre->getTemplateArgs();
    else if( clang::MemberExpr *me = llvm::dyn_cast<clang::MemberExpr>( ref_node() ) )
      return me->getTemplateArgs();
    else
      return 0;
  }

  unsigned int num_explicit_template_params() const {
    if( clang::DeclRefExpr *dre = llvm::dyn_cast<clang::DeclRefExpr>( ref_node() ) )
      return dre->getNumTemplateArgs();
    else if( clang::MemberExpr *me = llvm::dyn_cast<clang::MemberExpr>( ref_node() ) )
      return me->getNumTemplateArgs();
    else
      return 0;
  }
};

// This class represents a call of a built-in operator.
class TI_Builtin : public TI_CommonCall {
  TI_Builtin *_forwarded_src;
  ACM_Access *_packed_forward_requester;
public:
  TI_Builtin() : _forwarded_src( 0 ), _packed_forward_requester( 0 ) {}

  void tree_node( clang::Expr *n ) {
    assert(is_builtin_operator(n));
    TI_Access::tree_node( n );
  }

  virtual string code () const {
    if( TI_Builtin::is_implicit_dereference() ) {
      clang::ASTContext &ctx = TI_Access::origin()->getASTContext();
      std::string buf("*"); // add explicit built-in operator*
      llvm::raw_string_ostream out (buf);
      arg(0)->printPretty(out, 0, ctx.getPrintingPolicy(), 0);
      out.flush();
      return buf;
    }
    return TI_CommonCall::code();
  }
  
  virtual const WeavePos &after_pos (WeaverBase &wb) {
    if( TI_Builtin::is_implicit_dereference() ) {
      return get_pos_after_token( skipTransparent( arg(0) )->getLocEnd(), wb );
    }
    return TI_CommonCall::after_pos(wb);
  }

  virtual const WeavePos &before_pos (WeaverBase &wb) {
    if( TI_Builtin::is_implicit_dereference() ) {
      return wb.weave_pos( skipTransparent( arg(0) )->getLocStart(), WeavePos::WP_BEFORE );
    }
    return TI_CommonCall::before_pos(wb);  
  }

  static TI_Builtin *of (const ACM_Builtin &loc) {
    return static_cast<TI_Builtin*>(loc.transform_info ());
  }

  static bool is_builtin_operator(clang::Expr* node) {
    return llvm::isa<clang::UnaryOperator>( node )
        || llvm::isa<clang::BinaryOperator>( node )
        || llvm::isa<clang::ArraySubscriptExpr>( node )
        || llvm::isa<clang::ConditionalOperator>( node )
        || is_implicit_dereference( node );
  }

  static string operator_kind_string( clang::Expr *node ) {
    assert( is_builtin_operator( node ) );
    if( clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( node ) )
      return clang::UnaryOperator::getOpcodeStr( uo->getOpcode() ).str();
    else if( clang::BinaryOperator * bo = llvm::dyn_cast<clang::BinaryOperator>( node ) )
      return clang::BinaryOperator::getOpcodeStr( bo->getOpcode() ).str();
    else if( llvm::isa<clang::ArraySubscriptExpr>( node ) )
      return "[]";
    else if( llvm::isa<clang::ConditionalOperator>( node ) )
      return "?:";
    else if( is_implicit_dereference( node ) )
      return "*";

    assert( false && "Unknown expr type" );
    return "<?>"; // dummy
  }

  virtual string operator_kind_string() const {
    return( operator_kind_string( TI_Access::tree_node() ) );
  }

  static unsigned int arg_count( const clang::Expr *node ) {
    if( const clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( node ) ) {
      if( uo->isPostfix() )
        return 2;
      else
        return 1;
    }
    else if( llvm::isa<clang::BinaryOperator>( node ) )
      return 2;
    else if( llvm::isa<clang::ArraySubscriptExpr>( node ) )
      return 2;
    else if( llvm::isa<clang::ConditionalOperator>( node ) )
      return 3;
    else if( is_implicit_dereference( node ) )
      return 1;

    assert( false && "Unknown expr type" );
    return 0; // dummy
  }

  virtual unsigned int arg_count() const {
    return arg_count( TI_Access::tree_node() );
  }

  // This method takes an argument-index as unsigned int and returns the correspondent argument-clang::Expr-pointer.
  static const clang::Expr* arg( const clang::Expr *node, unsigned int index ) {
    // Valid index?
    assert( index < arg_count( node ) );
    if( const clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( node ) ) {
      if( uo->isPostfix() && index == 1 )
        return 0;
      else
        return uo->getSubExpr();
    }
    else if( const clang::BinaryOperator * bo = llvm::dyn_cast<clang::BinaryOperator>( node ) )
      return index == 0 ? bo->getLHS() : bo->getRHS();
    else if( const clang::ArraySubscriptExpr *ase = llvm::dyn_cast<clang::ArraySubscriptExpr>( node ) )
      return ( index == 0 ) ? ase->getLHS() : ase->getRHS();
    else if( const clang::ConditionalOperator *co = llvm::dyn_cast<clang::ConditionalOperator>( node ) )
      return ( index == 0 ) ? co->getCond() : ( ( index == 1 ) ? co->getTrueExpr() : co->getFalseExpr() );
    else if( is_implicit_dereference( node ) )
      return llvm::dyn_cast<clang::CallExpr>( node )->getCallee();

    assert( false && "Unknown expr type" );
    return 0; // dummy
  }

  // This method takes an argument-index as unsigned int and returns the correspondent argument-clang::Expr-pointer.
  virtual const clang::Expr* arg(unsigned int index) const {
    return arg( TI_Access::tree_node(), index );
  }

  static clang::QualType arg_type( const clang::Expr *node,  clang::ASTContext &ctx, unsigned no ) {
    if( const clang::UnaryOperator * uo = llvm::dyn_cast<clang::UnaryOperator>( node ) ) {
      if( uo->isPostfix() && ( no == 1 ) )
        return ctx.IntTy; // Dummy argument of postfix ops
    }
    const clang::Expr *argument = arg( node, no );
    if( is_implicit_dereference( node ) )
      return argument->getType().getCanonicalType();
    return TI_Type::get_reference_type_if_necessary(argument->getType().getCanonicalType(), argument, ctx);
  }

  static std::string arg_type_string( const clang::Expr *node, clang::ASTContext &ctx, unsigned no ) {
    return TI_Type::get_type_code_text(arg_type( node, ctx, no ), &ctx, 0);
  }

  virtual std::string arg_type_string (unsigned no) const {
    return arg_type_string( TI_Access::tree_node(), origin()->getASTContext(), no );
  }

  // returns whether the type of the argument with the given number is a reference-type
  bool arg_is_ref( unsigned no ) const {
    return arg_type( TI_Access::tree_node(), origin()->getASTContext(), no )->isReferenceType();
  }

  // returns whether the argument with the given number is NOT available in the call-wrapper
  // (e.g. due to short-circuit-evaluation)
  bool arg_is_unavailable( unsigned no, int wrapper ) {
    // First argument is always available
    if(no == 0) {
        return false;
    }
    // Second argument of && and ||
    if( clang::BinaryOperator *bo = llvm::dyn_cast<clang::BinaryOperator>( TI_Access::tree_node() ) ) {
      if( bo->getOpcode() == clang::BO_LAnd )
        return wrapper == 1 && no == 1;
      else if( bo->getOpcode() == clang::BO_LOr )
        return wrapper == 0 && no == 1;
      else
        return false;
    }
    // second or third argument of ?:
    else if( llvm::isa<clang::ConditionalOperator>( TI_Access::tree_node() ) )
      return no == static_cast<unsigned>( 2 - wrapper );
    else
      return false;
  }

  // returns whether this is a unary operator
  virtual bool is_unary_expr () const {
    return (llvm::isa<clang::UnaryOperator>( TI_Access::tree_node() ) && ! is_postfix_expr() ) ||
            is_implicit_dereference( TI_Access::tree_node() );
  }

  // returns whether this is a postfix operator (increment oder decrement)
  virtual bool is_postfix_expr () const {
    if( clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() ) )
      return uo->isPostfix();
    return false;
  }

  // returns whether this a binary operator ( but no array subscript operator
  virtual bool is_binary_expr () const {
    return llvm::isa<clang::BinaryOperator>( TI_Access::tree_node() );
  }

  // returns whether this is the binary array subscript operator
  virtual bool is_index_expr () const {
    return llvm::isa<clang::ArraySubscriptExpr>( TI_Access::tree_node() );
  }

  // returns whether this is a operator that changes and assigns a value
  bool is_compound_assignment() const {
    if( llvm::isa<clang::CompoundAssignOperator>( TI_Access::tree_node() ) )
      return true;
    else if( clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() ) )
      return uo->isIncrementDecrementOp();
    else
      return false;
  }

  // returns whether this is the arrow class member access operator "->"
  virtual bool is_arrow_class_member_access_expr() const {
    // built-in operator "->" is currently not supported:
    return false;
  }

  // returns whether this a ternary operator (currently only conditional operator)
  virtual bool is_ternary_expr() const {
    return llvm::isa<clang::ConditionalOperator>( TI_Access::tree_node() );
  }

  virtual bool is_implicit_conversion () const {
    // built-in conversion operators are currently not supported
    return false;
  }

  static bool is_implicit_dereference(const clang::Expr* node) {
    return llvm::isa<clang::CallExpr>( node );
  }
  
  virtual bool is_implicit_dereference() const {
    return is_implicit_dereference( TI_Access::tree_node() );
  }

  // This member-function returns whether this call uses the short-circuit evaluation. Currently only
  // the built-in operators "&&", "||" and "?:".
  bool is_short_circuiting() const {
    if( clang::BinaryOperator * bo = llvm::dyn_cast<clang::BinaryOperator>( TI_Access::tree_node() ) ) {
      return bo->getOpcode() == clang::BO_LAnd || bo->getOpcode() == clang::BO_LOr;
    }
    else {
      return llvm::isa<clang::ConditionalOperator>( TI_Access::tree_node() );
    }
  }

  // check if operator is normally forwarding the first arg as result (usually references)
  bool is_forwarding() const {
    if( llvm::isa<clang::CompoundAssignOperator>( TI_Access::tree_node() ) )
      return true;
    else if( clang::BinaryOperator * bo = llvm::dyn_cast<clang::BinaryOperator>( TI_Access::tree_node() ) )
      return bo->getOpcode() == clang::BO_Assign;
    else if( clang::UnaryOperator *uo = llvm::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() ) )
      return uo->isIncrementDecrementOp() && uo->isPrefix();
    else if( llvm::isa<clang::ArraySubscriptExpr>( TI_Access::tree_node() ) )
      return true;
    else
      return false;
  }

  // This method returns the clang::SourceLocation of the operator according to the operator-type.
  virtual clang::SourceLocation get_operator_location() const {
    clang::Expr *node = TI_Access::tree_node();
    if( clang::UnaryOperator *uo = clang::dyn_cast<clang::UnaryOperator>( node ) ) {
      return uo->getOperatorLoc();
    }
    else if( clang::BinaryOperator *bo = clang::dyn_cast<clang::BinaryOperator>( node ) ) {
      return bo->getOperatorLoc();
    }
    else if( is_implicit_dereference( node ) ) {
      assert( false && "An implicit dereference has no operator location." );
      return clang::SourceLocation(); // invalid location
    }

    assert( false && "The expr does not have THE ONE operator-location." );
    return node->getLocStart(); // dummy
  }

  // returns whether the current operator returns something
  virtual bool has_result () const {
    // All currently supported operators have a return value:
    return true;
  }

  // returns the type of the return value of the operator
  static clang::QualType result_type(clang::Expr *node, clang::ASTContext& ctx) {
    if( is_implicit_dereference( node ) ) {
      // Set the result-type manually, because we have no explicit expression, from which we could take the result-type.
      // On dereference operator, if the argument-type is T*, the result-type is T&, so first take the non-pointer-type and
      // get it as reference type:
      return ctx.getLValueReferenceType(arg(node, 0)->getType().getCanonicalType().getNonReferenceType()->getPointeeType());
    }
    return TI_Type::get_reference_type_if_necessary(node->getType().getCanonicalType(), node, ctx);
  }

  virtual clang::QualType result_type() const {
    return result_type(TI_Access::tree_node(), origin()->getASTContext());
  }

  // Built-in operators are global functions and therefore have no object/target on
  // which they are called. Hence we have no target expression and thus there
  // is no class of the type of this expression.
  virtual clang::Expr *find_target_expr() const {
    return 0;
  }
  virtual const clang::RecordDecl *target_class () const {
    return 0;
  }
  virtual const clang::RecordDecl *targetexpr_class () const {
    return 0;
  }

  void forwarded_src( TI_Builtin *src ) {
    _forwarded_src = src;
  }

  TI_Builtin *forwarded_src() {
    return _forwarded_src;
  }

  void request_packed_forward( ACM_Access *requester ) {
    if( _packed_forward_requester == 0 ) {
      if( _forwarded_src )
        _forwarded_src->request_packed_forward( requester );

      _packed_forward_requester = requester;
    }
  }

  bool forwarded_is_packed() const {
    return _packed_forward_requester != 0;
  }

  ACM_Access * packed_forward_requester() const {
    return _packed_forward_requester;
  }

  bool forwarded_needs_packing() {
    // check src for planned advice, which already packed it
    TI_Builtin *check = _forwarded_src;
    while( check ) {
      if( check->is_planned() )
        return false;
      check = check->_forwarded_src;
    }

    return true;
  }

  unsigned int forwarded_index_count() const {
    unsigned int result = 0;
    if( _forwarded_src )
      result = _forwarded_src->forwarded_index_count();

    if( is_index_expr() )
      result++;

    return result;
  }

  std::string forwarded_index_type( unsigned int i ) const {
    assert( forwarded_index_count() > i  );
    if( is_index_expr() ) {
      if( i == 0 )
        return arg_type_string( 1 );
      else
        i--;
    }

    assert( _forwarded_src );
    return _forwarded_src->forwarded_index_type( i );
  }
};


class TI_VariableAccess : public TI_Access {
private:
  TI_Builtin *_entity_src;
public:
  TI_VariableAccess() : _entity_src( 0 ) {}

  // unprotect function by forwarding (we need no special sig)
  void entity( clang::DeclaratorDecl *v ) { TI_Access::entity( v ); }
  void variable( clang::DeclaratorDecl *v ) { entity( v ); }
  clang::DeclaratorDecl *variable() const { return TI_Access::entity(); }

  virtual clang::QualType entity_type() const {
    clang::QualType type = TI_Access::entity()->getType();
    unsigned int unwrap = entity_index_count();
    while( unwrap > 0 && type->isArrayType() ) {
      type = clang::dyn_cast<clang::ArrayType>( type.getCanonicalType().getTypePtr() )->getElementType();
      unwrap--;
    }
    assert( unwrap == 0 ); // we should not meet any non array types above

    return type;
  }

  virtual std::string entity_type_string() const {
    return format_type( entity_type(), TI_Access::entity()->getASTContext() );
  }

  void entity_src( TI_Builtin *src ) { _entity_src = src; }
  virtual TI_Builtin *entity_src() const { return _entity_src; }

  virtual SpliceMode entity_src_splice_mode() const {
    TI_Builtin *src = _entity_src;
    while( src ) {
      if( src->is_planned() )
        return SpliceMode_Packed; // already done while weaving of src
      if( ( src->is_binary_expr() && ( src->operator_kind_string() == "=" ) ) || src->is_compound_assignment() )
        return SpliceMode_Pack; // this forwarding operators have side effects we cant splice, so we need to pack
      src = src->forwarded_src();
    }

    return SpliceMode_FullSplice; // no reasons to not do it, at least none found here
  }

  bool entity_is_const() const {
    clang::DeclaratorDecl *var = variable();
    return var && var->getType().isConstQualified();
  }

  virtual unsigned int entity_index_count() const {
    if( _entity_src )
      return _entity_src->forwarded_index_count();
    else
      return 0;
  }
  virtual unsigned long int entity_index_dimension( unsigned int i ) const {
    const clang::ConstantArrayType *type = llvm::dyn_cast<clang::ConstantArrayType>( variable()->getType().getCanonicalType().getTypePtr() );
    while( i > 0 )
      type = clang::dyn_cast<clang::ConstantArrayType>( type->getElementType().getTypePtr() ), i--;

    return type->getSize().getZExtValue();
  }
  virtual std::string entity_index_type( unsigned int i ) const {
    if( _entity_src )
      return _entity_src->forwarded_index_type( i );
    else
      return "void";
  }

  // unprotect function by forwarding (we need no special sig)
  void tree_node( clang::Expr* n ) { TI_Access::tree_node( n ); }

  // target type (for the JoinPoint-API)
  virtual const clang::RecordDecl *target_class () const {
    const clang::RecordDecl *result = targetexpr_class();

    // if variable is from a virtual base, we need to "downgrade" the target as member pointers cant represent that
    if( llvm::isa<clang::FieldDecl>( variable() ) ) {
      const clang::CXXRecordDecl *defining = llvm::dyn_cast_or_null<clang::CXXRecordDecl>( defining_class() );
      const clang::CXXRecordDecl *target = llvm::dyn_cast_or_null<clang::CXXRecordDecl>( result );
      if( defining && target && target->isVirtuallyDerivedFrom( defining ) )
        result = defining_class();
    }

    return result;
  }

  virtual const clang::RecordDecl *targetexpr_class() const {
    const clang::RecordDecl *result = 0;

    if( target_is_implicit() ) {
      clang::CXXMethodDecl *origin = llvm::dyn_cast<clang::CXXMethodDecl>( this->origin() );
      assert( origin );

      result = origin->getParent();
    }
    else if( has_target_expr() ) {
      const clang::Type *type = target_expr()->getType ().getTypePtr();
      result = type->getPointeeCXXRecordDecl();
      if( !result )
        result = type->getAsCXXRecordDecl();
    }
    else if( const clang::VarDecl *vd = llvm::dyn_cast<clang::VarDecl>( variable() ) ) {
      if( vd->isStaticDataMember() ) {
        const clang::DeclContext *dc = vd->getDeclContext();
        assert( dc->isRecord() );

        result = llvm::dyn_cast<clang::RecordDecl>( dc );
      }
      else
        result = 0;
    }

    return result;
  }
};

class TI_Get : public TI_VariableAccess {
public:
  TI_Get() {}

  static TI_Get *of( const ACM_Get &loc ) {
    return static_cast<TI_Get *>(loc.transform_info());
  }

  virtual bool has_result() const {
    return true;
  }

  virtual clang::QualType result_type() const {
    return entity_type();
  }
};

class TI_Set : public TI_VariableAccess {
public:
  TI_Set() {}

  static TI_Set *of( const ACM_Set &loc ) {
    return static_cast<TI_Set *>(loc.transform_info());
  }

  virtual std::string arg_type_string (unsigned no) const {
    assert( no == 0 );

    return entity_type_string();
  }

  virtual bool has_result() const {
    return false;
  }

  virtual clang::QualType result_type() const {
    clang::ASTContext& ctx = origin()->getASTContext();
    return ctx.VoidTy;
  }
};

class TI_Ref : public TI_VariableAccess {
public:
  TI_Ref() {}

  static TI_Ref *of( const ACM_Ref &loc ) {
    return static_cast<TI_Ref *>(loc.transform_info());
  }

  // emulate index on ArrayToPointerDecay
  virtual unsigned int entity_index_count() const {
    unsigned int result = TI_VariableAccess::entity_index_count();
    if( is_implicit_arraydecay() )
      result++;

    return result;
  }
  virtual std::string entity_index_type( unsigned int i ) const {
    if( is_implicit_arraydecay() && i == ( entity_index_count() - 1 ) )
      return "unsigned int";
    else
      return TI_VariableAccess::entity_index_type( i );
  }

  virtual bool has_result() const {
    return true;
  }

  virtual clang::QualType result_type() const {
    clang::Expr *node = TI_Access::tree_node();
    return TI_Type::get_reference_type_if_necessary(node->getType(), node, origin()->getASTContext());
  }

  bool result_is_ptr() const {
    return result_type().getTypePtr()->isPointerType();
  }

  const bool is_explicit_operator() const {
    if( clang::UnaryOperator *uo = clang::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() ) )
      if( uo->getOpcode() == clang::UO_AddrOf )
        return true;

    return false;
  }

  bool is_implicit_arraydecay() const {
    if( clang::ImplicitCastExpr *ice = clang::dyn_cast<clang::ImplicitCastExpr>( TI_Access::tree_node() ) )
      if( ice->getCastKind() == clang::CK_ArrayToPointerDecay )
        return true;

    return false;
  }

  const WeavePos &op_before_pos( WeaverBase &wb ) const {
    assert( clang::isa<clang::UnaryOperator>( TI_Access::tree_node() ) );
    return wb.weave_pos( clang::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() )->getOperatorLoc(), WeavePos::WP_BEFORE );
  }

  const WeavePos &op_after_pos( WeaverBase &wb ) const {
    assert( clang::isa<clang::UnaryOperator>( TI_Access::tree_node() ) );
    return get_pos_after_token( clang::dyn_cast<clang::UnaryOperator>( TI_Access::tree_node() )->getOperatorLoc(), wb );
  }
};

class TI_RefAccess : public TI_Access {
public:
  TI_RefAccess() {}

  bool entity_is_const() const {
    return entity_type().isConstQualified();
  }

  // unprotect function by forwarding (we need no special sig)
  void tree_node( clang::Expr* n ) { TI_Access::tree_node( n ); }

  // target type (for the JoinPoint-API)
  virtual const clang::RecordDecl *target_class () const {
    return 0; // refs dont have a target type currently, it has to be recovered from runtime info
  }
  virtual const clang::RecordDecl *targetexpr_class () const {
    return 0; // accesses by reference  dont have a target expr
  }

  // common function to determine entity type
  clang::QualType entity_type() const {
    return TI_Access::ref_node()->getType().getNonReferenceType();
  }

  // entity type (for the JoinPoint-API)
  virtual std::string entity_type_string() const {
    return format_type( entity_type(), TI_Access::origin()->getASTContext() );
  }
};

class TI_GetRef : public TI_RefAccess {
public:
  TI_GetRef() {}

  static const TI_GetRef *of( const ACM_GetRef &loc ) {
    return static_cast<TI_GetRef *>(loc.transform_info());
  }

  virtual bool has_result() const {
    return true;
  }

  virtual clang::QualType result_type() const {
    return entity_type();
  }
};

class TI_SetRef : public TI_RefAccess {
public:
  TI_SetRef() {}

  static const TI_SetRef *of( const ACM_SetRef &loc ) {
    return static_cast<TI_SetRef *>(loc.transform_info());
  }

  virtual std::string arg_type_string (unsigned no) const {
    assert( no == 0 );

    return entity_type_string();
  }

  virtual bool has_result() const {
    return false;
  }

  virtual clang::QualType result_type() const {
    clang::ASTContext& ctx = origin()->getASTContext();
    return ctx.VoidTy;
  }
};

class TI_Construction : public TI_Code {
  clang::FunctionDecl *_decl;
  clang::CXXRecordDecl *_that_decl;

public:
  TI_Construction () : _decl (0), _that_decl (0) {}

  void decl (clang::FunctionDecl *f) { _decl = f; }
  virtual clang::Decl *decl () const { return _that_decl; }
  void that_decl (clang::CXXRecordDecl *r) { _that_decl = r; }
  virtual clang::CXXRecordDecl *that_decl () const { return _that_decl; }
  
  // that type (for the JoinPoint-API)
  virtual std::string that_type_string() const {
    return get_type_string(_that_decl);
  }

  // target type (for the JoinPoint-API)
  virtual std::string target_type_string() const {
    return get_type_string(_that_decl);
  }

  virtual std::string arg_type_string (unsigned no) const {
    if (_decl) { // user-defined constructor
      return get_type_string (_decl->getParamDecl (no), true);
    }
    else { // built-in constructor
      assert (no == 0); // may have at most one argument
      string result;
      if (_that_decl->hasCopyConstructorWithConstParam ())
        result += "const ";
      result += get_type_string(_that_decl);
      result += "&";
      return result;
    }
  }

  // entity type (for the JoinPoint-API)
  virtual std::string entity_type_string() const {
    return format_type( _decl );
  }
};

class TI_Destruction : public TI_Code {
  clang::FunctionDecl *_decl;
  clang::CXXRecordDecl *_that_decl;

public:
  TI_Destruction () : _decl (0), _that_decl (0) {}

  void decl (clang::FunctionDecl *f) { _decl = f; }
  virtual clang::Decl *decl () const { return _that_decl; }
  void that_decl (clang::CXXRecordDecl *r) { _that_decl = r; }
  virtual clang::CXXRecordDecl *that_decl () const { return _that_decl; }

  // that type (for the JoinPoint-API)
  virtual std::string that_type_string() const {
    return get_type_string(_that_decl);
  }

  // target type (for the JoinPoint-API)
  virtual std::string target_type_string() const {
    return get_type_string(_that_decl);
  }

  // entity type (for the JoinPoint-API)
  virtual std::string entity_type_string() const {
    return format_type( _decl );
  }
};

class TI_AdviceCode : public TransformInfo {
  clang::FunctionDecl *_decl;
  ThisJoinPoint _this_join_point;
  
public:
  TI_AdviceCode () : _decl (0) {}
  
  void decl(clang::FunctionDecl *f) { _decl = f; }
  virtual clang::FunctionDecl *decl () const { return _decl; }

  clang::DeclContext *Scope () const {
    return _decl ? _decl->getParent() : 0;
  }
  string name () const {
    return _decl ? _decl->getNameAsString() : "";
  }
  string qual_name () {
    return _decl ? _decl->getQualifiedNameAsString() : "";
  }
  
  ThisJoinPoint &this_join_point () { return _this_join_point; }
  const ThisJoinPoint &this_join_point () const { return _this_join_point; }

  static TI_AdviceCode *of (const ACM_AdviceCode &loc) {
    return static_cast<TI_AdviceCode*>(loc.transform_info ());
  }
};

class TI_Introduction : public TransformInfo {
public:
  virtual clang::Decl *decl () const { return 0; }

  static TI_Introduction *of (const ACM_Introduction &loc) {
    return static_cast<TI_Introduction*>(loc.transform_info ());
  }
};

class TI_Order : public TransformInfo {
public:
  virtual clang::Decl *decl () const { return 0; }

  static TI_Order *of (const ACM_Order &loc) {
    return static_cast<TI_Order*>(loc.transform_info ());
  }
};

class TI_Pointcut : public TransformInfo {
  clang::FunctionDecl *_decl;
  int _phase;
  clang::FullSourceLoc _loc;
public:
  TI_Pointcut () : _decl(0), _phase (0) {}

  void decl (clang::FunctionDecl *c) { _decl = c; }
  virtual clang::Decl *decl () const { return _decl; }

  void phase (int p) { _phase = p; }
  int phase () const { return _phase; }
  void set_location (clang::FullSourceLoc loc) { _loc = loc; }
  clang::FullSourceLoc get_location () const { return _loc; }

  static TI_Pointcut *of (const ACM_Pointcut &loc) {
    return static_cast<TI_Pointcut*>(loc.transform_info ());
  }
};

class TI_Attribute : public TransformInfo {
  clang::Decl *_decl;
  int _phase;
//  ACM_Attribute *_attr;
  clang::FullSourceLoc _loc;
  string _params;
public:
  TI_Attribute () : _decl(0), _phase (0)/*, _pce (0)*/ {}
  ~TI_Attribute () {}
  void decl (clang::RecordDecl *c) { _decl = c; }
  void decl (clang::FunctionDecl *c) { _decl = c; }
  virtual clang::Decl *decl () const { return _decl; }

  void setParamStr(string params) {_params = params;}
  const string& getParamStr() {return _params;}
  void phase (int p) { _phase = p; }
  int phase () const { return _phase; }
//  void set_attr (ACM_Attribute *attr) { _attr = attr; }
//  ACM_Attribute *get_attr () const { return _attr; }
  void set_location (clang::FullSourceLoc loc) { _loc = loc; }
  clang::FullSourceLoc get_location () const { return _loc; }

  static TI_Attribute *of (const ACM_Attribute &loc) {
    return static_cast<TI_Attribute*>(loc.transform_info ());
  }
};

class TI_ClassSlice : public TransformInfo {
public:
  struct SliceBody {
    std::string text; // the pattern itself
    ACFileID file;    // file in which the slice (part) is implemented
    unsigned line;    // line in 'file'
    enum InsertType { TARGET_NAME, TARGET_QUAL_NAME, JP_NAME }; // kinds of replacements
    std::vector<std::pair<size_t, InsertType> > positions; // replacement positions
  };

private:
  ACFileID _slice_unit;

  // new phase 1 implementation:
  SliceBody _member_intro; // class slice body
  SliceBody _base_intro;   // introduce base classes
  bool _has_base_intro, _has_member_intro;
  std::vector<SliceBody> _non_inline_members; // members defined outside of body
  std::vector<ACFileID> _non_inline_member_units; // corresponding source units

public:

  TI_ClassSlice () : _slice_unit (0), _has_base_intro (false),
                     _has_member_intro (false) {}

  // new phase 1 implementation:
  void set_tokens (const SliceBody &member_intro, const SliceBody &base_intro,
                   bool has_base_intro, bool has_member_intro) {
    _member_intro = member_intro;
    _base_intro = base_intro;
    _has_base_intro = has_base_intro;
    _has_member_intro = has_member_intro;
  }
  const SliceBody &get_tokens () const { return _member_intro; }
  const SliceBody &get_base_tokens () const { return _base_intro; }
  std::vector<SliceBody> &non_inline_members () { return _non_inline_members; }
  std::vector<ACFileID> &non_inline_member_units () { return _non_inline_member_units; }
  void analyze_tokens (bool &has_base_intro, bool &has_member_intro) {
    has_base_intro = _has_base_intro;
    has_member_intro = _has_member_intro;
  }
  const std::string &base_intro () const {
    return _base_intro.text;
  }
  // end - new phase 1 implementation

  virtual clang::Decl *decl () const { return 0; }
  void slice_unit (ACFileID su) { _slice_unit = su; }
  ACFileID slice_unit () const { return _slice_unit; }

  static TI_ClassSlice *of (const ACM_ClassSlice &loc) {
    return static_cast<TI_ClassSlice*>(loc.transform_info ());
  }
};

#endif // __ClangTransformInfo_h__
