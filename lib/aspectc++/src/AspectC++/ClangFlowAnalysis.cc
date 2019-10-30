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

#include "ClangFlowAnalysis.h"

#include "clang/Analysis/CFG.h"
#include "clang/Analysis/CFGStmtMap.h"
//#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/Analyses/PostOrderCFGView.h"
#include "clang/AST/Stmt.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <map>
#include <vector>

#include <iostream>

using namespace clang;

//------------------------------------------------------------------------====//
// Worklist: worklist for dataflow analysis.
//====------------------------------------------------------------------------//

namespace {

class DataflowWorklist {
  SmallVector<const CFGBlock *, 20> worklist;
  llvm::BitVector enqueuedBlocks;
  PostOrderCFGView *POV;
public:
  DataflowWorklist(const CFG &cfg, AnalysisDeclContext &Ctx)
    : enqueuedBlocks(cfg.getNumBlockIDs()),
      POV(Ctx.getAnalysis<PostOrderCFGView>()) {}
  
  void enqueueBlock(const CFGBlock *block);
  void enqueueSuccessors(const CFGBlock *block);
  void enqueuePredecessors(const CFGBlock *block);

  const CFGBlock *dequeue();

  void sortWorklist();
};

}

void DataflowWorklist::enqueueBlock(const clang::CFGBlock *block) {
  if (block && !enqueuedBlocks[block->getBlockID()]) {
    enqueuedBlocks[block->getBlockID()] = true;
    worklist.push_back(block);
  }
}
  
void DataflowWorklist::enqueueSuccessors(const clang::CFGBlock *block) {
  const unsigned OldWorklistSize = worklist.size();
  for (CFGBlock::const_succ_iterator I = block->succ_begin(),
       E = block->succ_end(); I != E; ++I) {
    enqueueBlock(*I);
  }

  if (OldWorklistSize == 0 || OldWorklistSize == worklist.size())
    return;

  sortWorklist();
}

void DataflowWorklist::enqueuePredecessors(const clang::CFGBlock *block) {
  const unsigned OldWorklistSize = worklist.size();
  for (CFGBlock::const_pred_iterator I = block->pred_begin(),
       E = block->pred_end(); I != E; ++I) {
    enqueueBlock(*I);
  }
  
  if (OldWorklistSize == 0 || OldWorklistSize == worklist.size())
    return;

  sortWorklist();
}

void DataflowWorklist::sortWorklist() {
  // sort in reverse order //FIXME: howto sort in pre-order ??
  //std::sort(worklist.rbegin(), worklist.rend(), POV->getComparator());
}

const CFGBlock *DataflowWorklist::dequeue() {
  if (worklist.empty())
    return 0;
  const CFGBlock *b = worklist.pop_back_val();
  enqueuedBlocks[b->getBlockID()] = false;
  return b;
}


//------------------------------------------------------------------------====//
// Helper functions: decide whether a variable can be tracked by the analysis
//====------------------------------------------------------------------------//

