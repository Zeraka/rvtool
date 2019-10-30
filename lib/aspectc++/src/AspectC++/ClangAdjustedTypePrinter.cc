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

//AC++:
// How to add support for a new Clang version
// ******************************************
//  1) Determine the versions:
//      - "OLD": The most recent version of Clang that is currently supported by AspectC++
//      - "NEW": The version of Clang that should be supported by AspectC++
//  2) Add a version-number-#define for the NEW version ("#define VERSION_NUMBER_X_Y_Z GET_VERSION_NUMBER(X, Y, Z)") below.
//  3) Determine all changes between the OLD version of .../lib/AST/TypePrinter.cpp and the NEW version of this file.
//      - Use kdiff3, kompare, an online diff viewer, ... to see the differences between the two versions of the file.
//  4) Merge the changes that were determined in 3) into this file (ClangAdjustedTypePrinter.cpp) and also into
//     ClangAdjustedTypePrinter.h if necessary.
//      - Preprocessor directives could/ should be used to apply the merged changes only if
//        a corresponding Clang version is used (e.g.
//         //Ac++: added if
//         #if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
//           // Clang 3.8.0 changes that break the compilation if a lower Clang version is used.
//         #endif
//        ).
//      - Changes that would apply to commented out code (e.g. ObjC related code) can be ignored.
//  5) Repeat steps 3) and 4) for all of the adjusted (member) functions located at the bottom of this file:
//      - File .../lib/AST/Decl.cpp
//         - Merge changes that were made to NamedDecl::printQualifiedName(...) into adjusted_NamedDecl_printQualifiedName(...)
//      - File .../lib/AST/TemplateBase.cpp
//         - Merge changes that were made to printIntegral(...) into adjusted_printIntegral(...)
//         - Merge changes that were made to TemplateArgument::print(...) into adjusted_TemplateArgument_print(...)
//      - File .../lib/AST/TemplateName.cpp
//         - Merge changes that were made to TemplateName::print(...) into adjusted_TemplateName_print(...)
//      - File .../lib/AST/NestedNameSpecifier.cpp
//         - Merge changes that were made to NestedNameSpecifier::print(...) into adjusted_NestedNameSpecifier_print(...)
//  6) Test with every supported Clang version.
//  7) Add changelog entry below.

//AC++:
// Changelog
// *********
// 23.02.2016: - Commented out ObjC related things.
//             - Merged changes from 3.7.1 version. ObjC related changes were not merged.
// 28.03.2016: - Made AdjustedTypePrinter compatible with Clang 3.4 and 3.4.2
// 29.06.2016: - Made AdjustedTypePrinter compatible with Clang 3.8.0 (not all changes are merged yet)
// 24.07.2016: - Adopted remaining Clang 3.8.0 changes
//             - Added "How to add support for a new Clang version".
// 14.01.2017: - Made AdjustedTypePrinter compatible with Clang 3.9.1


//AC++: beginning of TypePrinter.cpp is located in ClangAdjustedTypePrinter.h

//AC++: includes added:
#include "ClangTransformInfo.h"
#include "ClangAdjustedTypePrinter.h"
#include "clang/AST/TemplateBase.h"
#include "version.h"

using namespace clang;

//AC++: Changed name of parameter from "C99" to "HasRestrictKeyword"
static void AppendTypeQualList(raw_ostream &OS, unsigned TypeQuals, bool HasRestrictKeyword) {
  bool appendSpace = false;
  if (TypeQuals & Qualifiers::Const) {
    OS << "const";
    appendSpace = true;
  }
  if (TypeQuals & Qualifiers::Volatile) {
    if (appendSpace) OS << ' ';
    OS << "volatile";
    appendSpace = true;
  }
  if (TypeQuals & Qualifiers::Restrict) {
    if (appendSpace) OS << ' ';
    if (HasRestrictKeyword) {
      OS << "restrict";
    } else {
      OS << "__restrict";
    }
  }
}

void AdjustedTypePrinter::spaceBeforePlaceHolder(raw_ostream &OS) {
  if (!HasEmptyPlaceHolder)
    OS << ' ';
}

void AdjustedTypePrinter::print(QualType t, raw_ostream &OS, StringRef PlaceHolder) {

  //AC++: Adjust the type according to given flags:
  if(keep_typedef == false) {
    // while(clang::isa<clang::TypedefType>(type)) {
    //   type = clang::dyn_cast<clang::TypedefType>(type)->desugar();
    // }
    // Doing type = type.desugar() in a loop until we do not have a TypedefType anymore does not
    // work always. Counterexample: array type with typedef'd element type.

    t = t.getCanonicalType();
  }
  if(as_parameter_signature_type) {
    t = ctx->getSignatureParameterType(t);
  }

  SplitQualType split = t.split();
  print(split.Ty, split.Quals, OS, PlaceHolder);
}

void AdjustedTypePrinter::print(const Type *T, Qualifiers Quals, raw_ostream &OS,
                        StringRef PlaceHolder) {
  if (!T) {
    OS << "NULL TYPE";
    return;
  }

  SaveAndRestore<bool> PHVal(HasEmptyPlaceHolder, PlaceHolder.empty());

  printBefore(T, Quals, OS);
  OS << PlaceHolder;
  printAfter(T, Quals, OS);
}

bool AdjustedTypePrinter::canPrefixQualifiers(const Type *T,
                                      bool &NeedARCStrongQualifier) {
  // CanPrefixQualifiers - We prefer to print type qualifiers before the type,
  // so that we get "const int" instead of "int const", but we can't do this if
  // the type is complex.  For example if the type is "int*", we *must* print
  // "int * const", printing "const int *" is different.  Only do this when the
  // type expands to a simple string.
  bool CanPrefixQualifiers = false;
  NeedARCStrongQualifier = false;
  Type::TypeClass TC = T->getTypeClass();
  if (const AutoType *AT = dyn_cast<AutoType>(T))
    TC = AT->desugar()->getTypeClass();
  if (const SubstTemplateTypeParmType *Subst
                                      = dyn_cast<SubstTemplateTypeParmType>(T))
    TC = Subst->getReplacementType()->getTypeClass();
  
  switch (TC) {
    case Type::Auto:
    case Type::Builtin:
    case Type::Complex:
    case Type::UnresolvedUsing:
    case Type::Typedef:
    case Type::TypeOfExpr:
    case Type::TypeOf:
    case Type::Decltype:
    case Type::UnaryTransform:
    case Type::Record:
    case Type::Enum:
    case Type::Elaborated:
    case Type::TemplateTypeParm:
    case Type::SubstTemplateTypeParmPack:
    case Type::TemplateSpecialization:
    case Type::InjectedClassName:
    case Type::DependentName:
    case Type::DependentTemplateSpecialization:
    case Type::ObjCObject:
    case Type::ObjCInterface:
    case Type::Atomic:
//Ac++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
    case Type::Pipe:
#endif
      CanPrefixQualifiers = true;
      break;
      
    case Type::ObjCObjectPointer:
      CanPrefixQualifiers = T->isObjCIdType() || T->isObjCClassType() ||
        T->isObjCQualifiedIdType() || T->isObjCQualifiedClassType();
      break;
      
    case Type::ConstantArray:
    case Type::IncompleteArray:
    case Type::VariableArray:
    case Type::DependentSizedArray:
      NeedARCStrongQualifier = true;
      // Fall through

//Ac++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
    case Type::Adjusted:
#endif
    case Type::Decayed:
    case Type::Pointer:
    case Type::BlockPointer:
    case Type::LValueReference:
    case Type::RValueReference:
    case Type::MemberPointer:
    case Type::DependentSizedExtVector:
    case Type::Vector:
    case Type::ExtVector:
    case Type::FunctionProto:
    case Type::FunctionNoProto:
    case Type::Paren:
    case Type::Attributed:
    case Type::PackExpansion:
    case Type::SubstTemplateTypeParm:
      CanPrefixQualifiers = false;
      break;
  }

  return CanPrefixQualifiers;
}

void AdjustedTypePrinter::printBefore(QualType T, raw_ostream &OS) {
  SplitQualType Split = T.split();

  // If we have cv1 T, where T is substituted for cv2 U, only print cv1 - cv2
  // at this level.
  Qualifiers Quals = Split.Quals;
  if (const SubstTemplateTypeParmType *Subst =
        dyn_cast<SubstTemplateTypeParmType>(Split.Ty))
    Quals -= QualType(Subst, 0).getQualifiers();

  printBefore(Split.Ty, Quals, OS);
}

/// \brief Prints the part of the type string before an identifier, e.g. for
/// "int foo[10]" it prints "int ".
void AdjustedTypePrinter::printBefore(const Type *T,Qualifiers Quals, raw_ostream &OS) {
  //AC++: Debug:
  //OS << "/*" << T->getTypeClassName() << "*/";

  if (Policy.SuppressSpecifiers && T->isSpecifierType())
    return;

  SaveAndRestore<bool> PrevPHIsEmpty(HasEmptyPlaceHolder);

  // Print qualifiers as appropriate.

  bool CanPrefixQualifiers = false;
  bool NeedARCStrongQualifier = false;
  CanPrefixQualifiers = canPrefixQualifiers(T, NeedARCStrongQualifier);

  if (CanPrefixQualifiers && !Quals.empty()) {
    if (NeedARCStrongQualifier) {
      IncludeStrongLifetimeRAII Strong(Policy);
      Quals.print(OS, Policy, /*appendSpaceIfNonEmpty=*/true);
    } else {
      Quals.print(OS, Policy, /*appendSpaceIfNonEmpty=*/true);
    }
  }

  bool hasAfterQuals = false;
  if (!CanPrefixQualifiers && !Quals.empty()) {
    hasAfterQuals = !Quals.isEmptyWhenPrinted(Policy);
    if (hasAfterQuals)
      HasEmptyPlaceHolder = false;
  }

  switch (T->getTypeClass()) {
#define ABSTRACT_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT) case Type::CLASS: \
    print##CLASS##Before(cast<CLASS##Type>(T), OS); \
    break;
#include "clang/AST/TypeNodes.def"
  }

  if (hasAfterQuals) {
    if (NeedARCStrongQualifier) {
      IncludeStrongLifetimeRAII Strong(Policy);
      Quals.print(OS, Policy, /*appendSpaceIfNonEmpty=*/!PrevPHIsEmpty.get());
    } else {
      Quals.print(OS, Policy, /*appendSpaceIfNonEmpty=*/!PrevPHIsEmpty.get());
    }
  }
}

