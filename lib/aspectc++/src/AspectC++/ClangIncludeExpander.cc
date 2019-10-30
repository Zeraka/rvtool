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

#include "ClangIncludeExpander.h"
#include "LineDirectiveMgr.h"

#include "clang/Basic/Version.h"
#include "clang/Frontend/CompilerInstance.h"
#include "ClangInclusionRewriter.h"
#include "llvm/Support/raw_os_ostream.h"

void ClangIncludeExpander::expand(std::ostream &out) {
  // Setup the strategy object that decides whether an include should be
  // expanded.
  struct ProjectStrategy : public clang::FileExpansionStrategy {
    ClangIncludeExpander &_ie;
    ProjectStrategy(ClangIncludeExpander &ie) : _ie(ie) {}

    virtual bool should_expand_file(clang::FileID From, clang::FileID To) {
      clang::SourceManager &sm =
          _ie._project.get_compiler_instance()->getSourceManager();

      // Check if From and To are coming from this project.
      const char *from_name = sm.getFileEntryForID(From)->getName();
      const char *to_name = sm.getFileEntryForID(To)->getName();
      bool from_below = _ie._project.isBelow(from_name);
      bool to_below = _ie._project.isBelow(to_name);

//      cout << "include '" << to_name
//                       << "' from '" << from_name << "'" << endl;
      if (!from_below && to_below) {
        // oops, an external file includes an internal => error!
        _ie._err << Puma::sev_error //<< this_token.location ()
                 << "internal file '" << to_name
                 << "' is included by external '" << from_name << "'"
                 << Puma::endMessage;
      }

      return to_below;
    }
  };

  clang::CompilerInstance *ci = _project.get_compiler_instance();
  if (!ci)
    return; // No compiler, nothing to do!
  // Create a new PP to get rid of old state.
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  ci->createPreprocessor();
#else // C++ 11 interface
  ci->createPreprocessor(clang::TU_Complete);
#endif
  ci->getDiagnosticClient().BeginSourceFile(ci->getLangOpts(), &ci->getPreprocessor());
  // Now call Clang's include rewriter to do the heavy lifting.
  llvm::raw_os_ostream osos(out);
  ProjectStrategy s(*this);
  clang::PreprocessorOutputOptions opts = ci->getPreprocessorOutputOpts();
  opts.ShowLineMarkers = (_line_mgr.noline () ? 0 : 1);
  clang::RewriteIncludesInInput(ci->getPreprocessor(), &osos, opts, &s);
}