namespace {

static bool is_trackable(clang::VarDecl* VD) {
  clang::QualType type = VD->getType();

  if ( VD->hasGlobalStorage() ) {
    if (type->isPointerType() && !type.isConstQualified()) {
      return false; // pointer with global storage must be const qualified
    }
  }

  // two levels of indirection are not trackable (i.e., pointers to pointers)
  if (type->isPointerType() || type->isReferenceType()) {
    return type->getPointeeType()->isClassType();
  }

  return type->isClassType();
}
  
static bool is_trackable(clang::MemberExpr* ME) {
  clang::FieldDecl* field = dyn_cast_or_null<clang::FieldDecl>(ME->getMemberDecl());
  if (field) {
    clang::QualType type = field->getType();

    // remove reference/pointer attribute from type 
    if ( (type->isPointerType() && type.isConstQualified()) || type->isReferenceType()) {
      type = type->getPointeeType(); // 1st dereference
      while ( type->isPointerType() ) {
        if ( !type.isConstQualified() ) {
          return false; // pointer to pointer must be const qualified
        }
        type = type->getPointeeType();
      }
    }
    if (!type->isClassType()) {
      return false;
    }

    clang::Expr* base = ME->getBase()->IgnoreImplicit();
    if( dyn_cast_or_null<CXXThisExpr>(base) ) {
      return true;
    }
    else if (clang::MemberExpr* ME_base = dyn_cast_or_null<MemberExpr>(base)) {
      return is_trackable(ME_base);
    }
    else if (clang::DeclRefExpr* DRF = dyn_cast_or_null<DeclRefExpr>(base)) {
      clang::VarDecl* VD = dyn_cast_or_null<VarDecl>(DRF->getDecl());
      if (VD == 0) {
        return false;
      }
      if (VD->getType()->isPointerType() && !VD->getType().isConstQualified() ) {
        return false; // do not allow non-const pointers, yet. (TODO)
      }
      return is_trackable(VD);
    }
  }
  return false;
}

// determine wheter two member expressions refer to the same object
static bool referencesSameObject(clang::Stmt* stmt1, clang::Stmt* stmt2) {
  if ( clang::DeclRefExpr* decl_ref1 = dyn_cast_or_null<DeclRefExpr>(stmt1) ) {
    if ( clang::DeclRefExpr* decl_ref2 = dyn_cast_or_null<DeclRefExpr>(stmt2) ) {
      if ( clang::VarDecl* var_decl1 = dyn_cast_or_null<VarDecl>(decl_ref1->getDecl()) ) {
        if ( clang::VarDecl* var_decl2 = dyn_cast_or_null<VarDecl>(decl_ref2->getDecl()) ) {
          // Note: The pointer (var_decl) must not have been changed in between.
          // Thus, we only allow const pointers, yet (TODO)
          // See: is_trackable(clang::MemberExpr* ME)
          if ( (decl_ref1->child_begin() == decl_ref1->child_end()) &&
               (decl_ref2->child_begin() == decl_ref2->child_end()) ) {
            // there must not exist any children, hence, we can return:
            return var_decl1 == var_decl2;
          }
        }
      }
    }
    return false;
  }
  else if ( clang::MemberExpr* member_expr1 = dyn_cast_or_null<MemberExpr>(stmt1) ) {
    if ( clang::MemberExpr* member_expr2 = dyn_cast_or_null<MemberExpr>(stmt2) ) {
      if ( clang::FieldDecl* field1 = dyn_cast_or_null<clang::FieldDecl>(member_expr1->getMemberDecl()) ) {
        if ( clang::FieldDecl* field2 = dyn_cast_or_null<clang::FieldDecl>(member_expr2->getMemberDecl()) ) {
          if ( field1 == field2 ) {
            return referencesSameObject( member_expr1->getBase()->IgnoreImplicit(),
                                         member_expr2->getBase()->IgnoreImplicit() );
          }
        }
      }
    }
    return false;
  }
  else if ( dyn_cast_or_null<CXXThisExpr>(stmt1) && dyn_cast_or_null<CXXThisExpr>(stmt2) ) {
    return true;
  }
  else if ( /*clang::ArraySubscriptExpr* array_expr1 =*/ dyn_cast_or_null<ArraySubscriptExpr>(stmt1) ) {
    // TODO
    /*
    if ( clang::ArraySubscriptExpr* array_expr2 = dyn_cast_or_null<ArraySubscriptExpr>(stmt2) ) {
      if ( clang::IntegerLiteral* literal1 = dyn_cast_or_null<IntegerLiteral>(array_expr1->getIdx()) ) {
        if ( clang::IntegerLiteral* literal2 = dyn_cast_or_null<IntegerLiteral>(array_expr2->getIdx()) ) {
          if( literal1->getValue() == literal2->getValue() ) {
            return referencesSameObject( array_expr1->getBase()->IgnoreImplicit(),
                                         array_expr2->getBase()->IgnoreImplicit() );
          }
        }
      }
    }
    */
    return false;
  }
  else if ( dyn_cast_or_null<CallExpr>(stmt1) || dyn_cast_or_null<CallExpr>(stmt2) ) {
    return false;
  }
  else if ( dyn_cast_or_null<ConditionalOperator>(stmt1) || dyn_cast_or_null<ConditionalOperator>(stmt2) ) {
    return false;
  }

  // recurse to all children
  Stmt::child_iterator it1 = stmt1->child_begin();
  Stmt::child_iterator it2 = stmt2->child_begin();
  for( ; (it1 != stmt1->child_end()) && (it2 != stmt2->child_end()); it1++, it2++) {
    if ( referencesSameObject((*it1)->IgnoreImplicit(), (*it2)->IgnoreImplicit()) == false ) {
      return false;
    }
  }
  return true;
}

}


