//===--- TypePrinter.cpp - Pretty-Print Clang Types -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to print types from Clang's type system.
//
//===----------------------------------------------------------------------===//

// Adjusted by AspectC++ to support printing of absolute types, printing of elaborated type specifier and removing of attributes.
// The file TypePrinter.cpp was taken from clang version 3.6.2 and split into a header file and a file containing the implementations.
// Changes are marked with //AC++.
// See the ClangAdjustedTypePrinter.cpp file for more information.

//AC++: added include guard:
#ifndef __ClangAdjustedTypePrinter_h__
#define __ClangAdjustedTypePrinter_h__

#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/SaveAndRestore.h"
#include "llvm/Support/raw_ostream.h"
//AC++: commented out: using namespace clang;

//AC++: commented out: namespace {

//AC++: enum added:
enum TriStateEnableFeature {
  TSEF_ENABLE = true,
  TSEF_DISABLE = false,
  TSEF_DONOTCHANGE = 2,
};

//AC++: added "clang::" nested name specifier to every clang type

  /// \brief RAII object that enables printing of the ARC __strong lifetime
  /// qualifier.
  class IncludeStrongLifetimeRAII {
    clang::PrintingPolicy &Policy;
    bool Old;
    
  public:
    explicit IncludeStrongLifetimeRAII(clang::PrintingPolicy &Policy)
      : Policy(Policy), Old(Policy.SuppressStrongLifetime) {
        if (!Policy.SuppressLifetimeQualifiers)
          Policy.SuppressStrongLifetime = false;
    }
    
    ~IncludeStrongLifetimeRAII() {
      Policy.SuppressStrongLifetime = Old;
    }
  };

  class ParamPolicyRAII {
    clang::PrintingPolicy &Policy;
    bool Old;
    
  public:
    explicit ParamPolicyRAII(clang::PrintingPolicy &Policy)
      : Policy(Policy), Old(Policy.SuppressSpecifiers) {
      Policy.SuppressSpecifiers = false;
    }
    
    ~ParamPolicyRAII() {
      Policy.SuppressSpecifiers = Old;
    }
  };
/*AC++: commented out:
  class ElaboratedTypePolicyRAII {
    clang::PrintingPolicy &Policy;
    bool SuppressTagKeyword;
    bool SuppressScope;
    
  public:
    explicit ElaboratedTypePolicyRAII(clang::PrintingPolicy &Policy) : Policy(Policy) {
      SuppressTagKeyword = Policy.SuppressTagKeyword;
      SuppressScope = Policy.SuppressScope;
      Policy.SuppressTagKeyword = true;
      Policy.SuppressScope = true;
    }
    
    ~ElaboratedTypePolicyRAII() {
      Policy.SuppressTagKeyword = SuppressTagKeyword;
      Policy.SuppressScope = SuppressScope;
    }
  };
*/
  //AC++: renamed TypePrinter to AdjustedTypePrinter
  class AdjustedTypePrinter {
    clang::PrintingPolicy Policy;
    unsigned Indentation;
    bool HasEmptyPlaceHolder;
    bool InsideCCAttribute;
    //AC++: added members:
    TriStateEnableFeature absolute_qualified;
    bool keep_typedef;
    TriStateEnableFeature elaborated_keyword;
    bool remove_attributes;
    bool as_parameter_signature_type;
    const clang::ASTContext* ctx;

  public:
    //AC++: adjusted constructor:
    explicit AdjustedTypePrinter(const clang::PrintingPolicy &Policy,
                                 TriStateEnableFeature absolute_qualfied,
                                 bool keep_typedef,
                                 TriStateEnableFeature elaborated_keyword,
                                 bool remove_attributes,
                                 bool as_parameter_signature_type,
                                 const clang::ASTContext* ctx,
                                 unsigned Indentation = 0)
      : Policy(Policy), Indentation(Indentation), HasEmptyPlaceHolder(false),
        InsideCCAttribute(false),
        absolute_qualified(absolute_qualfied), keep_typedef(keep_typedef),
        elaborated_keyword(elaborated_keyword), remove_attributes(remove_attributes),
        as_parameter_signature_type(as_parameter_signature_type), ctx(ctx){ }

    //AC++: commented out:
    //explicit AdjustedTypePrinter(const PrintingPolicy &Policy)
    //     : AdjustedTypePrinter(Policy, false, false) { }

  //AC++: added private:
  private:
    void print(const clang::Type *ty, clang::Qualifiers qs, clang::raw_ostream &OS,
               clang::StringRef PlaceHolder);
  //AC++: added public:
  public:
    void print(clang::QualType T, clang::raw_ostream &OS, clang::StringRef PlaceHolder);
  //AC++: added private:
  private:
    static bool canPrefixQualifiers(const clang::Type *T, bool &NeedARCStrongQualifier);
    void spaceBeforePlaceHolder(clang::raw_ostream &OS);
    void printTypeSpec(const clang::NamedDecl *D, clang::raw_ostream &OS);

    void printBefore(const clang::Type *ty, clang::Qualifiers qs, clang::raw_ostream &OS);
    void printBefore(clang::QualType T, clang::raw_ostream &OS);
    void printAfter(const clang::Type *ty, clang::Qualifiers qs, clang::raw_ostream &OS);
    void printAfter(clang::QualType T, clang::raw_ostream &OS);
    void AppendScope(clang::DeclContext *DC, clang::raw_ostream &OS);
    void printTag(clang::TagDecl *T, clang::raw_ostream &OS);
#define ABSTRACT_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT) \
    void print##CLASS##Before(const clang::CLASS##Type *T, clang::raw_ostream &OS); \
    void print##CLASS##After(const clang::CLASS##Type *T, clang::raw_ostream &OS);
#include "clang/AST/TypeNodes.def"
  //AC++: added public:
  public:
    //AC++: added member functions. For more information see implementation.
    void adjusted_PrintTemplateArgumentList(clang::raw_ostream &OS, const clang::TemplateArgument *Args,
        unsigned NumArgs, /*AC++: commented out: const clang::PrintingPolicy &Policy,*/ bool SkipBrackets = false);
    void adjusted_TemplateArgument_print(const clang::TemplateArgument& temp_arg, /*AC++: commented out:const clang::PrintingPolicy &Policy,*/
                                         clang::raw_ostream &Out);
    void adjusted_NamedDecl_printQualifiedName(const clang::NamedDecl* named_decl, clang::raw_ostream &OS /*AC++: removed:,
                                       const clang::PrintingPolicy &P*/);
    void adjusted_printIntegral(const clang::TemplateArgument &TemplArg,
                                clang::raw_ostream &Out/*AC++: commented out: , const clang::PrintingPolicy& Policy*/);
    void adjusted_TemplateName_print(const clang::TemplateName& templ_name, clang::raw_ostream &OS/*AC++: commented out: ,
                                      const PrintingPolicy &Policy, bool SuppressNNS*/);
    void adjusted_NestedNameSpecifier_print(const clang::NestedNameSpecifier* nns, clang::raw_ostream &OS/*AC++: commented out:,
                                            const PrintingPolicy &Policy*/);
  };

//AC++: continuation in ClangAdjustedTypePrinter.cc

//AC++: Added include guard:
#endif // __ClangAdjustedTypePrinter_h__