void AdjustedTypePrinter::printAfter(QualType t, raw_ostream &OS) {
  SplitQualType split = t.split();
  printAfter(split.Ty, split.Quals, OS);
}

/// \brief Prints the part of the type string after an identifier, e.g. for
/// "int foo[10]" it prints "[10]".
void AdjustedTypePrinter::printAfter(const Type *T, Qualifiers Quals, raw_ostream &OS) {
  switch (T->getTypeClass()) {
#define ABSTRACT_TYPE(CLASS, PARENT)
#define TYPE(CLASS, PARENT) case Type::CLASS: \
    print##CLASS##After(cast<CLASS##Type>(T), OS); \
    break;
#include "clang/AST/TypeNodes.def"
  }
}

 void AdjustedTypePrinter::printBuiltinBefore(const BuiltinType *T, raw_ostream &OS) {
  OS << T->getName(Policy);
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printBuiltinAfter(const BuiltinType *T, raw_ostream &OS) { }

void AdjustedTypePrinter::printComplexBefore(const ComplexType *T, raw_ostream &OS) {
  OS << "_Complex ";
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printComplexAfter(const ComplexType *T, raw_ostream &OS) {
  printAfter(T->getElementType(), OS);
}

void AdjustedTypePrinter::printPointerBefore(const PointerType *T, raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getPointeeType(), OS);
  // Handle things like 'int (*A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeType()))
    OS << '(';
  OS << '*';
}
void AdjustedTypePrinter::printPointerAfter(const PointerType *T, raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  // Handle things like 'int (*A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeType()))
    OS << ')';
  printAfter(T->getPointeeType(), OS);
}

void AdjustedTypePrinter::printBlockPointerBefore(const BlockPointerType *T,
                                          raw_ostream &OS) {
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getPointeeType(), OS);
  OS << '^';
}
void AdjustedTypePrinter::printBlockPointerAfter(const BlockPointerType *T,
                                          raw_ostream &OS) {
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printAfter(T->getPointeeType(), OS);
}

void AdjustedTypePrinter::printLValueReferenceBefore(const LValueReferenceType *T,
                                             raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getPointeeTypeAsWritten(), OS);
  // Handle things like 'int (&A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeTypeAsWritten()))
    OS << '(';
  OS << '&';
}
void AdjustedTypePrinter::printLValueReferenceAfter(const LValueReferenceType *T,
                                            raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  // Handle things like 'int (&A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeTypeAsWritten()))
    OS << ')';
  printAfter(T->getPointeeTypeAsWritten(), OS);
}

void AdjustedTypePrinter::printRValueReferenceBefore(const RValueReferenceType *T,
                                             raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getPointeeTypeAsWritten(), OS);
  // Handle things like 'int (&&A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeTypeAsWritten()))
    OS << '(';
  OS << "&&";
}
void AdjustedTypePrinter::printRValueReferenceAfter(const RValueReferenceType *T,
                                            raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  // Handle things like 'int (&&A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeTypeAsWritten()))
    OS << ')';
  printAfter(T->getPointeeTypeAsWritten(), OS);
}

void AdjustedTypePrinter::printMemberPointerBefore(const MemberPointerType *T,
                                           raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getPointeeType(), OS);
  // Handle things like 'int (Cls::*A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeType()))
    OS << '(';

  //AC++: commented out:
  //PrintingPolicy InnerPolicy(Policy);
  //InnerPolicy.SuppressTag = false;
  //AC++: elaborated not allowed
  // inside/in front of member pointer
  // TODO: RAII
  TriStateEnableFeature old_elab = elaborated_keyword;
  elaborated_keyword = TSEF_DISABLE;
  //AC++: before:  TypePrinter(InnerPolicy).print(QualType(T->getClass(), 0), OS, StringRef());
  print(QualType(T->getClass(), 0), OS, StringRef());
  elaborated_keyword = old_elab;

  OS << "::*";
}
void AdjustedTypePrinter::printMemberPointerAfter(const MemberPointerType *T,
                                          raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  // Handle things like 'int (Cls::*A)[4];' correctly.
  // FIXME: this should include vectors, but vectors use attributes I guess.
  if (isa<ArrayType>(T->getPointeeType()))
    OS << ')';
  printAfter(T->getPointeeType(), OS);
}