//------------------------------------------------------------------------====//
// DataFlowValues: Keep track of variables in each basic block
//====------------------------------------------------------------------------//

namespace {

class DataFlowValues {
private:
  static unsigned int next_free_id;
  
  typedef std::map<clang::VarDecl*, unsigned int> Variable_Map_t;
  Variable_Map_t _local_variables;
  Variable_Map_t _global_variables;

  // record updates of local variables (i.e. assignments), in order to
  // replay the same updates in each iteration (do not use new ids)
  typedef std::pair<clang::VarDecl*, clang::Stmt*> Update_Location_t;
  typedef std::map<Update_Location_t, unsigned int> Variable_Updates_t;
  Variable_Updates_t _updates;

  // record merge conflicts of local variables and their resolution
  Variable_Map_t _conflicts;

  // store member expressions that have already been used as target object
  // TODO: allow non-const pointers as base for a member expression
  //typedef std::pair<clang::MemberExpr*, unsigned int> Member_ID_t;
  //typedef std::map<Member_ID_t, unsigned int> Member_Expr_Map_t;
  typedef std::map<clang::MemberExpr*, unsigned int> Member_Expr_Map_t;
  Member_Expr_Map_t _member_expr_map;

  // TODO
  // store array expressions that have already been used as target object
  //typedef std::map<clang::ArraySubscriptExpr*, unsigned int> Array_Expr_Map_t;
  //Array_Expr_Map_t _array_expr_map;

public:
  static const unsigned int NO_ID = ClangFlowAnalysis::NO_ID;
  static const unsigned int THIS_ID = ClangFlowAnalysis::THIS_ID;
  
  unsigned int get_next_free_id() const { return next_free_id; }
  static void reset_next_free_id() { next_free_id = 2; }
  
  bool equals(const DataFlowValues& other) const {
    return (_local_variables == other._local_variables) &&
           (_global_variables == other._global_variables) &&
           (_member_expr_map == other._member_expr_map);
  }
  
  void copyHistory(const DataFlowValues& other) {
    _conflicts = other._conflicts;
    _updates = other._updates;
  }
  
  void add_variable(clang::VarDecl* VD) {
    add_variable(VD, get_next_free_id());
  }
  
  void add_variable(clang::VarDecl* VD, unsigned int new_id) {
    if (!is_trackable(VD)) {
      return;
    }
    if( VD->hasLocalStorage() ) {
      _local_variables[VD] = new_id;
      if (new_id == next_free_id) { next_free_id++; }
    }
    else { // hasGlobalStorage
      _global_variables[VD] = new_id;
      if (new_id == next_free_id) { next_free_id++; }
    }
  }
  
  // VD is the variable to update, location the statement in the source code
  // that triggers the update (e.g., a DeclRefExpr)
  void update_variable(clang::VarDecl* VD, clang::Stmt* location, unsigned int new_id) {
    if (VD == 0 || VD->hasGlobalStorage() || !is_trackable(VD)) {
      return; // only track updates of 'trackable' local variables
    }

    // do not allow 'kill' updates on objects (class-type variables)
    if ( (new_id == NO_ID) && (VD->getType()->isClassType()) ) {
      return;
    }

    // do not allow any updates on killed variables (with ID == 0)
    Variable_Map_t::iterator found = _local_variables.find(VD);
    if (found != _local_variables.end()) {
      if (found->second == NO_ID) {
        return;
      }
    }
    
    if ( new_id == next_free_id ) {
      // try to replay an existing update instead of a new_id
      Variable_Updates_t::iterator update = _updates.find(std::make_pair(VD, location));
      if ( update != _updates.end() ) {
        add_variable(VD, update->second);
        return;
      }
    }

    add_variable(VD, new_id);
    if ( new_id != NO_ID ) { // no need to remember NO_ID updates (won't merge)
      _updates[std::make_pair(VD, location)] = new_id;
    }
  }
  
