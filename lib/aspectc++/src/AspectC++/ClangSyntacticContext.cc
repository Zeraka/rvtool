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

#include "ClangSyntacticContext.h"
#include "ClangTransformInfo.h"

string ClangSyntacticContext::type() const {
  clang::ASTContext &ctx = _decl->getASTContext();
  return TI_Type::get_type_text(ctx.getTypeDeclType(llvm::cast<clang::TypeDecl>(_decl)), &ctx,
                                0, TSEF_ENABLE, true, TSEF_DONOTCHANGE, false, true, false);
}

int ClangSyntacticContext::is_in_extern_c_block() const {
  int depth = 0;
  const clang::DeclContext* decl_context = _decl->getDeclContext();
  while (decl_context->getDeclKind() != clang::Decl::TranslationUnit) {
    if (decl_context->getDeclKind() == clang::Decl::LinkageSpec) {
      const clang::LinkageSpecDecl* linkage_spec_decl = clang::cast<clang::LinkageSpecDecl>(decl_context);
      if(linkage_spec_decl->getLanguage() == clang::LinkageSpecDecl::lang_c && linkage_spec_decl->hasBraces()) { // only blocks have braces
        depth++;
      }
    }
    decl_context = decl_context->getLexicalParent();
  }
  return depth;
}

string ClangSyntacticContext::result_type(const string &name) const {
  const clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(_decl);
  clang::PrintingPolicy pp(fd->getASTContext().getPrintingPolicy());
  pp.SuppressUnwrittenScope = true; // Suppress <anonymous>
//  string ts = name;
//#if CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)
//  fd->getResultType().getAsStringInternal(ts, pp);
//#else // C++ 11 interface
//  fd->getReturnType().getAsStringInternal(ts, pp);
//#endif
//  return ts;
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
   clang::QualType returnType = fd->getResultType();
#else // C++ 11 interface
   clang::QualType returnType = fd->getReturnType();
#endif
  return TI_Type::get_type_code_text(returnType, &_decl->getASTContext(), name.c_str());
}

string ClangSyntacticContext::arg_type(unsigned int no,
                                       const string &name) const {
  const clang::FunctionDecl *fd = llvm::cast<clang::FunctionDecl>(_decl);
  clang::PrintingPolicy pp(fd->getASTContext().getPrintingPolicy());
  pp.SuppressUnwrittenScope = true; // Suppress <anonymous>
//  string ts = name;
//  fd->getParamDecl(no)->getType().getAsStringInternal(ts, pp);
//  return ts;
   return TI_Type::get_type_code_text(fd->getParamDecl(no)->getType(), &_decl->getASTContext(), name.c_str());
}
