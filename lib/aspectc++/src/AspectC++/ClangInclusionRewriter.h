//===--- Rewriters.h - Rewriter implementations     -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This header contains miscellaneous utilities for various front-end actions.
//
//===----------------------------------------------------------------------===//

#ifndef __ClangInclusionRewriter_h__
#define __ClangInclusionRewriter_h__

#include "clang/Basic/LLVM.h"

namespace clang {
class Preprocessor;
class PreprocessorOutputOptions;
class FileID;

// AspectClang LOCAL
struct FileExpansionStrategy {
  virtual ~FileExpansionStrategy() {}
  virtual bool should_expand_file(FileID From, FileID To) = 0;
};
// AspectClang LOCAL

/// RewriteMacrosInInput - Implement -rewrite-macros mode.
void RewriteMacrosInInput(Preprocessor &PP, raw_ostream *OS);

/// DoRewriteTest - A simple test for the TokenRewriter class.
void DoRewriteTest(Preprocessor &PP, raw_ostream *OS);

/// RewriteIncludesInInput - Implement -frewrite-includes mode.
void RewriteIncludesInInput(Preprocessor &PP, raw_ostream *OS,
                            const PreprocessorOutputOptions &Opts,
                            FileExpansionStrategy *strategy = 0);

}  // end namespace clang

#endif // __ClangInclusionRewriter_h__