  bool is_tracked(clang::VarDecl* VD) const {
    if( VD->hasLocalStorage() ) {
      Variable_Map_t::const_iterator found = _local_variables.find(VD);
      return found != _local_variables.end();
    }
    else { // hasGlobalStorage
      Variable_Map_t::const_iterator found = _global_variables.find(VD);
      return found != _global_variables.end();
    }
  }
  
  unsigned int get_id(clang::VarDecl* VD) const {
    if( VD->hasLocalStorage() ) {
      Variable_Map_t::const_iterator found = _local_variables.find(VD);
      if (found != _local_variables.end()) {
        return found->second;
      }
    }
    else { // hasGlobalStorage
      Variable_Map_t::const_iterator found = _global_variables.find(VD);
      if (found != _global_variables.end()) {
        return found->second;
      }
    }
    return NO_ID;
  }
  
  unsigned int get_id(clang::MemberExpr* ME) const {
    for(Member_Expr_Map_t::const_iterator it = _member_expr_map.begin();
        it != _member_expr_map.end(); ++it) {
      if( it->first == ME ) {
        return it->second; // same member-expression pointer found
      }
      else if( referencesSameObject(it->first, ME) ) {
        return it->second;
      }
    }
    // no match found
    return NO_ID;
  }
  
  unsigned int add_member_expr(clang::MemberExpr* ME) {
    _member_expr_map[ME] = next_free_id;
    return next_free_id++;
  }
  
  void merge(const DataFlowValues& src) {
    // merge local variables
    for ( Variable_Map_t::const_iterator it = src._local_variables.begin();
          it != src._local_variables.end(); it++) {
      unsigned int src_id = it->second;

      Variable_Map_t::iterator found = _local_variables.find(it->first);
      if( found != _local_variables.end() ) {
        unsigned int dst_id = found->second;
        if ( src_id != dst_id ) {
          // a merge with a killed variable always yields a 'killed' result
          if ( (src_id == NO_ID) || (dst_id == NO_ID) ) {
            found->second = NO_ID;
          }
          else {
            // did we solve that conflict before?
            Variable_Map_t::iterator conflict = _conflicts.find(it->first);
            if (conflict != _conflicts.end() ) {
              found->second = conflict->second; // adopt previous resolution
            }
            else {
              // multiple sources but different values: assign new id
              unsigned int new_id = next_free_id++;
              _conflicts[it->first] = new_id; // remember this resolution
              found->second = new_id;
            }
          }
        }
      }
      else {
        _local_variables[it->first] = src_id; // just copy
      }
    }

    // fix loops (use new ids for overwritten variables) 
    for ( Variable_Updates_t::iterator it = _updates.begin();
          it != _updates.end(); it++) {
      // if we find an update, we've already visited this basic block before
      clang::VarDecl* updated_var = it->first.first;

      // look if we already solved a merge-conflict on 'updated'var'
      Variable_Map_t::iterator conflict = _conflicts.find(updated_var);
      if (conflict == _conflicts.end()) {
         // use a new id and remember this as a conflict resolution
         unsigned int new_id = next_free_id++;
        _local_variables[updated_var] = new_id;
        _conflicts[updated_var] = new_id;
      }
    }

    // merge global variables
    for ( Variable_Map_t::const_iterator it = src._global_variables.begin();
          it != src._global_variables.end(); it++) {
      unsigned int src_id = it->second;

      Variable_Map_t::iterator found = _global_variables.find(it->first);
      if( found != _global_variables.end() ) {
        unsigned int dst_id = found->second;
        found->second = std::min(src_id, dst_id); // use smaller one
      }
      else {
        _global_variables[it->first] = src_id; // just copy
      }
    }

    // merge member expressions
    for(Member_Expr_Map_t::const_iterator it_src = src._member_expr_map.begin();
        it_src != src._member_expr_map.end(); ++it_src) {
      clang::MemberExpr* ME_src = it_src->first;

      bool found = false;
      for(Member_Expr_Map_t::iterator it_dst = _member_expr_map.begin();
          it_dst != _member_expr_map.end(); ++it_dst) {
        clang::MemberExpr* ME_dst = it_dst->first;
        if ( (ME_src == ME_dst) || referencesSameObject(ME_src, ME_dst) ) {
          it_dst->second = std::min(it_dst->second, it_src->second);
          found = true;
          break;
        }
      }
      if (found == false) {
        _member_expr_map[ME_src] = it_src->second; // just copy
      }
    }
  }
  
