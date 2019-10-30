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

#include "ClangPreprocessor.h"
#include "IncludeGraph.h"
#include "IntroductionUnit.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"

using namespace clang;

ClangPreprocessor::ClangPreprocessor (const string &tunit_name,
    ACConfig &conf, IncludeGraph &include_graph, ACProject &project) :
    _conf(conf), _project(project), _tunit_name(tunit_name) {

  CompilerInstance &CI = *_project.get_compiler_instance();

#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  CI.createPreprocessor();
#else // C++ 11 interface
  CI.createPreprocessor(clang::TU_Complete);
#endif
  Preprocessor &PP = CI.getPreprocessor();

  CI.getDiagnosticClient().BeginSourceFile(CI.getLangOpts(), &PP);

#if FRONTEND_CLANG < 38
  PP.getBuiltinInfo().InitializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());
#else
  // new interface (lowercase 'I')
  PP.getBuiltinInfo().initializeBuiltins(PP.getIdentifierTable(),
                                         PP.getLangOpts());
#endif

  CI.InitializeSourceManager(FrontendInputFile(tunit_name, IK_CXX));
  PP.EnterMainSourceFile();
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  PP.addPPCallbacks(new IncludeGraph::IncludeGraphCallback(include_graph));
#else // C++ 11 interface
  PP.addPPCallbacks(std::unique_ptr<IncludeGraph::IncludeGraphCallback>(new IncludeGraph::IncludeGraphCallback(include_graph)));
#endif
  next_token();
}

ClangPreprocessor::~ClangPreprocessor () {
  CompilerInstance &CI = *_project.get_compiler_instance();

  CI.getDiagnosticClient().EndSourceFile();
  CI.getPreprocessor().EndSourceFile();
}

unsigned ClangPreprocessor::tunit_len() const {
  CompilerInstance &CI = *_project.get_compiler_instance();
  clang::SourceManager &manager = CI.getSourceManager();
  clang::FileID main_fid = manager.getMainFileID();
  clang::SourceLocation last = manager.getLocForEndOfFile(main_fid);
  return manager.getPresumedLoc(last).getLine();
}

bool ClangPreprocessor::keywords_enabled (clang::SourceLocation loc) const {

  SourceManager &SM = _project.get_compiler_instance()->getSourceManager();
  ACFileID funit = SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc (loc)));

  if (!funit.is_valid() || !_project.isBelow(funit.name().c_str ()))
    return false;

  string name = funit.name ();
  size_t len  = name.length();
  if (len > 2 && strcmp (name.c_str () + (len - 3), ".ah") == 0)
    return true;

  return _conf.keywords();
}

ACErrorStream &ClangPreprocessor::err () const {
  return _project.err ();
}

ACToken ClangPreprocessor::next_token () {
  Preprocessor &PP = _project.get_compiler_instance()->getPreprocessor();
  clang::Token tok;
  PP.Lex(tok);
  _current_token = ACToken(tok, keywords_enabled(tok.getLocation()));
  if (!_recorded_tokens.empty())
    _recorded_tokens.push_back(_current_token);

  return _current_token;
}

ACToken ClangPreprocessor::curr_token () {
  return _current_token;
}

ACToken ClangPreprocessor::look_ahead (int n) {
  Preprocessor &PP = _project.get_compiler_instance()->getPreprocessor();
  clang::Token Tok(PP.LookAhead(n - 1));
  return ACToken(Tok, keywords_enabled(Tok.getLocation()));
}

// functions to record the token stream

void ClangPreprocessor::start_recording () {
  if (_recorded_tokens.empty())
    _recorded_tokens.push_back(curr_token());
}

ClangPreprocessor::TokenVector ClangPreprocessor::stop_recording () {
  TokenVector result;
  std::swap(result, _recorded_tokens);
  return result;
}

ACFileID ClangPreprocessor::source_unit (ACToken token) const {
  SourceManager &SM = _project.get_compiler_instance()->getSourceManager();
  return SM.getFileEntryForID(SM.getFileID(SM.getExpansionLoc (token.location ())));
}

// check whether a token is located within the files of the project
bool ClangPreprocessor::is_in_project(ACToken token) const {
  ACFileID funit = source_unit(token);
  return funit.is_valid() && _project.isBelow(funit.name().c_str ());
}

std::string ClangPreprocessor::token_text (ACToken tok) const {
  Preprocessor &PP = _project.get_compiler_instance()->getPreprocessor();
  return PP.getSpelling(tok.get());
}

unsigned ClangPreprocessor::token_line_number (ACToken tok) const {
  SourceManager &SM = _project.get_compiler_instance()->getSourceManager();
  return SM.getPresumedLineNumber(tok.location());
}
