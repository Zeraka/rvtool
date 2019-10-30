// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2015  The 'ac++' developers (see aspectc.org)
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

#ifndef __ClangFlowAnalysis_h__
#define __ClangFlowAnalysis_h__

#include "clang/Analysis/AnalysisContext.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include <map>

class ClangFlowAnalysis {
private:
  // interface to clang's CFG
  clang::AnalysisDeclContextManager _adcm;
  clang::AnalysisDeclContext* _analysis_decl_ctx; // last-used cache

  void analyze(clang::DeclaratorDecl* func);
  void do_target_object_analysis(clang::DeclaratorDecl* func);

public:
  typedef std::map<clang::CallExpr*, unsigned int> call_target_obj_lid_map_t;

private:
  call_target_obj_lid_map_t _target_obj_lids;

public:
  // reserved ID for get_target_obj_lid(...)
  static const unsigned int NO_ID = 0;
  static const unsigned int THIS_ID = 1;

  ClangFlowAnalysis() : _analysis_decl_ctx(0) {}
  ~ClangFlowAnalysis();

  // Calculate the basic-block id for a 'stmt' within a 'func'tion
  unsigned int get_block_lid(clang::DeclaratorDecl* func, clang::Stmt* stmt);

  // Calculate an ID for the target object used in a call expression.
  // If the same ID is calculated for two different call expressions,
  // both call expression refer to the same target object.
  unsigned int get_target_obj_lid(clang::DeclaratorDecl* func, clang::CallExpr* CE);
  
  // Determine whether the current analysis succeeds/succeeded
  bool valid() const;
};

#endif // __ClangFlowAnalysis_h__