  void dump() const {
    for ( Variable_Map_t::const_iterator it = _local_variables.begin();
          it != _local_variables.end(); it++) {
      std::cout << it->first->getNameAsString() << " = " << it->second << " (local)" << std::endl;
    }
    for ( Variable_Map_t::const_iterator it = _global_variables.begin();
          it != _global_variables.end(); it++) {
      std::cout << it->first->getNameAsString() << " = " << it->second << " (global)" << std::endl;
    }
  }
};

}

unsigned int DataFlowValues::next_free_id = 2;


//------------------------------------------------------------------------====//
// TransferFunctions: Perform the dataflow analysis for one basic block
//====------------------------------------------------------------------------//

namespace {

class TransferFunctions : public clang::RecursiveASTVisitor<TransferFunctions> {
private:
  DataFlowValues& _dfvals;
  ClangFlowAnalysis::call_target_obj_lid_map_t& _target_obj_lids;
  const clang::ASTContext& _context;
  
  void update(clang::Stmt* stmt, unsigned int new_id);
  unsigned int lookup_target_obj_lid(clang::Expr* expr);

public:
  TransferFunctions(DataFlowValues& dfvals,
                    ClangFlowAnalysis::call_target_obj_lid_map_t& lids,
                    clang::ASTContext& ctx)
    : _dfvals(dfvals), _target_obj_lids(lids), _context(ctx) {}
  
  bool VisitDeclStmt(DeclStmt *DS); // variable initialization
  bool VisitCallExpr(CallExpr *CE);
  bool VisitBinaryOperator(BinaryOperator *BO); // assignment
  bool VisitUnaryOperator(UnaryOperator *UO); // ++, --, & operators
  //VisitCastExpr(CastExpr *CE)
  //void VisitDeclRefExpr(DeclRefExpr *DR);
  
  static const unsigned int NO_ID = ClangFlowAnalysis::NO_ID;
  static const unsigned int THIS_ID = ClangFlowAnalysis::THIS_ID;
};

}

bool TransferFunctions::VisitDeclStmt(DeclStmt* DS) {
  for (clang::DeclStmt::decl_iterator I = DS->decl_begin();
       I != DS->decl_end(); ++I) {
    clang::VarDecl* VD = dyn_cast<VarDecl>(*I);
    if (VD) {
      clang::Expr* init_value = VD->getInit(); // initialization expression

      // handle references to a pointer
      if (VD->getType()->isReferenceType()) { // reference ...
        if (VD->getType()->getPointeeType()->isPointerType()) { // to pointer
          // example: Foo*& ref = foo_ptr;
          if (init_value) {
            update(init_value, NO_ID); // kill the aliased pointer variable
          }
          return true; // no need update the reference to a pointer
        }
      }

      // add the new variable; try to copy existing ID from initialization
      unsigned int new_id = lookup_target_obj_lid(init_value);
      if (new_id == NO_ID) {
        new_id = _dfvals.get_next_free_id();
      }
      _dfvals.update_variable(VD, DS, new_id);
    }
  }
  return true;
}

bool TransferFunctions::VisitCallExpr(clang::CallExpr *CE) {
  clang::MemberExpr* target = dyn_cast_or_null<MemberExpr>(CE->getCallee());
  if (target) {
    // dive into the member expression to find the callee object
    _target_obj_lids[CE] = lookup_target_obj_lid(target->getBase());
  }
  else {
    _target_obj_lids[CE] = NO_ID; // lookup failed
  }

  // Variables passed as reference arguments:
  // If a value is passed by non-const reference to a function, we must assume
  // that it can be changed silently. Moreoever, the reference could be stored
  // somewhere and the value could change at arbitrary points in time.
  // Thus, kill that variable:
  for (CallExpr::arg_iterator I = CE->arg_begin(), E = CE->arg_end();
       I != E; ++I) {
    if ( (!(*I)->getType().isConstQualified()) && (*I)->isGLValue() ) {
      // TODO: isGLValue seems to do what it is expected to, but it is correct?
      // see: http://stackoverflow.com/questions/3601602
      update(*I, NO_ID); // kill
    }
  }
  return true;
}