void AdjustedTypePrinter::printConstantArrayBefore(const ConstantArrayType *T,
                                           raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printConstantArrayAfter(const ConstantArrayType *T,
                                          raw_ostream &OS) {
  OS << '[';
  if (T->getIndexTypeQualifiers().hasQualifiers()) {
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    AppendTypeQualList(OS, T->getIndexTypeCVRQualifiers(), Policy.Restrict);
#else
    AppendTypeQualList(OS, T->getIndexTypeCVRQualifiers(), Policy.LangOpts.C99);
#endif
    OS << ' ';
  }

  if (T->getSizeModifier() == ArrayType::Static)
    OS << "static ";

  OS << T->getSize().getZExtValue() << ']';
  printAfter(T->getElementType(), OS);
}

void AdjustedTypePrinter::printIncompleteArrayBefore(const IncompleteArrayType *T,
                                             raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printIncompleteArrayAfter(const IncompleteArrayType *T,
                                            raw_ostream &OS) {
  OS << "[]";
  printAfter(T->getElementType(), OS);
}

void AdjustedTypePrinter::printVariableArrayBefore(const VariableArrayType *T,
                                           raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printVariableArrayAfter(const VariableArrayType *T,
                                          raw_ostream &OS) {
  OS << '[';
  if (T->getIndexTypeQualifiers().hasQualifiers()) {
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    AppendTypeQualList(OS, T->getIndexTypeCVRQualifiers(), Policy.Restrict);
#else
    AppendTypeQualList(OS, T->getIndexTypeCVRQualifiers(), Policy.LangOpts.C99);
#endif
    OS << ' ';
  }

  if (T->getSizeModifier() == VariableArrayType::Static)
    OS << "static ";
  else if (T->getSizeModifier() == VariableArrayType::Star)
    OS << '*';

  if (T->getSizeExpr())
    T->getSizeExpr()->printPretty(OS, nullptr, Policy);
  OS << ']';

  printAfter(T->getElementType(), OS);
}

//Ac++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
void AdjustedTypePrinter::printAdjustedBefore(const AdjustedType *T, raw_ostream &OS) {
  // Print the adjusted representation, otherwise the adjustment will be
  // invisible.
  printBefore(T->getAdjustedType(), OS);
}
void AdjustedTypePrinter::printAdjustedAfter(const AdjustedType *T, raw_ostream &OS) {
  printAfter(T->getAdjustedType(), OS);
}
#endif

void AdjustedTypePrinter::printDecayedBefore(const DecayedType *T, raw_ostream &OS) {
  // Print as though it's a pointer.
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
  printAdjustedBefore(T, OS);
#else
  printBefore(T->getDecayedType(), OS);
#endif
}
void AdjustedTypePrinter::printDecayedAfter(const DecayedType *T, raw_ostream &OS) {
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
  printAdjustedAfter(T, OS);
#else
  printAfter(T->getDecayedType(), OS);
#endif
}

void AdjustedTypePrinter::printDependentSizedArrayBefore(
                                               const DependentSizedArrayType *T, 
                                               raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printDependentSizedArrayAfter(
                                               const DependentSizedArrayType *T, 
                                               raw_ostream &OS) {
  OS << '[';
  if (T->getSizeExpr())
    T->getSizeExpr()->printPretty(OS, nullptr, Policy);
  OS << ']';
  printAfter(T->getElementType(), OS);
}

void AdjustedTypePrinter::printDependentSizedExtVectorBefore(
                                          const DependentSizedExtVectorType *T, 
                                          raw_ostream &OS) { 
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printDependentSizedExtVectorAfter(
                                          const DependentSizedExtVectorType *T, 
                                          raw_ostream &OS) { 
  //AC++: added if:
  if(!remove_attributes) {
    OS << " __attribute__((ext_vector_type(";
    if (T->getSizeExpr())
      T->getSizeExpr()->printPretty(OS, nullptr, Policy);
    OS << ")))";
  }
  printAfter(T->getElementType(), OS);
}

void AdjustedTypePrinter::printVectorBefore(const VectorType *T, raw_ostream &OS) {
  //AC++: added if:
  if(!remove_attributes) {
    switch (T->getVectorKind()) {
    case VectorType::AltiVecPixel:
      OS << "__vector __pixel ";
      break;
    case VectorType::AltiVecBool:
      OS << "__vector __bool ";
      printBefore(T->getElementType(), OS);
      break;
    case VectorType::AltiVecVector:
      OS << "__vector ";
      printBefore(T->getElementType(), OS);
      break;
    case VectorType::NeonVector:
      OS << "__attribute__((neon_vector_type("
         << T->getNumElements() << "))) ";
      printBefore(T->getElementType(), OS);
      break;
    case VectorType::NeonPolyVector:
      OS << "__attribute__((neon_polyvector_type(" <<
            T->getNumElements() << "))) ";
      printBefore(T->getElementType(), OS);
      break;
    case VectorType::GenericVector: {
      // FIXME: We prefer to print the size directly here, but have no way
      // to get the size of the type.
      OS << "__attribute__((__vector_size__("
         << T->getNumElements()
         << " * sizeof(";
      print(T->getElementType(), OS, StringRef());
      OS << ")))) ";
      printBefore(T->getElementType(), OS);
      break;
    }
    }
  }
}
void AdjustedTypePrinter::printVectorAfter(const VectorType *T, raw_ostream &OS) {
  printAfter(T->getElementType(), OS);
} 

void AdjustedTypePrinter::printExtVectorBefore(const ExtVectorType *T,
                                       raw_ostream &OS) { 
  printBefore(T->getElementType(), OS);
}
void AdjustedTypePrinter::printExtVectorAfter(const ExtVectorType *T, raw_ostream &OS) {
  printAfter(T->getElementType(), OS);
  //AC++: added if:
  if(!remove_attributes) {
    OS << " __attribute__((ext_vector_type(";
    OS << T->getNumElements();
    OS << ")))";
  }
}

/*AC++: commented out:
void 
FunctionProtoType::printExceptionSpecification(raw_ostream &OS, 
                                               const PrintingPolicy &Policy)
                                                                         const {
  
  if (hasDynamicExceptionSpec()) {
    OS << " throw(";
    if (getExceptionSpecType() == EST_MSAny)
      OS << "...";
    else
      for (unsigned I = 0, N = getNumExceptions(); I != N; ++I) {
        if (I)
          OS << ", ";
        
        OS << getExceptionType(I).stream(Policy);
      }
    OS << ')';
  } else if (isNoexceptExceptionSpec(getExceptionSpecType())) {
    OS << " noexcept";
    if (getExceptionSpecType() == EST_ComputedNoexcept) {
      OS << '(';
      if (getNoexceptExpr())
        getNoexceptExpr()->printPretty(OS, nullptr, Policy);
      OS << ')';
    }
  }
}
*/
void AdjustedTypePrinter::printFunctionProtoBefore(const FunctionProtoType *T,
                                           raw_ostream &OS) {
  if (T->hasTrailingReturn()) {
    OS << "auto ";
    if (!HasEmptyPlaceHolder)
      OS << '(';
  } else {
    // If needed for precedence reasons, wrap the inner part in grouping parens.
    SaveAndRestore<bool> PrevPHIsEmpty(HasEmptyPlaceHolder, false);
    printBefore(
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        T->getReturnType()
#else
        T->getResultType()
#endif
        , OS);
    if (!PrevPHIsEmpty.get())
      OS << '(';
  }
}

//Ac++: commented out:
//llvm::StringRef clang::getParameterABISpelling(ParameterABI ABI) {
//  switch (ABI) {
//    case ParameterABI::Ordinary:
//      llvm_unreachable("asking for spelling of ordinary parameter ABI");
//    case ParameterABI::SwiftContext:
//      return "swift_context";
//    case ParameterABI::SwiftErrorResult:
//      return "swift_error_result";
//    case ParameterABI::SwiftIndirectResult:
//      return "swift_indirect_result";
//  }
//  llvm_unreachable("bad parameter ABI kind");
//}

void AdjustedTypePrinter::printFunctionProtoAfter(const FunctionProtoType *T,
                                          raw_ostream &OS) { 
  // If needed for precedence reasons, wrap the inner part in grouping parens.
  if (!HasEmptyPlaceHolder)
    OS << ')';
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);

//Ac++: added if
#if CLANG_VERSION_NUMBER < VERSION_NUMBER_3_6_2
#define getNumParams getNumArgs
#define getParamType getArgType
#endif

  OS << '(';
  {
    ParamPolicyRAII ParamPolicy(Policy);
    for (unsigned i = 0, e = T->getNumParams(); i != e; ++i) {
      if (i) OS << ", ";

//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
      auto EPI = T->getExtParameterInfo(i);
      if (EPI.isConsumed()) OS << "__attribute__((ns_consumed)) ";
      auto ABI = EPI.getABI();
      if (ABI != ParameterABI::Ordinary)
        OS << "__attribute__((" << getParameterABISpelling(ABI) << ")) ";
#endif

      print(T->getParamType(i), OS, StringRef());
    }
  }
  
  if (T->isVariadic()) {
    if (T->getNumParams())
      OS << ", ";
    OS << "...";
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
  } else if (T->getNumParams() == 0 && Policy.UseVoidForZeroParams) {
#else
  } else if (T->getNumParams() == 0 && !Policy.LangOpts.CPlusPlus) {
#endif
    // Do not emit int() if we have a proto, emit 'int(void)'.
    OS << "void";
  }

//Ac++: added if
#if CLANG_VERSION_NUMBER < VERSION_NUMBER_3_6_2
#undef getNumParams
#undef getParamType
#endif

  OS << ')';

  //AC++: added if:
  if(!remove_attributes) {
    //AC++: FunctionType -> clang::FuntionType
    clang::FunctionType::ExtInfo Info = T->getExtInfo();

    if (!InsideCCAttribute) {
      switch (Info.getCC()) {
      case CC_C:
        // The C calling convention is the default on the vast majority of platforms
        // we support.  If the user wrote it explicitly, it will usually be printed
        // while traversing the AttributedType.  If the type has been desugared, let
        // the canonical spelling be the implicit calling convention.
        // FIXME: It would be better to be explicit in certain contexts, such as a
        // cdecl function typedef used to declare a member function with the
        // Microsoft C++ ABI.
        break;
      case CC_X86StdCall:
        OS << " __attribute__((stdcall))";
        break;
      case CC_X86FastCall:
        OS << " __attribute__((fastcall))";
        break;
      case CC_X86ThisCall:
        OS << " __attribute__((thiscall))";
        break;
//AC++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
      case CC_X86VectorCall:
        OS << " __attribute__((vectorcall))";
        break;
#endif
      case CC_X86Pascal:
        OS << " __attribute__((pascal))";
        break;
      case CC_AAPCS:
        OS << " __attribute__((pcs(\"aapcs\")))";
        break;
      case CC_AAPCS_VFP:
        OS << " __attribute__((pcs(\"aapcs-vfp\")))";
        break;
        //AC++: kept case for backward compatibility
#if CLANG_VERSION_NUMBER <= VERSION_NUMBER_3_6_2
      case CC_PnaclCall:
        OS << " __attribute__((pnaclcall))";
        break;
#endif
      case CC_IntelOclBicc:
        OS << " __attribute__((intel_ocl_bicc))";
        break;
      case CC_X86_64Win64:
        OS << " __attribute__((ms_abi))";
        break;
      case CC_X86_64SysV:
        OS << " __attribute__((sysv_abi))";
        break;
        //AC++: added two #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_7_1
      case CC_SpirFunction:
#if CLANG_VERSION_NUMBER < VERSION_NUMBER_3_9_1
      case CC_SpirKernel:
#else
      case CC_OpenCLKernel:
#endif
        // Do nothing. These CCs are not available as attributes.
        break;
#endif
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
      case CC_Swift:
        OS << " __attribute__((swiftcall))";
        break;
      case CC_PreserveMost:
        OS << " __attribute__((preserve_most))";
        break;
      case CC_PreserveAll:
        OS << " __attribute__((preserve_all))";
        break;
#endif
      }
    }

    if (Info.getNoReturn())
      OS << " __attribute__((noreturn))";
    if (Info.getRegParm())
      OS << " __attribute__((regparm ("
         << Info.getRegParm() << ")))";
  }

  if (unsigned quals = T->getTypeQuals()) {
    OS << ' ';
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
    AppendTypeQualList(OS, quals, Policy.Restrict);
#else
    AppendTypeQualList(OS, quals, Policy.LangOpts.C99);
#endif
  }

  switch (T->getRefQualifier()) {
  case RQ_None:
    break;
    
  case RQ_LValue:
    OS << " &";
    break;
    
  case RQ_RValue:
    OS << " &&";
    break;
  }
  T->printExceptionSpecification(OS, Policy);

  if (T->hasTrailingReturn()) {
    OS << " -> ";
    print(
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        T->getReturnType()
#else
        T->getResultType()
#endif
        , OS, StringRef());
  } else
    printAfter(
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        T->getReturnType()
#else
        T->getResultType()
#endif
        , OS);
}

void AdjustedTypePrinter::printFunctionNoProtoBefore(const FunctionNoProtoType *T,
                                             raw_ostream &OS) { 
  // If needed for precedence reasons, wrap the inner part in grouping parens.
  SaveAndRestore<bool> PrevPHIsEmpty(HasEmptyPlaceHolder, false);
  printBefore(
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        T->getReturnType()
#else
        T->getResultType()
#endif
      , OS);
  if (!PrevPHIsEmpty.get())
    OS << '(';
}
void AdjustedTypePrinter::printFunctionNoProtoAfter(const FunctionNoProtoType *T,
                                            raw_ostream &OS) {
  // If needed for precedence reasons, wrap the inner part in grouping parens.
  if (!HasEmptyPlaceHolder)
    OS << ')';
  SaveAndRestore<bool> NonEmptyPH(HasEmptyPlaceHolder, false);
  
  OS << "()";
  //AC++: before if (T->getNoReturnAttr())
  if (T->getNoReturnAttr() && !remove_attributes)
    OS << " __attribute__((noreturn))";
  printAfter(
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        T->getReturnType()
#else
        T->getResultType()
#endif
      , OS);
}
void AdjustedTypePrinter::printTypeSpec(const NamedDecl *D, raw_ostream &OS) {
  //AC++: commented out:
//  IdentifierInfo *II = D->getIdentifier();
//  OS << II->getName();
  //AC++: added:
  adjusted_NamedDecl_printQualifiedName(D, OS);

  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printUnresolvedUsingBefore(const UnresolvedUsingType *T,
                                             raw_ostream &OS) {
  printTypeSpec(T->getDecl(), OS);
}
void AdjustedTypePrinter::printUnresolvedUsingAfter(const UnresolvedUsingType *T,
                                             raw_ostream &OS) { }

void AdjustedTypePrinter::printTypedefBefore(const TypedefType *T, raw_ostream &OS) {
  printTypeSpec(T->getDecl(), OS);
}
void AdjustedTypePrinter::printTypedefAfter(const TypedefType *T, raw_ostream &OS) { }
//AC++: TODO: adjust expression?
void AdjustedTypePrinter::printTypeOfExprBefore(const TypeOfExprType *T,
                                        raw_ostream &OS) {
  OS << "typeof ";
  if (T->getUnderlyingExpr())
    T->getUnderlyingExpr()->printPretty(OS, nullptr, Policy);
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printTypeOfExprAfter(const TypeOfExprType *T,
                                       raw_ostream &OS) { }

void AdjustedTypePrinter::printTypeOfBefore(const TypeOfType *T, raw_ostream &OS) {
  OS << "typeof(";
  print(T->getUnderlyingType(), OS, StringRef());
  OS << ')';
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printTypeOfAfter(const TypeOfType *T, raw_ostream &OS) { }

void AdjustedTypePrinter::printDecltypeBefore(const DecltypeType *T, raw_ostream &OS) {
  OS << "decltype(";
  if (T->getUnderlyingExpr())
    T->getUnderlyingExpr()->printPretty(OS, nullptr, Policy);
  OS << ')';
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printDecltypeAfter(const DecltypeType *T, raw_ostream &OS) { }

void AdjustedTypePrinter::printUnaryTransformBefore(const UnaryTransformType *T,
                                            raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);

  switch (T->getUTTKind()) {
    case UnaryTransformType::EnumUnderlyingType:
      OS << "__underlying_type(";
      print(T->getBaseType(), OS, StringRef());
      OS << ')';
      spaceBeforePlaceHolder(OS);
      return;
  }

  printBefore(T->getBaseType(), OS);
}
void AdjustedTypePrinter::printUnaryTransformAfter(const UnaryTransformType *T,
                                           raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);

  switch (T->getUTTKind()) {
    case UnaryTransformType::EnumUnderlyingType:
      return;
  }

  printAfter(T->getBaseType(), OS);
}

void AdjustedTypePrinter::printAutoBefore(const AutoType *T, raw_ostream &OS) {
  // If the type has been deduced, do not print 'auto'.
  if (!T->getDeducedType().isNull()) {
    printBefore(T->getDeducedType(), OS);
  } else {
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
    switch (T->getKeyword()) {
    case AutoTypeKeyword::Auto: OS << "auto"; break;
    case AutoTypeKeyword::DecltypeAuto: OS << "decltype(auto)"; break;
    case AutoTypeKeyword::GNUAutoType: OS << "__auto_type"; break;
    }
#else
    OS << (T->isDecltypeAuto() ? "decltype(auto)" : "auto");
#endif
    spaceBeforePlaceHolder(OS);
  }
}
void AdjustedTypePrinter::printAutoAfter(const AutoType *T, raw_ostream &OS) {
  // If the type has been deduced, do not print 'auto'.
  if (!T->getDeducedType().isNull())
    printAfter(T->getDeducedType(), OS);
}

void AdjustedTypePrinter::printAtomicBefore(const AtomicType *T, raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);

  OS << "_Atomic(";
  print(T->getValueType(), OS, StringRef());
  OS << ')';
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printAtomicAfter(const AtomicType *T, raw_ostream &OS) { }

//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
void AdjustedTypePrinter::printPipeBefore(const PipeType *T, raw_ostream &OS) {
  IncludeStrongLifetimeRAII Strong(Policy);

  OS << "pipe ";
  print(T->getElementType(), OS, StringRef());
  spaceBeforePlaceHolder(OS);
}

void AdjustedTypePrinter::printPipeAfter(const PipeType *T, raw_ostream &OS) {
}
#endif

/// Appends the given scope to the end of a string.
void AdjustedTypePrinter::AppendScope(DeclContext *DC, raw_ostream &OS) {
  if (DC->isTranslationUnit()) return;
  if (DC->isFunctionOrMethod()) return;
  AppendScope(DC->getParent(), OS);

  if (NamespaceDecl *NS = dyn_cast<NamespaceDecl>(DC)) {
    if (Policy.SuppressUnwrittenScope && 
        (NS->isAnonymousNamespace() || NS->isInline()))
      return;
    if (NS->getIdentifier())
      OS << NS->getName() << "::";
    else
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
      OS << "(anonymous namespace)::";
#else
      OS << "<anonymous>::";
#endif
  } else if (ClassTemplateSpecializationDecl *Spec
               = dyn_cast<ClassTemplateSpecializationDecl>(DC)) {
    IncludeStrongLifetimeRAII Strong(Policy);
    OS << Spec->getIdentifier()->getName();
    const TemplateArgumentList &TemplateArgs = Spec->getTemplateArgs();
    //AC++:  TemplateSpecializationType::PrintTemplateArgumentList(OS,
    adjusted_PrintTemplateArgumentList(OS,
                                            TemplateArgs.data(),
                                            TemplateArgs.size()/*AC++: commented out:,
                                             Policy*/);
    OS << "::";
  } else if (TagDecl *Tag = dyn_cast<TagDecl>(DC)) {
    //AC++: added curly braces
    if (TypedefNameDecl *Typedef = Tag->getTypedefNameForAnonDecl()){
      //AC++: added if block:
      if(!keep_typedef) {
        Typedef = Typedef->getCanonicalDecl();
      }
      OS << Typedef->getIdentifier()->getName() << "::";
    }
    else if (Tag->getIdentifier())
      OS << Tag->getIdentifier()->getName() << "::";
    else
      return;
  }
}

void AdjustedTypePrinter::printTag(TagDecl *D, raw_ostream &OS) {
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
  if (Policy.IncludeTagDefinition) {
    PrintingPolicy SubPolicy = Policy;
    SubPolicy.IncludeTagDefinition = false;
    D->print(OS, SubPolicy, Indentation);
    spaceBeforePlaceHolder(OS);
    return;
  }
#else
  if (Policy.SuppressTag)
    return;
#endif


  bool HasKindDecoration = false;

  // bool SuppressTagKeyword
  //   = Policy.LangOpts.CPlusPlus || Policy.SuppressTagKeyword;

  // We don't print tags unless this is an elaborated type.
  // In C, we just assume every RecordType is an elaborated type.

  //AC++: before:
  // if (!(Policy.LangOpts.CPlusPlus || Policy.SuppressTagKeyword ||
  //      D->getTypedefNameForAnonDecl())) {
  //AC++: if we have something like "union {} type_name;" then there is
  // no non-typedef'd declaration available. Therefore we can not print the
  // elaborated keyword in such cases:
  if (elaborated_keyword == TSEF_ENABLE && !D->getTypedefNameForAnonDecl()) {
    HasKindDecoration = true;
    OS << D->getKindName();
    OS << ' ';
  }

  // Compute the full nested-name-specifier for this type.
  // In C, this will always be empty except when the type
  // being printed is anonymous within other Record.
  //AC++: before: if (!Policy.SuppressScope) {
  if (absolute_qualified != TSEF_DISABLE) {
    //AC++:
    if(absolute_qualified == TSEF_ENABLE) {
      OS << "::";
    }

    AppendScope(D->getDeclContext(), OS);
  }

  if (const IdentifierInfo *II = D->getIdentifier())
    OS << II->getName();
  else if (TypedefNameDecl *Typedef = D->getTypedefNameForAnonDecl()) {
    //AC++: added if block:
    if(!keep_typedef) {
      Typedef = Typedef->getCanonicalDecl();
    }
    assert(Typedef->getIdentifier() && "Typedef without identifier?");
    OS << Typedef->getIdentifier()->getName();
  } else {
    // Make an unambiguous representation for anonymous types, e.g.
    //   (anonymous enum at /usr/include/string.h:120:9)

//Ac++: added if else and moved prefix printing out of the if-else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
      OS << (Policy.MSVCFormatting ? '`' : '(');
#elif CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
      OS << "(";
#else
      OS << "<";
#endif
    if (isa<CXXRecordDecl>(D) && cast<CXXRecordDecl>(D)->isLambda()) {
      OS << "lambda";
      HasKindDecoration = true;
    } else {
      OS << "anonymous";
    }
    
    if (Policy.AnonymousTagLocations) {
      // Suppress the redundant tag keyword if we just printed one.
      // We don't have to worry about ElaboratedTypes here because you can't
      // refer to an anonymous type with one.
      if (!HasKindDecoration)
        OS << " " << D->getKindName();

      PresumedLoc PLoc = D->getASTContext().getSourceManager().getPresumedLoc(
          D->getLocation());
      if (PLoc.isValid()) {
        OS << " at " << PLoc.getFilename()
           << ':' << PLoc.getLine()
           << ':' << PLoc.getColumn();
      }
    }
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
    OS << (Policy.MSVCFormatting ? '\'' : ')');
#elif CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
    OS << ')';
#else
    OS << '>';
#endif
  }

  // If this is a class template specialization, print the template
  // arguments.
  if (ClassTemplateSpecializationDecl *Spec
        = dyn_cast<ClassTemplateSpecializationDecl>(D)) {
    const TemplateArgument *Args;
    unsigned NumArgs;
    if (TypeSourceInfo *TAW = Spec->getTypeAsWritten()) {
      const TemplateSpecializationType *TST =
        cast<TemplateSpecializationType>(TAW->getType());
      Args = TST->getArgs();
      NumArgs = TST->getNumArgs();
    } else {
      const TemplateArgumentList &TemplateArgs = Spec->getTemplateArgs();
      Args = TemplateArgs.data();
      NumArgs = TemplateArgs.size();
    }
    IncludeStrongLifetimeRAII Strong(Policy);
    //AC++: before: TemplateSpecializationType::PrintTemplateArgumentList(OS,
    adjusted_PrintTemplateArgumentList(OS,
                                                          Args, NumArgs/*AC++: commented out:,
                                                          Policy*/);
  }

  spaceBeforePlaceHolder(OS);
}

void AdjustedTypePrinter::printRecordBefore(const RecordType *T, raw_ostream &OS) {
  printTag(T->getDecl(), OS);
}
void AdjustedTypePrinter::printRecordAfter(const RecordType *T, raw_ostream &OS) { }

void AdjustedTypePrinter::printEnumBefore(const EnumType *T, raw_ostream &OS) {
  printTag(T->getDecl(), OS);
}
void AdjustedTypePrinter::printEnumAfter(const EnumType *T, raw_ostream &OS) { }

void AdjustedTypePrinter::printTemplateTypeParmBefore(const TemplateTypeParmType *T,
                                              raw_ostream &OS) { 
  if (IdentifierInfo *Id = T->getIdentifier())
    OS << Id->getName();
  else
    OS << "type-parameter-" << T->getDepth() << '-' << T->getIndex();
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printTemplateTypeParmAfter(const TemplateTypeParmType *T,
                                             raw_ostream &OS) { } 

void AdjustedTypePrinter::printSubstTemplateTypeParmBefore(
                                             const SubstTemplateTypeParmType *T, 
                                             raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  printBefore(T->getReplacementType(), OS);
}
void AdjustedTypePrinter::printSubstTemplateTypeParmAfter(
                                             const SubstTemplateTypeParmType *T, 
                                             raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  printAfter(T->getReplacementType(), OS);
}

void AdjustedTypePrinter::printSubstTemplateTypeParmPackBefore(
                                        const SubstTemplateTypeParmPackType *T, 
                                        raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  printTemplateTypeParmBefore(T->getReplacedParameter(), OS);
}
void AdjustedTypePrinter::printSubstTemplateTypeParmPackAfter(
                                        const SubstTemplateTypeParmPackType *T, 
                                        raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  printTemplateTypeParmAfter(T->getReplacedParameter(), OS);
}

void AdjustedTypePrinter::printTemplateSpecializationBefore(
                                            const TemplateSpecializationType *T, 
                                            raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  //AC++: before: T->getTemplateName().print(OS, Policy);
  adjusted_TemplateName_print(T->getTemplateName(), OS);
  
  //AC++: before: TemplateSpecializationType::PrintTemplateArgumentList(OS,
  adjusted_PrintTemplateArgumentList(OS,
                                                        T->getArgs(), 
                                                        T->getNumArgs()/*AC++: commented out:,
                                                        Policy*/);
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printTemplateSpecializationAfter(
                                            const TemplateSpecializationType *T, 
                                            raw_ostream &OS) { } 

void AdjustedTypePrinter::printInjectedClassNameBefore(const InjectedClassNameType *T,
                                               raw_ostream &OS) {
  printTemplateSpecializationBefore(T->getInjectedTST(), OS);
}
void AdjustedTypePrinter::printInjectedClassNameAfter(const InjectedClassNameType *T,
                                               raw_ostream &OS) { }

void AdjustedTypePrinter::printElaboratedBefore(const ElaboratedType *T,
                                        raw_ostream &OS) {
//AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
  if (Policy.IncludeTagDefinition && isa<TagType>(T->getNamedType()))
#else
  if (Policy.SuppressTag && isa<TagType>(T->getNamedType()))
#endif
    return;

  //AC++: added if:
  if(elaborated_keyword != TSEF_DISABLE) {
    OS << TypeWithKeyword::getKeywordName(T->getKeyword());
    if (T->getKeyword() != ETK_None)
      OS << " ";
  }

  //AC++: commented out:We ignore the qualifiers here and print it in printTagDecl
  // if necessary.
//  NestedNameSpecifier* Qualifier = T->getQualifier();
//  if (Qualifier)
//    Qualifier->print(OS, Policy);
//
//  ElaboratedTypePolicyRAII PolicyRAII(Policy);
  printBefore(T->getNamedType(), OS);
}
void AdjustedTypePrinter::printElaboratedAfter(const ElaboratedType *T,
                                        raw_ostream &OS) {
  //AC++: commented out:
  //ElaboratedTypePolicyRAII PolicyRAII(Policy);
  printAfter(T->getNamedType(), OS);
}

void AdjustedTypePrinter::printParenBefore(const ParenType *T, raw_ostream &OS) {
  //AC++: FunctionType -> clang::FunctionType
  if (!HasEmptyPlaceHolder && !isa<clang::FunctionType>(T->getInnerType())) {
    printBefore(T->getInnerType(), OS);
    OS << '(';
  } else
    printBefore(T->getInnerType(), OS);
}
void AdjustedTypePrinter::printParenAfter(const ParenType *T, raw_ostream &OS) {
  //AC++: FunctionType -> clang::FunctionType
  if (!HasEmptyPlaceHolder && !isa<clang::FunctionType>(T->getInnerType())) {
    OS << ')';
    printAfter(T->getInnerType(), OS);
  } else
    printAfter(T->getInnerType(), OS);
}

void AdjustedTypePrinter::printDependentNameBefore(const DependentNameType *T,
                                           raw_ostream &OS) { 
  OS << TypeWithKeyword::getKeywordName(T->getKeyword());
  if (T->getKeyword() != ETK_None)
    OS << " ";
  
  T->getQualifier()->print(OS, Policy);
  
  OS << T->getIdentifier()->getName();
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printDependentNameAfter(const DependentNameType *T,
                                          raw_ostream &OS) { } 

void AdjustedTypePrinter::printDependentTemplateSpecializationBefore(
        const DependentTemplateSpecializationType *T, raw_ostream &OS) { 
  IncludeStrongLifetimeRAII Strong(Policy);
  
  OS << TypeWithKeyword::getKeywordName(T->getKeyword());
  if (T->getKeyword() != ETK_None)
    OS << " ";
  
  if (T->getQualifier())
    T->getQualifier()->print(OS, Policy);    
  OS << T->getIdentifier()->getName();
  //AC++: before: TemplateSpecializationType::PrintTemplateArgumentList(OS,
  adjusted_PrintTemplateArgumentList(OS,
                                                        T->getArgs(),
                                                        T->getNumArgs()/*AC++: commented out: ,
                                                        Policy*/);
  spaceBeforePlaceHolder(OS);
}
void AdjustedTypePrinter::printDependentTemplateSpecializationAfter(
        const DependentTemplateSpecializationType *T, raw_ostream &OS) { } 

void AdjustedTypePrinter::printPackExpansionBefore(const PackExpansionType *T,
                                           raw_ostream &OS) {
  printBefore(T->getPattern(), OS);
}
void AdjustedTypePrinter::printPackExpansionAfter(const PackExpansionType *T,
                                          raw_ostream &OS) {
  printAfter(T->getPattern(), OS);
  OS << "...";
}

void AdjustedTypePrinter::printAttributedBefore(const AttributedType *T,
                                        raw_ostream &OS) {
  // Prefer the macro forms of the GC and ownership qualifiers.
  if (T->getAttrKind() == AttributedType::attr_objc_gc ||
      T->getAttrKind() == AttributedType::attr_objc_ownership)
    return printBefore(T->getEquivalentType(), OS);

  printBefore(T->getModifiedType(), OS);

  if (T->isMSTypeSpec()) {
    switch (T->getAttrKind()) {
    default: return;
    case AttributedType::attr_ptr32: OS << " __ptr32"; break;
    case AttributedType::attr_ptr64: OS << " __ptr64"; break;
    case AttributedType::attr_sptr: OS << " __sptr"; break;
    case AttributedType::attr_uptr: OS << " __uptr"; break;
    }
    spaceBeforePlaceHolder(OS);
  }

  //AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_7_1
  // Print nullability type specifiers.
  if (T->getAttrKind() == AttributedType::attr_nonnull ||
      T->getAttrKind() == AttributedType::attr_nullable ||
      T->getAttrKind() == AttributedType::attr_null_unspecified) {
    if (T->getAttrKind() == AttributedType::attr_nonnull)
      OS << " _Nonnull";
    else if (T->getAttrKind() == AttributedType::attr_nullable)
      OS << " _Nullable";
    else if (T->getAttrKind() == AttributedType::attr_null_unspecified)
      OS << " _Null_unspecified";
    else
      llvm_unreachable("unhandled nullability");
    spaceBeforePlaceHolder(OS);
  }
#endif
}

void AdjustedTypePrinter::printAttributedAfter(const AttributedType *T,
                                       raw_ostream &OS) {
  // Prefer the macro forms of the GC and ownership qualifiers.
  if (T->getAttrKind() == AttributedType::attr_objc_gc ||
      T->getAttrKind() == AttributedType::attr_objc_ownership)
    return printAfter(T->getEquivalentType(), OS);

  // TODO: not all attributes are GCC-style attributes.
  if (T->isMSTypeSpec())
    return;

  //AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_7_1
  // Nothing to print after.
  if (T->getAttrKind() == AttributedType::attr_nonnull ||
      T->getAttrKind() == AttributedType::attr_nullable ||
      T->getAttrKind() == AttributedType::attr_null_unspecified)
    return printAfter(T->getModifiedType(), OS);
#endif

  // If this is a calling convention attribute, don't print the implicit CC from
  // the modified type.
  SaveAndRestore<bool> MaybeSuppressCC(InsideCCAttribute, T->isCallingConv());

  printAfter(T->getModifiedType(), OS);

//AC++: added #if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
  // Don't print the inert __unsafe_unretained attribute at all.
  if (T->getAttrKind() == AttributedType::attr_objc_inert_unsafe_unretained)
    return;
#endif

  //AC++: added #if for backward compatibility
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_7_1
  // Print nullability type specifiers that occur after
  if (T->getAttrKind() == AttributedType::attr_nonnull ||
      T->getAttrKind() == AttributedType::attr_nullable ||
      T->getAttrKind() == AttributedType::attr_null_unspecified) {
    if (T->getAttrKind() == AttributedType::attr_nonnull)
      OS << " _Nonnull";
    else if (T->getAttrKind() == AttributedType::attr_nullable)
      OS << " _Nullable";
    else if (T->getAttrKind() == AttributedType::attr_null_unspecified)
      OS << " _Null_unspecified";
    else
      llvm_unreachable("unhandled nullability");

    return;
  }
#endif

  //AC++: added if:
  if(!remove_attributes) {
    OS << " __attribute__((";
    switch (T->getAttrKind()) {
    default: llvm_unreachable("This attribute should have been handled already");
    case AttributedType::attr_address_space:
      OS << "address_space(";
      OS << T->getEquivalentType().getAddressSpace();
      OS << ')';
      break;

    case AttributedType::attr_vector_size: {
      OS << "__vector_size__(";
      if (const VectorType *vector =T->getEquivalentType()->getAs<VectorType>()) {
        OS << vector->getNumElements();
        OS << " * sizeof(";
        print(vector->getElementType(), OS, StringRef());
        OS << ')';
      }
      OS << ')';
      break;
    }

    case AttributedType::attr_neon_vector_type:
    case AttributedType::attr_neon_polyvector_type: {
      if (T->getAttrKind() == AttributedType::attr_neon_vector_type)
        OS << "neon_vector_type(";
      else
        OS << "neon_polyvector_type(";
      const VectorType *vector = T->getEquivalentType()->getAs<VectorType>();
      OS << vector->getNumElements();
      OS << ')';
      break;
    }

    case AttributedType::attr_regparm: {
      // FIXME: When Sema learns to form this AttributedType, avoid printing the
      // attribute again in printFunctionProtoAfter.
      OS << "regparm(";
      QualType t = T->getEquivalentType();
      while (!t->isFunctionType())
        t = t->getPointeeType();
      //AC++: FunctionType -> clang::FunctionType
      OS << t->getAs<clang::FunctionType>()->getRegParmType();
      OS << ')';
      break;
    }

    case AttributedType::attr_objc_gc: {
      OS << "objc_gc(";

      QualType tmp = T->getEquivalentType();
      while (tmp.getObjCGCAttr() == Qualifiers::GCNone) {
        QualType next = tmp->getPointeeType();
        if (next == tmp) break;
        tmp = next;
      }

      if (tmp.isObjCGCWeak())
        OS << "weak";
      else
        OS << "strong";
      OS << ')';
      break;
    }

    case AttributedType::attr_objc_ownership:
      OS << "objc_ownership(";
      switch (T->getEquivalentType().getObjCLifetime()) {
      case Qualifiers::OCL_None: llvm_unreachable("no ownership!");
      case Qualifiers::OCL_ExplicitNone: OS << "none"; break;
      case Qualifiers::OCL_Strong: OS << "strong"; break;
      case Qualifiers::OCL_Weak: OS << "weak"; break;
      case Qualifiers::OCL_Autoreleasing: OS << "autoreleasing"; break;
      }
      OS << ')';
      break;

    // FIXME: When Sema learns to form this AttributedType, avoid printing the
    // attribute again in printFunctionProtoAfter.
    case AttributedType::attr_noreturn: OS << "noreturn"; break;

    case AttributedType::attr_cdecl: OS << "cdecl"; break;
    case AttributedType::attr_fastcall: OS << "fastcall"; break;
    case AttributedType::attr_stdcall: OS << "stdcall"; break;
    case AttributedType::attr_thiscall: OS << "thiscall"; break;
//AC++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
    case AttributedType::attr_vectorcall: OS << "vectorcall"; break;
#endif
    case AttributedType::attr_pascal: OS << "pascal"; break;
    case AttributedType::attr_ms_abi: OS << "ms_abi"; break;
    case AttributedType::attr_sysv_abi: OS << "sysv_abi"; break;
    case AttributedType::attr_pcs:
    case AttributedType::attr_pcs_vfp: {
      OS << "pcs(";
     QualType t = T->getEquivalentType();
     while (!t->isFunctionType())
       t = t->getPointeeType();
     //AC++: FunctionType -> clang::FunctionType
     OS << (t->getAs<clang::FunctionType>()->getCallConv() == CC_AAPCS ?
           "\"aapcs\"" : "\"aapcs-vfp\"");
     OS << ')';
     break;
    }
    //AC++: kept case for backward compatibility
#if CLANG_VERSION_NUMBER <= VERSION_NUMBER_3_6_2
    case AttributedType::attr_pnaclcall: OS << "pnaclcall"; break;
#endif
    case AttributedType::attr_inteloclbicc: OS << "inteloclbicc"; break;
    }
    OS << "))";
  }
}

void AdjustedTypePrinter::printObjCInterfaceBefore(const ObjCInterfaceType *T,
                                           raw_ostream &OS) { 
  //AC++: added assert:
  assert(false && "ObjC not supported.");
  /*AC++: commented out:
  OS << T->getDecl()->getName();
  spaceBeforePlaceHolder(OS);
  */
}
void AdjustedTypePrinter::printObjCInterfaceAfter(const ObjCInterfaceType *T,
                                          raw_ostream &OS) { } 

void AdjustedTypePrinter::printObjCObjectBefore(const ObjCObjectType *T,
                                        raw_ostream &OS) {
  //AC++: added assert:
  assert(false && "ObjC not supported.");
  /*AC++: commented out:
  if (T->qual_empty())
    return printBefore(T->getBaseType(), OS);

  print(T->getBaseType(), OS, StringRef());
  OS << '<';
  bool isFirst = true;
  for (const auto *I : T->quals()) {
    if (isFirst)
      isFirst = false;
    else
      OS << ',';
    OS << I->getName();
  }
  OS << '>';
  spaceBeforePlaceHolder(OS);

  */
}
void AdjustedTypePrinter::printObjCObjectAfter(const ObjCObjectType *T,
                                        raw_ostream &OS) {
  //AC++: added assert:
  assert(false && "ObjC not supported.");
  /*AC++: commented out:
  if (T->qual_empty())
    return printAfter(T->getBaseType(), OS);
  */
}


void AdjustedTypePrinter::printObjCObjectPointerBefore(const ObjCObjectPointerType *T,
                                               raw_ostream &OS) {
  //AC++: added assert:
  assert(false && "ObjC not supported.");
  //AC++: commented out:
  //T->getPointeeType().getLocalQualifiers().print(OS, Policy,
  //                                              /*appendSpaceIfNonEmpty=*/true);
  /*
  assert(!T->isObjCSelType());

  if (T->isObjCIdType() || T->isObjCQualifiedIdType())
    OS << "id";
  else if (T->isObjCClassType() || T->isObjCQualifiedClassType())
    OS << "Class";
  else
    OS << T->getInterfaceDecl()->getName();

  if (!T->qual_empty()) {
    OS << '<';
    for (ObjCObjectPointerType::qual_iterator I = T->qual_begin(),
                                              E = T->qual_end();
         I != E; ++I) {
      OS << (*I)->getName();
      if (I+1 != E)
        OS << ',';
    }
    OS << '>';
  }

  if (!T->isObjCIdType() && !T->isObjCQualifiedIdType() &&
      !T->isObjCClassType() && !T->isObjCQualifiedClassType()) {
    OS << " *"; // Don't forget the implicit pointer.
  } else {
    spaceBeforePlaceHolder(OS);
  }
  */
}

void AdjustedTypePrinter::printObjCObjectPointerAfter(const ObjCObjectPointerType *T,
                                              raw_ostream &OS) { }


/* AC++: commented out:
void TemplateSpecializationType::
  PrintTemplateArgumentList(raw_ostream &OS,
                            const TemplateArgumentListInfo &Args,
                            const PrintingPolicy &Policy) {
  return PrintTemplateArgumentList(OS,
                                   Args.getArgumentArray(),
                                   Args.size(),
                                   Policy);
}
*/

//AC++: adjusted version of PrintTemplateArgumentList(...)
void AdjustedTypePrinter::adjusted_PrintTemplateArgumentList(
        raw_ostream &OS, const TemplateArgument *Args, unsigned NumArgs,
        /*AC++: commented out: const PrintingPolicy &Policy,*/ bool SkipBrackets) {
  //Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
  const char *Comma = Policy.MSVCFormatting ? "," : ", ";
#else
  const char *Comma = ", ";
#endif

  if (!SkipBrackets)
    OS << '<';

  bool needSpace = false;
  for (unsigned Arg = 0; Arg < NumArgs; ++Arg) {
    // Print the argument into a string.
    SmallString<128> Buf;
    llvm::raw_svector_ostream ArgOS(Buf);
    if (Args[Arg].getKind() == TemplateArgument::Pack) {
      if (Args[Arg].pack_size() && Arg > 0)
        OS << Comma;
      //AC++: before: PrintTemplateArgumentList(ArgOS,
      adjusted_PrintTemplateArgumentList(ArgOS,
                                Args[Arg].pack_begin(),
                                Args[Arg].pack_size(),
                                /*AC++: commented out: Policy,*/ true);
    } else {
      if (Arg > 0)
        //AC++: before: OS << ", ";
        //AC++: TODO: Change this to
        //  OS << Comma;
        // and adjust test reference output!
        OS << ",";
      //AC++: before:  Args[Arg].print(Policy, ArgOS);
      adjusted_TemplateArgument_print(Args[Arg], ArgOS);
    }
    StringRef ArgString = ArgOS.str();

    // If this is the first argument and its string representation
    // begins with the global scope specifier ('::foo'), add a space
    // to avoid printing the diagraph '<:'.
    if (!Arg && !ArgString.empty() && ArgString[0] == ':')
      OS << ' ';

    OS << ArgString;

    needSpace = (!ArgString.empty() && ArgString.back() == '>');
  }

  // If the last character of our string is '>', add another space to
  // keep the two '>''s separate tokens. We don't *have* to do this in
  // C++0x, but it's still good hygiene.
  if (needSpace)
    OS << ' ';

  if (!SkipBrackets)
    OS << '>';
}

/* AC++: commented out:
// Sadly, repeat all that with TemplateArgLoc.
void TemplateSpecializationType::
PrintTemplateArgumentList(raw_ostream &OS,
                          const TemplateArgumentLoc *Args, unsigned NumArgs,
                          const PrintingPolicy &Policy) {
  OS << '<';

  bool needSpace = false;
  for (unsigned Arg = 0; Arg < NumArgs; ++Arg) {
    if (Arg > 0)
      OS << ", ";

    // Print the argument into a string.
    SmallString<128> Buf;
    llvm::raw_svector_ostream ArgOS(Buf);
    if (Args[Arg].getArgument().getKind() == TemplateArgument::Pack) {
      PrintTemplateArgumentList(ArgOS,
                                Args[Arg].getArgument().pack_begin(),
                                Args[Arg].getArgument().pack_size(),
                                Policy, true);
    } else {
      Args[Arg].getArgument().print(Policy, ArgOS);
    }
    StringRef ArgString = ArgOS.str();

    // If this is the first argument and its string representation
    // begins with the global scope specifier ('::foo'), add a space
    // to avoid printing the diagraph '<:'.
    if (!Arg && !ArgString.empty() && ArgString[0] == ':')
      OS << ' ';

    OS << ArgString;

    needSpace = (!ArgString.empty() && ArgString.back() == '>');
  }

  // If the last character of our string is '>', add another space to
  // keep the two '>''s separate tokens. We don't *have* to do this in
  // C++0x, but it's still good hygiene.
  if (needSpace)
    OS << ' ';

  OS << '>';
}
*/


/*AC++: commented out:
std::string Qualifiers::getAsString() const {
  LangOptions LO;
  return getAsString(PrintingPolicy(LO));
}

// Appends qualifiers to the given string, separated by spaces.  Will
// prefix a space if the string is non-empty.  Will not append a final
// space.
std::string Qualifiers::getAsString(const PrintingPolicy &Policy) const {
  SmallString<64> Buf;
  llvm::raw_svector_ostream StrOS(Buf);
  print(StrOS, Policy);
  return StrOS.str();
}

bool Qualifiers::isEmptyWhenPrinted(const PrintingPolicy &Policy) const {
  if (getCVRQualifiers())
    return false;

  if (getAddressSpace())
    return false;

  if (getObjCGCAttr())
    return false;

  if (Qualifiers::ObjCLifetime lifetime = getObjCLifetime())
    if (!(lifetime == Qualifiers::OCL_Strong && Policy.SuppressStrongLifetime))
      return false;

  return true;
}

// Appends qualifiers to the given string, separated by spaces.  Will
// prefix a space if the string is non-empty.  Will not append a final
// space.
void Qualifiers::print(raw_ostream &OS, const PrintingPolicy& Policy,
                       bool appendSpaceIfNonEmpty) const {
  bool addSpace = false;

  unsigned quals = getCVRQualifiers();
  if (quals) {
    AppendTypeQualList(OS, quals);
    addSpace = true;
  }
  if (unsigned addrspace = getAddressSpace()) {
    if (addSpace)
      OS << ' ';
    addSpace = true;
    switch (addrspace) {
      case LangAS::opencl_global:
        OS << "__global";
        break;
      case LangAS::opencl_local:
        OS << "__local";
        break;
      case LangAS::opencl_constant:
        OS << "__constant";
        break;
      case LangAS::opencl_generic:
        OS << "__generic";
        break;
      default:
        OS << "__attribute__((address_space(";
        OS << addrspace;
        OS << ")))";
    }
  }
  if (Qualifiers::GC gc = getObjCGCAttr()) {
    if (addSpace)
      OS << ' ';
    addSpace = true;
    if (gc == Qualifiers::Weak)
      OS << "__weak";
    else
      OS << "__strong";
  }
  if (Qualifiers::ObjCLifetime lifetime = getObjCLifetime()) {
    if (!(lifetime == Qualifiers::OCL_Strong && Policy.SuppressStrongLifetime)){
      if (addSpace)
        OS << ' ';
      addSpace = true;
    }

    switch (lifetime) {
    case Qualifiers::OCL_None: llvm_unreachable("none but true");
    case Qualifiers::OCL_ExplicitNone: OS << "__unsafe_unretained"; break;
    case Qualifiers::OCL_Strong: 
      if (!Policy.SuppressStrongLifetime)
        OS << "__strong"; 
      break;
        
    case Qualifiers::OCL_Weak: OS << "__weak"; break;
    case Qualifiers::OCL_Autoreleasing: OS << "__autoreleasing"; break;
    }
  }

  if (appendSpaceIfNonEmpty && addSpace)
    OS << ' ';
}

std::string QualType::getAsString(const PrintingPolicy &Policy) const {
  std::string S;
  getAsStringInternal(S, Policy);
  return S;
}

std::string QualType::getAsString(const Type *ty, Qualifiers qs) {
  std::string buffer;
  LangOptions options;
  getAsStringInternal(ty, qs, buffer, PrintingPolicy(options));
  return buffer;
}

void QualType::print(const Type *ty, Qualifiers qs,
                     raw_ostream &OS, const PrintingPolicy &policy,
                     const Twine &PlaceHolder) {
  SmallString<128> PHBuf;
  StringRef PH = PlaceHolder.toStringRef(PHBuf);

  TypePrinter(policy).print(ty, qs, OS, PH);
}

void QualType::getAsStringInternal(const Type *ty, Qualifiers qs,
                                   std::string &buffer,
                                   const PrintingPolicy &policy) {
  SmallString<256> Buf;
  llvm::raw_svector_ostream StrOS(Buf);
  TypePrinter(policy).print(ty, qs, StrOS, buffer);
  std::string str = StrOS.str();
  buffer.swap(str);
}
*/

//AC++: commented out: } //AC++: belongs to anonymous namespace commented out in header file

//AC++: end of TypePrinter.cpp
//AC++: ########################################################################


//AC++: additional member functions:

//AC++: adjusted version of NamedDecl::printQualifiedName(...) (from Decl.cpp)
//AC++: added first argument as replacement of this
void AdjustedTypePrinter::adjusted_NamedDecl_printQualifiedName(const NamedDecl* named_decl, raw_ostream &OS/*AC++: removed:,
                                       const PrintingPolicy &P*/) {
  const DeclContext *Ctx = named_decl->getDeclContext();


  if (Ctx->isFunctionOrMethod()) {
    //AC++: before: printName(OS);
    named_decl->printName(OS);
    return;
  }

  typedef SmallVector<const DeclContext *, 8> ContextsTy;
  ContextsTy Contexts;

  //AC++: If the decl-to-print is a TagDecl, then handle the decl
  // itself like its contexts. Thus e.g. template arguments of the decl
  // are printed.
  bool decl_is_handled = false;
  if(isa<TagDecl>(named_decl)) {
    Contexts.push_back(cast<TagDecl>(named_decl));
    decl_is_handled = true;
  }

  // AC++:
  bool has_global_namespace = false;

  //AC++: do not collect contexts if we should not print qualifiers/nested name specifiers/contexts
  if(!(absolute_qualified == TSEF_DISABLE)) {
    // Collect contexts.
    while (Ctx && isa<NamedDecl>(Ctx)) {
      // AC++:
      if (absolute_qualified == TSEF_ENABLE && Ctx->isNamespace() && cast<NamespaceDecl>(Ctx)->getName() == "::") {
        has_global_namespace = true;
      }

      Contexts.push_back(Ctx);
      Ctx = Ctx->getParent();
    }
  }

  // AC++:
  if (elaborated_keyword == TSEF_ENABLE && isa<TagDecl>(named_decl)) {
    OS << cast<TagDecl>(named_decl)->getKindName();
    OS << ' ';
  }

  // AC++:
  if(absolute_qualified == TSEF_ENABLE && !has_global_namespace) {
    OS << "::";
  }

  //AC++: The following code uses
  //   inline raw_ostream &operator<<(raw_ostream &OS, const NamedDecl &ND) {
  //     ND.printName(OS);
  //     return OS;
  //   }


  for (ContextsTy::reverse_iterator I = Contexts.rbegin(), E = Contexts.rend();
       I != E; ++I) {
    //AC++: added if block:
    if(!keep_typedef && isa<TypedefNameDecl>(*I)){
      const TypedefNameDecl* typedefdecl = cast<TypedefNameDecl>(*I);
      QualType underlying_type = typedefdecl->getCanonicalDecl()->getTypeSourceInfo()->getType();
      // TODO: RAII
      TriStateEnableFeature old_abs = absolute_qualified;
      absolute_qualified = TSEF_DISABLE;
      print(underlying_type, OS, StringRef());
      absolute_qualified = old_abs;
    }
    else if (const ClassTemplateSpecializationDecl *Spec
          = dyn_cast<ClassTemplateSpecializationDecl>(*I)) {
      OS << Spec->getName();
      const TemplateArgumentList &TemplateArgs = Spec->getTemplateArgs();
      //AC++: before: TemplateSpecializationType::PrintTemplateArgumentList(OS,
      adjusted_PrintTemplateArgumentList(OS,
                                                            TemplateArgs.data(),
                                                            TemplateArgs.size()/*AC++: commented out: ,
                                                            P*/);
    } else if (const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(*I)) {
      //AC++: P -> Policy
      if (Policy.SuppressUnwrittenScope &&
          (ND->isAnonymousNamespace() || ND->isInline()))
        continue;
      if (ND->isAnonymousNamespace())
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
        OS << (Policy.MSVCFormatting ? "`anonymous namespace\'"
                                     : "(anonymous namespace)");
#elif CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        OS << "(anonymous namespace)";
#else
        OS << "<anonymous namespace>";
#endif

      else
        OS << *ND; // see above
    } else if (const RecordDecl *RD = dyn_cast<RecordDecl>(*I)) {
      if (!RD->getIdentifier())
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
        OS << "(anonymous " << RD->getKindName() << ')';
#else
        OS << "<anonymous " << RD->getKindName() << '>';
#endif
      else
        OS << *RD;
    } else if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(*I)) {
      const FunctionProtoType *FT = 0;
      if (FD->hasWrittenPrototype())
        //AC++: FunctionType -> clang::FunctionType
        FT = dyn_cast<FunctionProtoType>(FD->getType()->castAs<clang::FunctionType>());

      OS << *FD << '(';
      if (FT) {
        unsigned NumParams = FD->getNumParams();
        for (unsigned i = 0; i < NumParams; ++i) {
          if (i)
            OS << ", ";
          //AC++: before: OS << FD->getParamDecl(i)->getType().stream(P);
          print(FD->getParamDecl(i)->getType(), OS, StringRef());
        }

        if (FT->isVariadic()) {
          if (NumParams > 0)
            OS << ", ";
          OS << "...";
        }
      }
      OS << ')';
    } else if (const auto *ED = dyn_cast<EnumDecl>(*I)) {
      // C++ [dcl.enum]p10: Each enum-name and each unscoped
      // enumerator is declared in the scope that immediately contains
      // the enum-specifier. Each scoped enumerator is declared in the
      // scope of the enumeration.
      if (ED->isScoped() || ED->getIdentifier())
        OS << *ED;
      else
        continue;
    } else {
      OS << *cast<NamedDecl>(*I);
    }
    //AC++: added if:
    // If we handled the decl-to-print and this is the last iteration,
    // do not add the "::".
    if(!(decl_is_handled && *I == cast<DeclContext>(named_decl))) {
      OS << "::";
    }
  }

  //AC++: added first if:
  // If we already handled the decl to print (like its contexts)
  // we do not need handle it here anymore.
  if(!decl_is_handled){
    if (named_decl->getDeclName())
      //AC++: before: OS << *this;
      OS << *named_decl;
    else
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
      OS << "(anonymous)";
#else
      OS << "<anonymous>";
#endif
  }
}

//AC++: adjusted version of printIntegral (from TemplateBase.cpp) used by adjusted_TemplateArgument_print(...)
/// \brief Print a template integral argument value.
///
/// \param TemplArg the TemplateArgument instance to print.
///
/// \param Out the raw_ostream instance to use for printing.
///
/// \param Policy the printing policy for EnumConstantDecl printing.
void AdjustedTypePrinter::adjusted_printIntegral(const TemplateArgument &TemplArg,
                          raw_ostream &Out/*AC++: commented out: , const PrintingPolicy& Policy*/) {
  const ::clang::Type *T = TemplArg.getIntegralType().getTypePtr();
  const llvm::APSInt &Val = TemplArg.getAsIntegral();

//AC++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
  if (const EnumType *ET = T->getAs<EnumType>()) {
    for (const EnumConstantDecl* ECD : ET->getDecl()->enumerators()) {
      // In Sema::CheckTemplateArugment, enum template arguments value are
      // extended to the size of the integer underlying the enum type.  This
      // may create a size difference between the enum value and template
      // argument value, requiring isSameValue here instead of operator==.
      if (llvm::APSInt::isSameValue(ECD->getInitVal(), Val)) {
        // AC++: Before: ECD->printQualifiedName(Out, Policy);
        adjusted_NamedDecl_printQualifiedName(cast<NamedDecl>(ECD), Out);
        return;
      }
    }
  }
#endif

  if (T->isBooleanType()
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_8_0
      && !Policy.MSVCFormatting
#endif
  ) {
    Out << (Val.getBoolValue() ? "true" : "false");
  } else if (T->isCharType()) {
    const char Ch = Val.getZExtValue();
    Out << ((Ch == '\'') ? "'\\" : "'");
    Out.write_escaped(StringRef(&Ch, 1), /*UseHexEscapes=*/ true);
    Out << "'";
  } else {
    Out << Val;
  }
}

//AC++: adjusted version of TemplateArgument::print(...) (from TemplateBase.cpp)
//AC++: added first argument as replacement of *this
void AdjustedTypePrinter::adjusted_TemplateArgument_print(const TemplateArgument& temp_arg, /*AC++: commented out:const PrintingPolicy &Policy,*/
                             raw_ostream &Out) {
  //AC++: added nested name specifier "TemplateArgument::" at every case.
  //AC++: before: switch (getKind()) {
  switch (temp_arg.getKind()) {
  case TemplateArgument::Null:
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
    Out << "(no value)";
#else
    Out << "<no value>";
#endif
    break;

  case TemplateArgument::Type: {
    //AC++: commented out:
    //PrintingPolicy SubPolicy(Policy);
    //SubPolicy.SuppressStrongLifetime = true;

    //AC++: Before: getAsType().print(Out, SubPolicy);
    print(temp_arg.getAsType(), Out, StringRef());
    break;
  }

  case TemplateArgument::Declaration: {
    //AC++: before: NamedDecl *ND = cast<NamedDecl>(getAsDecl());
    NamedDecl *ND = cast<NamedDecl>(temp_arg.getAsDecl());
    Out << '&';
    if (ND->getDeclName()) {
      // FIXME: distinguish between pointer and reference args?
      // AC++: Before: ND->printQualifiedName(Out);
      adjusted_NamedDecl_printQualifiedName(ND, Out);
    } else {
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
      Out << "(anonymous)";
#else
      Out << "<anonymous>";
#endif
    }
    break;
  }

  case TemplateArgument::NullPtr:
    Out << "nullptr";
    break;

  case TemplateArgument::Template:
    //AC++: before: getAsTemplate().print(Out, Policy);
    adjusted_TemplateName_print(temp_arg.getAsTemplate(), Out);
    break;

  case TemplateArgument::TemplateExpansion:
    //AC++: before: getAsTemplateOrTemplatePattern().print(Out, Policy);
    adjusted_TemplateName_print(temp_arg.getAsTemplateOrTemplatePattern(), Out);
    Out << "...";
    break;

  case TemplateArgument::Integral: {
    //AC++: before: printIntegral(*this, Out, Policy);
    adjusted_printIntegral(temp_arg, Out);
    break;
  }

  case TemplateArgument::Expression:
    //AC++: before: getAsExpr()->printPretty(Out, nullptr, Policy);
    //AC++: TODO: make adjusted version!
    temp_arg.getAsExpr()->printPretty(Out, nullptr, Policy);
    break;

  case TemplateArgument::Pack:
    Out << "<";
    bool First = true;
//Ac++: added if else
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
    //AC++: before: for (const auto &P : pack_elements()) {
    for (const auto &P : temp_arg.pack_elements()) {
#else
    for (TemplateArgument::pack_iterator P_p = temp_arg.pack_begin(), PEnd = temp_arg.pack_end();
        P_p != PEnd; ++P_p) {
      const auto &P = *P_p;
#endif
      if (First)
        First = false;
      else
        Out << ", ";

      //AC++: before: P.print(Policy, Out);
      adjusted_TemplateArgument_print(P, Out);
    }
    Out << ">";
    break;
  }
}

//AC++: adjusted version of TemplateName::print(...) (from TemplateName.cpp)
//AC++: added first argument as replacement of *this
void AdjustedTypePrinter::adjusted_TemplateName_print(const TemplateName& templ_name, raw_ostream &OS/*AC++: commented out:,
                                                      const PrintingPolicy &Policy, bool SuppressNNS*/) {
  //AC++: replaced implicit *this by templ_name

  //AC++: before: if (TemplateDecl *Template = Storage.dyn_cast<TemplateDecl *>())
  //AC++: Storage is private, so take an indirection to emulate Storage.dyn_cast<TemplateDecl *>()
  TemplateDecl* Template = 0;
  if (templ_name.getKind() == TemplateName::Template && (Template = templ_name.getAsTemplateDecl()))
    //AC++: before:  OS << *Template;
    adjusted_NamedDecl_printQualifiedName(Template, OS);
  else if (QualifiedTemplateName *QTN = templ_name.getAsQualifiedTemplateName()) {
    //AC++: removed: if(!suppressNNS)
    //AC++: before:  QTN->getQualifier()->print(OS, Policy);
    adjusted_NestedNameSpecifier_print(QTN->getQualifier(), OS);

    if (QTN->hasTemplateKeyword())
      OS << "template ";
    //AC++: OS << *QTN->getDecl();
    //AC++: only print the decl (without nested name specifier):
    TriStateEnableFeature old_abs = absolute_qualified;
    absolute_qualified = TSEF_DISABLE;
    adjusted_NamedDecl_printQualifiedName(QTN->getDecl(), OS);
    absolute_qualified = old_abs;
  } else if (DependentTemplateName *DTN = templ_name.getAsDependentTemplateName()) {
    //AC++: before: if (!SuppressNNS && DTN->getQualifier())
    if (DTN->getQualifier())
      //AC++: before: DTN->getQualifier()->print(OS, Policy);
      adjusted_NestedNameSpecifier_print(DTN->getQualifier(), OS);
    OS << "template ";

    if (DTN->isIdentifier())
      //AC++: TODO: Use adjusted printing?
      OS << DTN->getIdentifier()->getName();
    else
      OS << "operator " << getOperatorSpelling(DTN->getOperator());
  } else if (SubstTemplateTemplateParmStorage *subst
               = templ_name.getAsSubstTemplateTemplateParm()) {
    //AC++: before: subst->getReplacement().print(OS, Policy, SuppressNNS);
    adjusted_TemplateArgument_print(subst->getReplacement(), OS);
  } else if (SubstTemplateTemplateParmPackStorage *SubstPack
                                        = templ_name.getAsSubstTemplateTemplateParmPack())
    OS << *SubstPack->getParameterPack();
  else {
    OverloadedTemplateStorage *OTS = templ_name.getAsOverloadedTemplate();
    //AC++: TODO: Use adjusted printing?
    (*OTS->begin())->printName(OS);
  }
}

//AC++: adjusted version of NestedNameSpecifier::print(...) (from NestedNameSpecifier.cpp)
//AC++: added first argument as replacement of this
void AdjustedTypePrinter::adjusted_NestedNameSpecifier_print(const NestedNameSpecifier* nns, raw_ostream &OS/*AC++: commented out:,
                                                             const PrintingPolicy &Policy*/) {
  //AC++: replaced implicit this by nns

  //AC++:
  if(absolute_qualified == TSEF_DISABLE) {
    return;
  }

  if (nns->getPrefix()) {
    //AC++: before getPrefix()->print(OS, Policy);
    adjusted_NestedNameSpecifier_print(nns->getPrefix(), OS);
  }
  else {
    //AC++: Add root qualifier if it does not exist
    if(absolute_qualified == TSEF_ENABLE && nns->getKind() != NestedNameSpecifier::Global) {
      OS << "::";
    }
  }

  //AC++: added qualifier to constants used in cases of switch
  switch (nns->getKind()) {
  case NestedNameSpecifier::Identifier:
    OS << nns->getAsIdentifier()->getName();
    break;

  case NestedNameSpecifier::Namespace:
    if (nns->getAsNamespace()->isAnonymousNamespace())
      return;

    OS << nns->getAsNamespace()->getName();
    break;

  case NestedNameSpecifier::NamespaceAlias:
    OS << nns->getAsNamespaceAlias()->getName();
    break;

  case NestedNameSpecifier::Global:
    break;

//Ac++: added if
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_6_2
  case NestedNameSpecifier::Super:
    OS << "__super";
    break;
#endif

  case NestedNameSpecifier::TypeSpecWithTemplate:
    OS << "template ";
    // Fall through to print the type.

  case NestedNameSpecifier::TypeSpec: {
    const Type *T = nns->getAsType();

    //AC++: commented out:
//    PrintingPolicy InnerPolicy(Policy);
//    InnerPolicy.SuppressScope = true;

    TriStateEnableFeature old_abs = absolute_qualified;
    absolute_qualified = TSEF_DISABLE;

    // Nested-name-specifiers are intended to contain minimally-qualified
    // types. An actual ElaboratedType will not occur, since we'll store
    // just the type that is referred to in the nested-name-specifier (e.g.,
    // a TypedefType, TagType, etc.). However, when we are dealing with
    // dependent template-id types (e.g., Outer<T>::template Inner<U>),
    // the type requires its own nested-name-specifier for uniqueness, so we
    // suppress that nested-name-specifier during printing.
    assert(!isa<ElaboratedType>(T) &&
           "Elaborated type in nested-name-specifier");
    if (const TemplateSpecializationType *SpecType
          = dyn_cast<TemplateSpecializationType>(T)) {
      // Print the template name without its corresponding
      // nested-name-specifier.
      //AC++: before: SpecType->getTemplateName().print(OS, InnerPolicy, true);
      adjusted_TemplateName_print(SpecType->getTemplateName(), OS);

      // Print the template argument list.
      //AC++: before: TemplateSpecializationType::PrintTemplateArgumentList(
      //     OS, SpecType->getArgs(), SpecType->getNumArgs(), InnerPolicy);
      adjusted_PrintTemplateArgumentList(OS, SpecType->getArgs(), SpecType->getNumArgs());
    } else {
      // Print the type normally
      //AC++: before: QualType(T, 0).print(OS, InnerPolicy);
      print(QualType(T, 0), OS, StringRef());
    }
    absolute_qualified = old_abs;
    break;
  }
  }

  OS << "::";
}