bool TransferFunctions::VisitBinaryOperator(BinaryOperator *BO) {
  if (BO->isAssignmentOp()) {
    unsigned int new_id;
    if (BO->getOpcode() == BO_Assign) {
      // extract rhs' id
      new_id = lookup_target_obj_lid( BO->getRHS() );
      if (new_id == NO_ID) {
        new_id = _dfvals.get_next_free_id();
      }
    }
    else {
      new_id = _dfvals.get_next_free_id();
    }
    update( BO->getLHS(), new_id );
  }
  return true;
}

bool TransferFunctions::VisitUnaryOperator(UnaryOperator* UO) {
  if (UO->isIncrementDecrementOp()) {
    update(UO->getSubExpr(), _dfvals.get_next_free_id()); // assign a new id
  }
  else if (UO->getOpcode() == UO_AddrOf) {
    update(UO->getSubExpr(), NO_ID); // kill
  }
  return true;
}


unsigned int TransferFunctions::lookup_target_obj_lid(clang::Expr* expr) {
  clang::Stmt* stmt = expr;
  while (stmt) {
    stmt = stmt->IgnoreImplicit();
    if( clang::DeclRefExpr* DRF = dyn_cast<DeclRefExpr>(stmt) ) {
      // non-member variable
      clang::VarDecl* VD = dyn_cast_or_null<VarDecl>(DRF->getDecl());
      if (VD == 0) {
        return NO_ID;
      }
      if (!_dfvals.is_tracked(VD)) {
        _dfvals.add_variable(VD);
      }
      return _dfvals.get_id(VD);
    }
    else if ( clang::MemberExpr* ME = dyn_cast<MemberExpr>(stmt) ) {
      // member variable
      if ( is_trackable(ME) ) {
        unsigned int id = _dfvals.get_id(ME);
        if ( id == NO_ID ) {
          return _dfvals.add_member_expr(ME);
        }
        else {
          return id;
        }
      }   
      return NO_ID;
    }
    else if ( dyn_cast<CXXThisExpr>(stmt) ) {
      // this pointer
      return THIS_ID;
    }
    else if ( /*clang::ArraySubscriptExpr* ASE =*/ dyn_cast<ArraySubscriptExpr>(stmt) ) {
      // array access
      //return lookup_or_insert(ASE); //FIXME
      return NO_ID;
    }
    else if ( dyn_cast<CallExpr>(stmt) ) {
      // if the callee is a CallExpr, stop the static analysis,
      // as we cannot guarantee that a call yields always the same object.
      return NO_ID;
    }
    else if ( clang::ConditionalOperator* CO = dyn_cast<ConditionalOperator>(stmt) ) {
      // try constant folding
      bool res;
      if ( CO->getCond()->EvaluateAsBooleanCondition(res, _context) ) {
        // condition result known at compile time: proceed with that
        stmt = res ? CO->getTrueExpr() : CO->getFalseExpr();
      }
      else {
        // condition not evaluable
        return NO_ID;
      }
    }
    else if ( BinaryOperator* BO = dyn_cast<BinaryOperator>(stmt) ) {
      switch(BO->getOpcode()) {
        case clang::BO_Comma: {
          // comma operator: proceed with rhs (lhs is dropped)
          stmt = BO->getRHS();
          break;
        }
        case clang::BO_Assign:
        case clang::BO_MulAssign:
        case clang::BO_DivAssign:
        case clang::BO_RemAssign:
        case clang::BO_AddAssign:
        case clang::BO_SubAssign:
        case clang::BO_ShlAssign:
        case clang::BO_ShrAssign:
        case clang::BO_AndAssign:
        case clang::BO_XorAssign:
        case clang::BO_OrAssign: {
          // some assignment operator: proceed with rhs
          stmt = BO->getRHS();
          break;
        }
        default:
          return NO_ID;
      }
    }
    else if ( UnaryOperator* UO = dyn_cast<UnaryOperator>(stmt) ) {
      if (UO->isIncrementDecrementOp()) {
        return NO_ID; // cannot determine where it will point to
      }
      else {
        stmt = UO->getSubExpr();
      }
    }
    else if ( stmt->child_begin() == stmt->child_end() ) {
      return NO_ID; // no more children to search for
    }
    else {
      if( (++(stmt->child_begin())) != (stmt->child_end()) ) {
        // more than one child found
        return NO_ID;
      }
      
      // proceed with next child
      stmt = *(stmt->child_begin());
    }
  }
  return NO_ID; // lookup failed
}


void TransferFunctions::update(clang::Stmt* stmt, unsigned int new_id) {
  stmt = stmt->IgnoreImplicit();
  
  if( clang::DeclRefExpr* DRF = dyn_cast<DeclRefExpr>(stmt) ) {
    clang::VarDecl* VD = dyn_cast_or_null<VarDecl>(DRF->getDecl());
    _dfvals.update_variable(VD, DRF, new_id);
    // we are only interested in changes of local-storage variables:
  }
  else if ( dyn_cast<MemberExpr>(stmt) ) {
    // no need to update any member expressions, as we only consider constant
    // member expressions
    return;
  }
  else if ( dyn_cast<ArraySubscriptExpr>(stmt) ) {
    //_array_expr_map.clear(); //TODO: do not remove all items
    return;
  }
  else if ( dyn_cast<CallExpr>(stmt) ) {
    // call expressions cannot yield a local-storage pointer whose address was
    // not taken before (and which was thus blacklisted)
    return;
  }
  else if ( clang::ConditionalOperator* CO = dyn_cast<ConditionalOperator>(stmt) ) {
    // try constant folding
    bool res;
    if ( CO->getCond()->EvaluateAsBooleanCondition(res, _context) ) {
      // condition result known at compile time: proceed with that
      update(res ? CO->getTrueExpr() : CO->getFalseExpr(), new_id);
    }
    else {
      // condition not evaluable
      // recurse to both subexpressions
      update(CO->getTrueExpr(), new_id);
      update(CO->getFalseExpr(), new_id);
    }
  }
  else {
    // recurse to all children
    for(Stmt::child_iterator it = stmt->child_begin();
        it != stmt->child_end(); it++) {
      update(*it, new_id);
    }
  }
}


//------------------------------------------------------------------------====//
// ClangBasicBlockMap: Interface to the static analyses
//====------------------------------------------------------------------------//

ClangFlowAnalysis::~ClangFlowAnalysis() {
  //_adcm.clear(); //needed? useful?
  if (_analysis_decl_ctx) {
    delete _analysis_decl_ctx; // drop cache
    _analysis_decl_ctx = 0;
  }
}

void ClangFlowAnalysis::analyze(clang::DeclaratorDecl* func) {
  if ( func && func->hasBody() ) {
    // check if the AnalysisDeclContext cache is valid for the given function
    if ((_analysis_decl_ctx == 0) || (_analysis_decl_ctx->getDecl() != func)){
      // AnalysisDeclContext is invalid => construct a new one
      if (_analysis_decl_ctx) {
        delete _analysis_decl_ctx; // drop cache
      }
      _analysis_decl_ctx = new clang::AnalysisDeclContext(&_adcm, func);
      _target_obj_lids.clear();
      do_target_object_analysis(func); // dataflow analysis
    }
  }
  else {
    // invalid function given => clean up old analysis result
    if (_analysis_decl_ctx) {
      delete _analysis_decl_ctx; // drop cache
    }
    _analysis_decl_ctx = 0;
    _target_obj_lids.clear();
  }
}

bool ClangFlowAnalysis::valid() const {
  return (_analysis_decl_ctx) &&
         (_analysis_decl_ctx->getCFG()) &&
         (_analysis_decl_ctx->getCFGStmtMap());
}

void ClangFlowAnalysis::do_target_object_analysis(clang::DeclaratorDecl* func) {
  if ( !valid() ) {
    return; // CFG construction failed
  }

  clang::CFG* cfg = _analysis_decl_ctx->getCFG();

  DataflowWorklist worklist(*_analysis_decl_ctx->getCFG(), *_analysis_decl_ctx);

  // enqueue (each CFG block):
  for (CFG::const_iterator it = cfg->begin(), ei = cfg->end(); it != ei; ++it) {
    const CFGBlock *block = *it;
    worklist.enqueueBlock(block);
  }
  worklist.sortWorklist();

  DataFlowValues::reset_next_free_id();

  // allocate the to-be-computed DataFlowValues for each basic block
  std::vector<DataFlowValues> perBlockValues; // indexed by CFG-Block IDs
  perBlockValues.resize(cfg->getNumBlockIDs());

  // dequeue:
  while (const CFGBlock *block = worklist.dequeue()) {

    // FIXME: make sure that we've already visited at least one predecessor

    // variable values at the end of this basic block (after applying TF)
    DataFlowValues &vals_end = perBlockValues[block->getBlockID()];

    // variables values at the beginning of this basic block (computed by merge)
    DataFlowValues vals_begin;
    vals_begin.copyHistory(vals_end); // remember merge-conflict resolutions

    // merge the values of all predecessor blocks to compute 'vals_begin'
    for (CFGBlock::const_pred_iterator it = block->pred_begin(), ei = block->pred_end(); it != ei; ++it) {
      if (const CFGBlock *pred = *it) {
        vals_begin.merge(perBlockValues[pred->getBlockID()]);
      }
    }
    
    // if there are no predecessors, this must be the entry block:
    if ( block->pred_empty() ) {
      if (clang::FunctionDecl* func_decl = dyn_cast<FunctionDecl>(func)) {      
        // initialize the entry block with the function parameters
        DeclContext::specific_decl_iterator<ParmVarDecl> I(func_decl->decls_begin()),
                                                         E(func_decl->decls_end());
        for ( ; I != E; ++I) {
          vals_begin.add_variable(*I);
        }
      }
    }

    // call RecursiveASTVisitor and apply the TransferFunctions
    TransferFunctions TF(vals_begin, _target_obj_lids, _analysis_decl_ctx->getASTContext());
    
    for (CFGBlock::const_iterator BI=block->begin(), BE=block->end(); BI != BE; ++BI) {
      if (Optional<CFGStmt> stmt = BI->getAs<CFGStmt>()) {
        TF.TraverseStmt(const_cast<Stmt*>(stmt->getStmt()));
      }
    }

    // did the variable values change (compared to last iteration)?
    if (vals_begin.equals(vals_end)) {
      continue; // fix point reached
    }

    // update the variable values after applying the TF
    vals_end = vals_begin;

    // enqueue the successors, which consume the updated variable values
    worklist.enqueueSuccessors(block);
  }

  /*
  // debug:
  for (CFG::const_reverse_iterator it = cfg->rbegin(), ei = cfg->rend(); it != ei; ++it) {
    const CFGBlock *block = *it;
    std::cout << "================ [" << block->getBlockID() << "] ===================" << std::endl;
    perBlockValues[block->getBlockID()].dump();
    std::cout << "========================================" << std::endl << std::endl;
  }
  */
}

unsigned int ClangFlowAnalysis::get_block_lid(clang::DeclaratorDecl* func,
    clang::Stmt* stmt) {
  
  analyze(func); // do the hard work
  if ( valid() ) {
    // lookup the basic block id for the given stmt
    clang::CFGStmtMap* stmt_map = _analysis_decl_ctx->getCFGStmtMap();
    clang::CFGBlock* cfg_block = stmt_map->getBlock(stmt);
    if (cfg_block) {
      // building a DominatorTree segfaults :-(
      //clang::DominatorTree dt;
      //dt.buildDominatorTree(*_analysis_decl_ctx);
      return cfg_block->getBlockID();
    }
  }
  return 0; // cannot calculate a block id (e.g., no function body found)
}

unsigned int ClangFlowAnalysis::get_target_obj_lid(clang::DeclaratorDecl* func,
    clang::CallExpr* CE) {
  
  analyze(func); // do the hard work
  call_target_obj_lid_map_t::iterator found = _target_obj_lids.find(CE);
  if( found != _target_obj_lids.end() ) {
    return found->second;
  }
  return NO_ID; // lookup failed
}
