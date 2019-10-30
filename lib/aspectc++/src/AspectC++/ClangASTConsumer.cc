// This file is part of the AspectC++ compiler 'ac++'.
// Copyright (C) 1999-2013  The 'ac++' developers (see aspectc.org)
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

#include "ClangASTConsumer.h"
#include "clang/Basic/Version.h"
#include "clang/AST/ASTContext.h"
#include <iostream>
#include "ClangAnnotation.h"

using namespace clang;


bool ClangASTConsumer::isInProject(clang::Decl *D) {
  PresumedLoc PL = _sm->getPresumedLoc(D->getLocation());
  StringRef Name = PL.getFilename();
  StringRef BufferName = _sm->getBufferName(D->getLocation());
  assert(!Name.startswith("<intro") || BufferName.startswith("<intro"));
  return BufferName.startswith("<intro") ||
         (!Name.empty() && _model.get_project().isBelow(Name.str().c_str()));
}

void ClangASTConsumer::Initialize(ASTContext &Context) {
  _sm = &Context.getSourceManager();
  _parent_map[Context.getTranslationUnitDecl()] =
      _model.register_namespace1(0, "::");
}

bool ClangASTConsumer::HandleTopLevelDecl(DeclGroupRef DG) {
  for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; ++i)
    TraverseDecl(*i);

  return true;
}

void ClangASTConsumer::DeclContextStack::updateContextWithCurrentElement( clang::Decl *decl, ACM_Name *element ) {
  assert( decl && element );
  assert( _decl_map.count( decl ) == 0 ); // information should only go in once

  if(!_outer._decl_stack.empty()
      &&  _outer._decl_stack.back().first == decl ) { // only the keep information for elements on stack
                                                      // updates should only occur for recent element
    _decl_map[ decl ] = element;
    if( DeclContext *dc = dyn_cast<DeclContext>( decl ) )
      _outer._parent_map[ dc ] = element;
  }
}

bool ClangASTConsumer::DeclContextStack::isCurrentContextValid() {
  // current decl context does not exist inside of project
  if( _outer._decl_stack.empty() || ! _outer.isInProject( _outer._decl_stack.back().first ) )
    return false;

  return true;
}

ClangASTConsumer::JPContext ClangASTConsumer::DeclContextStack::getJPContext() {
  assert( ! _outer._decl_stack.empty() );
  ClangASTConsumer::JPContext ctx;

  clang::Decl *decl = _outer._decl_stack.back().first;
  ctx.parent_decl = decl;

  if( FunctionDecl *fd = dyn_cast<FunctionDecl>( decl ) )
    ctx.parent = _outer._parent_map[fd];
  else if( RecordDecl *rd = dyn_cast<RecordDecl>( decl ) )
    ctx.parent = _outer._parent_map[rd];
  else if( VarDecl *vd = dyn_cast<VarDecl>( decl ) )
    ctx.parent = _decl_map[vd];
  else // should not happen => abort
    assert( false && "This should not happen: programming error !" );

  ctx.local_id = _outer._last_fn_local_id++;
  return ctx;
}

clang::Decl *ClangASTConsumer::DeclContextStack::getCurrentDecl() {
  assert( ! _outer._decl_stack.empty() );
  return _outer._decl_stack.back().first;
}

bool ClangASTConsumer::VisitType(clang::Type *Ty) {
  // Revisit template instances, there may be new ones we didn't see the first
  // time.
  if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl())
    if (ClassTemplateSpecializationDecl *CTSD =
            dyn_cast<ClassTemplateSpecializationDecl>(RD))
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
      if (!_parent_map.count(CTSD) && _seen_specs.insert(CTSD))
#else // C++ 11 interface
      if (!_parent_map.count(CTSD) && _seen_specs.insert(CTSD).second)
#endif
        return TraverseCXXRecordDecl(CTSD);

  return true;
}

bool ClangASTConsumer::VisitNamespaceDecl(NamespaceDecl *D) {
  ACM_Name *parent = _parent_map[D->getDeclContext()];
  TU_Namespace *nameSp = _model.register_namespace(D, parent);
  _parent_map[D] = nameSp;

  if (nameSp && _model.conf().attributes())
    _model.handle_attributes(nameSp, D);

  return true;
}

bool ClangASTConsumer::VisitCXXRecordDecl(CXXRecordDecl *D) {
  if (!isInProject(D) || D->isLocalClass ())
    return true;

//  D = D->getDefinition();
//  if (!D)
//    return true;
//
  ACM_Name *parent = _parent_map[D->getDeclContext()];

  ACM_Class *jpl_class = _model.register_aspect(D, parent);
  if (!jpl_class)
    jpl_class = _model.register_class(D, parent);

  if (!jpl_class)
    return true;

  _parent_map[D] = jpl_class;

  // for class definitions attribute handling is done by the introducer
  if(_model.conf().attributes() && !D->isThisDeclarationADefinition())
    _model.handle_attributes(jpl_class, D);

  if (D->isThisDeclarationADefinition() && jpl_class->type_val () == JPT_Aspect) {
    ACM_Aspect *jpl_aspect = (ACM_Aspect*)jpl_class;
    //_vm << (is_abstract (*jpl_aspect) ? "Abstract" : "Concrete")
    //  << " aspect " << signature (*jpl_aspect) << endvm;
    //_vm++;
    _model.advice_infos (jpl_aspect);
    //_vm--;
  }

  return true;
}

bool ClangASTConsumer::VisitFunctionDecl(FunctionDecl *D) {
  if (!isInProject(D))
    return true;

  // functions defined in local classes must be ignored
  clang::CXXRecordDecl *cd = clang::dyn_cast<clang::CXXRecordDecl>(D->getDeclContext());
  if (cd && cd->isLocalClass ())
    return true;

  // ignore local function declarations
  if (D->getLexicalParent ()->isFunctionOrMethod ())
    return true;

  // functions that are template instances or members of template instances
  // must be ignored
  // strange: an instance node should not be visited anyway!
  if (D->getInstantiatedFromMemberFunction ())
    return true;

//  cout << "VisitFunctionDecl " << D->getQualifiedNameAsString() << endl;
  ACM_Name *parent = _parent_map[D->getDeclContext()];
  _parent_map[D] = _model.register_pointcut(D, parent);
  if (!_parent_map[D]) {
    if (TU_Function *func = _model.register_function(D, parent)) {
      // There may be a new redeclaration, add it.

      if(_model.conf().attributes())
        _model.handle_attributes(func, D);

      func->add_decl(D);
      _parent_map[D] = func;
    }
  }

  return true;
}

bool ClangASTConsumer::VisitVarDecl( VarDecl *VD ) {
  if( ! isInProject( VD ) )
    return true;

  if( VD->getType()->isReferenceType() && ! VD->isFileVarDecl() )
    return true; // skip references for now, but only if they are not on the top level as we need these as context for joinpoints

#if 0
  const clang::Expr *init_expr = VD->getInit();
  if (init_expr && VD->getInitStyle() == clang::VarDecl::CInit) {
    if (dyn_cast<clang::InitListExpr>(init_expr))
      cout << "initialized var" << VD->getNameAsString () << endl;
  }
#endif

  ACM_Name *parent = _parent_map[VD->getDeclContext()];
  if( parent && parent->type_val() == JPT_Function )
    return true;// skip local variables for now
  if( TU_Variable *var = _model.register_variable( VD, parent ) ) {
    // remember model object, we might need it as context
    _context_stack.updateContextWithCurrentElement( VD, var );

    if (_model.conf().attributes())
      _model.handle_attributes(var, VD);
  }
  return true;
}

bool ClangASTConsumer::VisitFieldDecl( clang::FieldDecl *FD ) {
  if( ! isInProject( FD ) )
    return true;

  if( FD->getType()->isReferenceType() )
    return true; // skip references for now

  ACM_Name *parent = _parent_map[FD->getDeclContext()];
  TU_Variable *var = _model.register_variable( FD, parent );

  if (var && _model.conf().attributes())
    _model.handle_attributes(var, FD);

  return true;
}

bool ClangASTConsumer::VisitDeclRefExpr( clang::DeclRefExpr *RE ) {
  if( ! _model.conf().data_joinpoints() ) // only data joinpoints need this information so far, so supress handling if not required
    return true;

  if( !_expr_stack.considerSubtree() || _expr_stack.empty() || ! _expr_stack.peek().isValidSubExpr() )
    return true; // skip as nobody is interested in the information

  // ignore invalid scope
  if( ! _context_stack.isCurrentContextValid() )
    return true;

  clang::VarDecl *VD = clang::dyn_cast_or_null<clang::VarDecl>( RE->getDecl() );
  clang::FunctionDecl *FD = clang::dyn_cast_or_null<clang::FunctionDecl>( RE->getDecl() );

  if( VD ) {
    if( ! isInProject( VD ) )
      return true;

    bool is_ref = VD->getType()->isReferenceType();

    TU_Variable *var = 0;
    if( ! is_ref ) {
      var = _model.register_variable( VD );
      if( !var )
        return true;
    }

    // push collected information to expr traversal stack to be processed when all informations there
    if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
      _expr_stack.peek().pushEntityRefNode( RE );
      if( ! is_ref )
        _expr_stack.peek().pushEntityInfo( var, VD );
    }
  }
  else if( FD ) {
    // currently we need only the RefNode for some check in call context
    // full handling has to be added later

    // push collected information to expr traversal stack to be processed when all informations there
    if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
      _expr_stack.peek().pushEntityRefNode( RE );
    }
  }

  return true;
}

// find member entities
// further we need to act as context for the base expr traversal
// we dont care for information from it yet,
// but else other data is corrupted
bool ClangASTConsumer::TraverseMemberExpr( clang::MemberExpr *ME ) {
  // only consider evaluated subtrees and additionally: only data joinpoints need this information so far, so supress handling if not required
  if( ! _expr_stack.considerSubtree() || ! _model.conf().data_joinpoints() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseMemberExpr( ME );

  DummyContext ctx( *this );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseMemberExpr( ME );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  // ignore invalid scope
  if( ! _context_stack.isCurrentContextValid() )
    return result;

  clang::DeclaratorDecl *DD = clang::dyn_cast_or_null<clang::DeclaratorDecl>( ME->getMemberDecl() ); // maybe refine because IndirectFieldDecl / EnumConstant
  if( ! DD )
    return result;

  if( llvm::isa<VarDecl>( DD ) || llvm::isa<FieldDecl>( DD ) ) {
    if( ! isInProject( DD ) )
      return result;

    bool is_ref = DD->getType()->isReferenceType();

    TU_Variable *var = 0;
    if( ! is_ref ) {
      var = _model.register_variable( DD );
      if( !var )
        return result;
    }

    // push collected information to expr traversal stack to be processed when all informations are collected
    if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
      _expr_stack.peek().pushEntityRefNode( ME );
      if( ! is_ref )
        _expr_stack.peek().pushEntityInfo( var, DD );
    }
  }
  else if( llvm::isa<FunctionDecl>( DD ) ) {
    // currently we need only the RefNode for some check in call context
    // full handling has to be added later

    // push collected information to expr traversal stack to be processed when all informations there
    if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
      _expr_stack.peek().pushEntityRefNode( ME );
    }
  }

  return result;
}

// on traversing an array access we need to find the var reference in base
bool ClangASTConsumer::TraverseArraySubscriptExpr( clang::ArraySubscriptExpr *ASE ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseArraySubscriptExpr( ASE );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseArraySubscriptExpr( ASE );
    handle_built_in_operator_traverse( ASE );
    return result;
  }

  ArrayAccessContext ctx( *this, ASE );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseArraySubscriptExpr( ASE );
  TU_Builtin *op = handle_built_in_operator_traverse( ASE );

  if( op )
    ctx.updateOperator( op );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  if( ! op ) // return early if the element where not registered
    return result;

  // push info about forwarder onto stack
  if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
    _expr_stack.peek().pushEntityForwardingJP( op );
  }

  return result;
}

// Override Traverse functions to build an expression stack for EntityReference classification
// checking which subExpr we are in, normally we could use TraverseExpr(...) but that does not exists
bool ClangASTConsumer::TraverseStmt( clang::Stmt* S ) {
  clang::Expr *E = 0;
  if( _model.conf().data_joinpoints() && _expr_stack.considerSubtree() && ! _expr_stack.empty() && _expr_stack.peek().isSearching() ) {
    if( ( E = dyn_cast_or_null<clang::Expr>( S ) ) ) {
      _expr_stack.peek().enterSubExpr( E );
    }
  }

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseStmt( S );

  if( E ) {
    assert( ! _expr_stack.empty() );
    _expr_stack.peek().registerJPforSubExpr();
    _expr_stack.peek().leaveSubExpr( E );
  }

  return result;
}

// build the context handlers on the traversal stack as place to collect information about EntityReferences
bool ClangASTConsumer::TraverseImplicitCastExpr( clang::ImplicitCastExpr *ICE ) {
  if( ! _model.conf().data_joinpoints() || ! _expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseImplicitCastExpr( ICE );

  bool result;
  if( ICE->getCastKind() == clang::CK_LValueToRValue ) {
    SingleGetContext ctx( *this, ICE, ICE->getSubExpr() );
    _expr_stack.push( ctx );

    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseImplicitCastExpr( ICE );

    assert( &_expr_stack.peek() == &ctx );
    _expr_stack.drop();
    ctx.registerJPforContext();
  }
  else if( ICE->getCastKind() == clang::CK_ArrayToPointerDecay && ! ( ! _expr_stack.empty() && _expr_stack.peek().suppressDecayJPs() ) ) {
    SingleRefContext ctx( *this, ICE, ICE->getSubExpr() );
    _expr_stack.push( ctx );

    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseImplicitCastExpr( ICE );

    assert( &_expr_stack.peek() == &ctx );
    _expr_stack.drop();
    ctx.registerJPforContext();
  }
  else
    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseImplicitCastExpr( ICE );

  return result;
}

bool ClangASTConsumer::TraverseUnaryAddrOf( clang::UnaryOperator *UO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryAddrOf( UO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryAddrOf( UO );
    handle_built_in_operator_traverse( UO );
    return result;
  }

  SingleRefContext ctx( *this, UO, UO->getSubExpr() );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryAddrOf( UO );
  TU_Builtin *op = handle_built_in_operator_traverse( UO );

  if( op )
    ctx.updateOperator( op );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseReturnStmt( clang::ReturnStmt *RS ) {
  clang::FunctionDecl *FD = llvm::dyn_cast<clang::FunctionDecl>( _context_stack.getCurrentDecl() );
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  if( ! _model.conf().data_joinpoints() || ! _expr_stack.considerSubtree() || ! FD->getResultType().getTypePtr()->isReferenceType() )
#else // C++ 11 interface
  if( ! _model.conf().data_joinpoints() || ! _expr_stack.considerSubtree() || ! FD->getReturnType().getTypePtr()->isReferenceType() )
#endif
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseReturnStmt( RS );

  SingleRefContext ctx( *this, RS->getRetValue(), RS->getRetValue() );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseReturnStmt( RS );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseUnaryDeref( clang::UnaryOperator *UO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryDeref( UO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryDeref( UO );
    handle_built_in_operator_traverse( UO );
    return result;
  }

  DummyContext ctx( *this );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryDeref( UO );
  handle_built_in_operator_traverse( UO );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  // ignore invalid scope
  if( ! _context_stack.isCurrentContextValid() )
    return result;

  // push collected information to expr traversal stack to be processed when all informations are collected
  if( ! _expr_stack.empty() && _expr_stack.peek().isValidSubExpr() ) {
    _expr_stack.peek().pushEntityRefNode( UO );
  }

  return result;
}

bool ClangASTConsumer::TraverseBinComma( clang::BinaryOperator *BO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseBinComma( BO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinComma( BO );
    // disable short-circuit evalaluating operator
    // handle_built_in_operator_traverse( BO );
    return result;
  }

  BinaryForwardContext ctx( *this, BO->getLHS(), BO->getRHS(), BinaryForwardContext::SEI_RHS );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinComma( BO );
// disable left-to-right evalaluating operator
//  handle_built_in_operator_traverse( BO );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  return result;
}

bool ClangASTConsumer::TraverseBinAssign( clang::BinaryOperator *BO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseBinAssign( BO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinAssign( BO );
    handle_built_in_operator_traverse( BO );
    return result;
  }

  AssignmentContext ctx( *this, BO );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinAssign( BO );
  TU_Builtin *op = handle_built_in_operator_traverse( BO );

  if( op )
    ctx.updateOperator( op );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

#if FRONTEND_CLANG < 38
bool ClangASTConsumer::hookTraverseCompoundAssign( FwdCall_CAssign fwd, clang::CompoundAssignOperator *CAO ) {
#else
bool ClangASTConsumer::hookTraverseCompoundAssign( FwdCall_CAssign fwd, clang::CompoundAssignOperator *CAO, DataRecursionQueue *Q ) {
#endif
  if( !_expr_stack.considerSubtree() )
#if FRONTEND_CLANG < 38
    return (this->*fwd)( CAO );
#else
    return (this->*fwd)( CAO, Q );
#endif

  if( ! _model.conf().data_joinpoints() ) {
#if FRONTEND_CLANG < 38
    bool result = (this->*fwd)( CAO );
#else
    bool result = (this->*fwd)( CAO, Q );
#endif
    handle_built_in_operator_traverse( CAO );
    return result;
  }

  AssignmentContext ctx( *this, CAO );
  _expr_stack.push( ctx );

#if FRONTEND_CLANG < 38
  bool result = (this->*fwd)( CAO );
#else
  bool result = (this->*fwd)( CAO, Q );
#endif
  TU_Builtin *op = handle_built_in_operator_traverse( CAO );

  if( op )
    ctx.updateOperator( op );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

#if FRONTEND_CLANG < 38
bool ClangASTConsumer::hookTraversePrePost( FwdCall_PrePost fwd, clang::UnaryOperator *UO ) {
#else
bool ClangASTConsumer::hookTraversePrePost( FwdCall_PrePost fwd, clang::UnaryOperator *UO, DataRecursionQueue *Q ) {
#endif
  if( !_expr_stack.considerSubtree() )
#if FRONTEND_CLANG < 38
    return (this->*fwd)( UO );
#else
    return (this->*fwd)( UO, Q );
#endif

  if( ! _model.conf().data_joinpoints() ) {
#if FRONTEND_CLANG < 38
    bool result = (this->*fwd)( UO );
#else
    bool result = (this->*fwd)( UO, Q );
#endif
    handle_built_in_operator_traverse( UO );
    return result;
  }

  AssignmentContext ctx( *this, UO );
  _expr_stack.push( ctx );

#if FRONTEND_CLANG < 38
  bool result = (this->*fwd)( UO );
#else
  bool result = (this->*fwd)( UO, Q );
#endif
  TU_Builtin *op = handle_built_in_operator_traverse( UO );

  if( op )
    ctx.updateOperator( op );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseConditionalOperator( clang::ConditionalOperator* CO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseConditionalOperator( CO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseConditionalOperator( CO );
    handle_built_in_operator_traverse( CO );
    return result;
  }

  DummyContext ctx( *this );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseConditionalOperator( CO );

  handle_built_in_operator_traverse( CO );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  return result;
}

bool ClangASTConsumer::TraverseBinPtrMemD( clang::BinaryOperator *BO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemD( BO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemD( BO );
    handle_built_in_operator_traverse( BO );
    return result;
  }

  DummyContext ctx( *this );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemD( BO );
  handle_built_in_operator_traverse( BO );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  return result;
}

bool ClangASTConsumer::TraverseBinPtrMemI( clang::BinaryOperator *BO ) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemI( BO );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemI( BO );
    handle_built_in_operator_traverse( BO );
    return result;
  }

  DummyContext ctx( *this );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseBinPtrMemI( BO );
  handle_built_in_operator_traverse( BO );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();

  return result;
}

bool ClangASTConsumer::TraverseUnaryExprOrTypeTraitExpr( clang::UnaryExprOrTypeTraitExpr *subtree ) {
  _expr_stack.beginIgnoreTree();

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseUnaryExprOrTypeTraitExpr( subtree );

  _expr_stack.endIgnoreTree();

  return result;
}

// enable dummy mode if ParmVar is assosiated with already handled function tree
bool ClangASTConsumer::TraverseParmVarDecl( clang::ParmVarDecl *VD ) {
  // check if the current (function-)declaration is a redeclaration
  // if so we enable dummymode as we have already processed the default argument tree
  bool enable_DM = ! _dummy_mode && ( ! _decl_stack.empty() && _decl_stack.back().first->getPreviousDecl() != 0 );

  if( enable_DM )
    _dummy_mode = true;

  bool result;
  if( _model.conf().data_joinpoints() && _expr_stack.considerSubtree() && VD->hasInit() && VD->getType().getTypePtr()->isReferenceType() ) {
    SingleRefContext ctx( *this, VD->getDefaultArg(), VD->getDefaultArg() );
    ctx.ignoreTypeInfo( VD->getTypeSourceInfo() );
    _expr_stack.push( ctx );

    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseParmVarDecl( VD );

    assert( &_expr_stack.peek() == &ctx );
    _expr_stack.drop();
    ctx.registerJPforContext();
  }
  else {
    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseParmVarDecl( VD );
  }

  if( enable_DM ) // disable dummymode again if we enabled it
    _dummy_mode = false;

  return result;
}

// TraverseDeclaratorHelper is private, so we need to catch the cases here
bool ClangASTConsumer::TraverseTypeLoc( clang::TypeLoc TL ) {
  // check if we have to ignore the whole tree as parts of type references are off limits for weaving
  bool ignore = _expr_stack.considerSubtree() && ! _expr_stack.empty() && _expr_stack.peek().shouldIgnoreTypeInfo( TL );
  if( ignore )
    _expr_stack.beginIgnoreTree();

  bool result = RecursiveASTVisitor::TraverseTypeLoc( TL );

  if( ignore )
    _expr_stack.endIgnoreTree();

  return result;
}

// Override traverse functions so we can build up a stack of scopes.
bool ClangASTConsumer::TraverseVarDecl( clang::VarDecl *VD ) {
  // Only remember the scope for stand alone vardecls.
  if( VD->isFileVarDecl() )
    _decl_stack.push_back( std::make_pair( VD, _last_fn_local_id ) );

  bool result;
  if( _model.conf().data_joinpoints() && _expr_stack.considerSubtree() && VD->hasInit() && VD->getType().getTypePtr()->isReferenceType() ) {
    SingleRefContext ctx( *this, VD->getInit(), VD->getInit() );
    ctx.ignoreTypeInfo( VD->getTypeSourceInfo() );
    _expr_stack.push( ctx );

    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseVarDecl( VD );

    assert( &_expr_stack.peek() == &ctx );
    _expr_stack.drop();
    ctx.registerJPforContext();
  }
  else {
    result = RecursiveASTVisitor<ClangASTConsumer>::TraverseVarDecl( VD );
  }

  if( VD->isFileVarDecl() )
    _decl_stack.pop_back();
  return result;
}

bool ClangASTConsumer::TraverseFunctionDecl(clang::FunctionDecl *FD) {
  _decl_stack.push_back(std::make_pair(FD, _last_fn_local_id));
  _last_fn_local_id = 0;
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseFunctionDecl(FD);
  _last_fn_local_id = _decl_stack.pop_back_val().second;
  return result;
}

bool ClangASTConsumer::TraverseCXXMethodDecl(clang::CXXMethodDecl *FD) {
  _decl_stack.push_back(std::make_pair(FD, _last_fn_local_id));
  _last_fn_local_id = 0;
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXMethodDecl(FD);
  _last_fn_local_id = _decl_stack.pop_back_val().second;
  return result;
}

bool ClangASTConsumer::TraverseCXXConstructorDecl(clang::CXXConstructorDecl *FD) {
  _decl_stack.push_back(std::make_pair(FD, _last_fn_local_id));
  _last_fn_local_id = 0;
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXConstructorDecl(FD);
  _last_fn_local_id = _decl_stack.pop_back_val().second;
  return result;
}

bool ClangASTConsumer::TraverseCXXConversionDecl(clang::CXXConversionDecl *FD) {
  _decl_stack.push_back(std::make_pair(FD, _last_fn_local_id));
  _last_fn_local_id = 0;
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXConversionDecl(FD);
  _last_fn_local_id = _decl_stack.pop_back_val().second;
  return result;
}

bool ClangASTConsumer::TraverseCXXDestructorDecl(clang::CXXDestructorDecl *FD) {
  _decl_stack.push_back(std::make_pair(FD, _last_fn_local_id));
  _last_fn_local_id = 0;
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXDestructorDecl(FD);
  _last_fn_local_id = _decl_stack.pop_back_val().second;
  return result;
}

bool ClangASTConsumer::TraverseRecordDecl(clang::RecordDecl *RD) {
  _decl_stack.push_back(std::make_pair(RD, _last_fn_local_id));
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseRecordDecl(RD);
  _decl_stack.pop_back();
  return result;
}

bool ClangASTConsumer::TraverseCXXRecordDecl(clang::CXXRecordDecl *RD) {
  _decl_stack.push_back(std::make_pair(RD, _last_fn_local_id));
  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXRecordDecl(RD);
  _decl_stack.pop_back();
  return result;
}

// defining Traverse... instead of VisitCallExpr guarantees that calls nested
// in argument lists, e.g. foo(nested()) are registered first.
bool ClangASTConsumer::TraverseCallExpr(clang::CallExpr *CE) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseCallExpr( CE );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCallExpr(CE);
    if (CE->getCallee()->IgnoreParenImpCasts()->getType()->isFunctionPointerType())
      handle_built_in_operator_traverse(CE); // handle as built-in dereference
    RegisterCallExpr(CE);
    return result;
  }

  CallContext ctx( *this, CE );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCallExpr(CE);
  if (CE->getCallee()->IgnoreParenImpCasts()->getType()->isFunctionPointerType())
    handle_built_in_operator_traverse(CE); // handle as built-in dereference
  RegisterCallExpr(CE);

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr *CE) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXMemberCallExpr( CE );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXMemberCallExpr(CE);
    RegisterCallExpr(CE);
    return result;
  }

  CallContext ctx( *this, CE );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXMemberCallExpr(CE);
  RegisterCallExpr(CE);

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr *CE) {
  if( !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXOperatorCallExpr( CE );

  if( ! _model.conf().data_joinpoints() ) {
    bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXOperatorCallExpr(CE);
    RegisterCallExpr(CE);
    return result;
  }

  CallContext ctx( *this, CE );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXOperatorCallExpr(CE);
  RegisterCallExpr(CE);

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

bool ClangASTConsumer::TraverseCXXTemporaryObjectExpr( clang::CXXTemporaryObjectExpr *TOE ) {
  if( ! _model.conf().data_joinpoints() || !_expr_stack.considerSubtree() )
    return RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXTemporaryObjectExpr( TOE );

  CallContext ctx( *this, TOE );
  _expr_stack.push( ctx );

  bool result = RecursiveASTVisitor<ClangASTConsumer>::TraverseCXXTemporaryObjectExpr( TOE );

  assert( &_expr_stack.peek() == &ctx );
  _expr_stack.drop();
  ctx.registerJPforContext();

  return result;
}

void ClangASTConsumer::RegisterCallExpr(CallExpr *CE) {
  // Ignore CallExprs in (inline) functions outside of the project.
  if (_decl_stack.empty() || !isInProject(_decl_stack.back().first))
    return;

  clang::FunctionDecl *Callee = dyn_cast_or_null<FunctionDecl>(CE->getCalleeDecl());
  if (Callee) {
    // Ignore explicit destructor calls
    // TODO: Is that really what the user wants?
    if (dyn_cast<CXXDestructorDecl> (Callee))
      return;

    // Ignore calls to functions with a local forward decl only.
    bool only_local_decls = true;
    for (clang::FunctionDecl::redecl_iterator ri = Callee->redecls_begin(),
                                              re = Callee->redecls_end();
         ri != re; ++ri) {
      if (!ri->getLexicalParent()->isFunctionOrMethod())
        only_local_decls = false;
    }
    if (only_local_decls)
      return;
  }

  if( ! _dummy_mode ) {
    DeclaratorDecl *DD = dyn_cast<DeclaratorDecl>(_decl_stack.back().first);
    if (DD) // ignore calls without proper context, e.g. typedef decltype(somecall()) T;
      _model.register_call(Callee, CE, DD, _last_fn_local_id++);
  }
  else
    _last_fn_local_id++;
}

// This method handles a traverse over a built-in operator.
TU_Builtin * ClangASTConsumer::handle_built_in_operator_traverse(Expr* built_in_operator) {
  // Is the current scope valid?
  if( _decl_stack.empty() || ! isInProject(_decl_stack.back().first) )
    return 0;

  // Try to cast the clang::Decl to a clang::DeclaratorDecl ...
  clang::DeclaratorDecl* lexical_parent_decl = dyn_cast<DeclaratorDecl>(_decl_stack.back().first);
  // ... and only consider the declaration, if it came out of a declarator:
  if( ! lexical_parent_decl )
    return 0;

  // Registers the built-in-operator-call in the AspectC++-model
  if( ! _dummy_mode )
    return _model.register_builtin_operator_call(built_in_operator, lexical_parent_decl, _last_fn_local_id++);
  else {
    _last_fn_local_id++;
    return 0;
  }
}

void ClangASTConsumer::SingleAccessContext::enterSubExpr( clang::Expr *sub ) {
  assert( sub == _child );
  assert( getMode() == Mode_Search );

  setCurSubExprIndex( 0 );
}

void ClangASTConsumer::SingleAccessContext::leaveSubExpr( clang::Expr *sub ) {
  assert( sub == _child );
  assert( getCurSubExprIndex() == 0 );

  setMode( Mode_Done );
}

void ClangASTConsumer::SingleAccessContext::pushEntityRefNode( clang::Expr *RE ) {
  assert( isValidSubExpr() );

  // null values are not allowed, skip calling in that case
  assert( RE );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_info.ref_node );

  _var_info.ref_node = RE;
}

void ClangASTConsumer::SingleAccessContext::pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *VD ) {
  assert( isValidSubExpr() );

  // null values are not allowed, skip calling in that case
  assert( var );
  assert( VD );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_info.element );
  assert( ! _var_info.decl );

  _var_info.element = var;
  _var_info.decl = VD;
}

void ClangASTConsumer::SingleAccessContext::pushEntityForwardingJP( TU_Builtin *JP ) {
  assert( isValidSubExpr() );

  // null values are not allowed, skip calling in that case
  assert( JP );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_src );

  _var_src = JP;
}

void ClangASTConsumer::SingleGetContext::registerJPforContext() {
  if( _var_info.ref_node ) { // check if we found some reference ...
    ClangModelBuilder::JoinpointContext ctx = _consumer._context_stack.getJPContext();
    if( ! _consumer._dummy_mode )
      _consumer._model.register_get( _var_info, ctx, _var_src ); // ... and register
  }
}

void ClangASTConsumer::SingleRefContext::registerJPforContext() {
  if( _var_info.element && _var_info.decl && _var_info.ref_node ) { // check if we found some reference ...
    ClangModelBuilder::JoinpointContext ctx = _consumer._context_stack.getJPContext();
    if( _op_call ) // on implicit joinpoints
      ctx.parent = _op_call; // change parent element to call node

    if( ! _consumer._dummy_mode )
      _consumer._model.register_ref( _var_info, ctx, _op_call ? _op_call : _var_src ); // ... and register
  }
}

void ClangASTConsumer::SingleRefContext::updateOperator( TU_Builtin *op ) {
  _op_call = op; // remember for implicit joinpoints

  op->forwarded_src( _var_src ); // remeber the dependency chain
}

void ClangASTConsumer::BinaryClassificationContext::enterSubExpr( clang::Expr *sub ) {
  assert( sub == _lhs || sub == _rhs );

  if( isSearching() ) {
    if( sub == _lhs )
      setCurSubExprIndex( SEI_LHS );
    else if( sub == _rhs )
      setCurSubExprIndex( SEI_RHS );
  }
}

void ClangASTConsumer::BinaryClassificationContext::leaveSubExpr( clang::Expr *sub ) {
  assert( getMode() == Mode_Done || ( sub == _lhs && getCurSubExprIndex() == SEI_LHS ) || ( sub == _rhs && getCurSubExprIndex() == SEI_RHS ) );

  setMode( Mode_Search );
}

void ClangASTConsumer::BinaryForwardContext::enterSubExpr( clang::Expr *sub ) {
  if( getParent() == 0 )
    setMode( Mode_Done ); // with nothing to forward to, we're done already
  else
    BinaryClassificationContext::enterSubExpr( sub );
}

void ClangASTConsumer::BinaryForwardContext::leaveSubExpr( clang::Expr *sub ) {
  bool done = ( getCurSubExprIndex() == _forward );

  BinaryClassificationContext::leaveSubExpr( sub );

  if( done )
    setMode( Mode_Done );
}

void ClangASTConsumer::BinaryForwardContext::pushEntityRefNode( clang::Expr *RE ) {
  // if already done just ignore new data
  if( getMode() == Mode_Done )
    return;

  assert( isValidSubExpr() );

  if( getCurSubExprIndex() == _forward ) { // only forward data from chosen subexpr
    getParent()->pushEntityRefNode( RE );
  }
}

void ClangASTConsumer::BinaryForwardContext::pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *VD ) {
  // if already done just ignore new data
  if( getMode() == Mode_Done )
    return;

  assert( isValidSubExpr() );

  if( getCurSubExprIndex() == _forward ) { // only forward data from chosen subexpr
    getParent()->pushEntityInfo( var, VD );
  }
}

void ClangASTConsumer::BinaryForwardContext::pushEntityForwardingJP( TU_Builtin *JP ) {
  // if already done just ignore new data
  if( getMode() == Mode_Done )
    return;

  assert( isValidSubExpr() );

  if( getCurSubExprIndex() == _forward ) { // only forward data from chosen subexpr
    getParent()->pushEntityForwardingJP( JP );
  }
}

void ClangASTConsumer::ArrayAccessContext::pushEntityForwardingJP( TU_Builtin *JP ) {
  // if already done just ignore new data
  if( getMode() == Mode_Done )
    return;

  assert( isValidSubExpr() );

  assert( ! _var_src );
  assert( JP );

  if( getCurSubExprIndex() == SEI_LHS ) { // only remember data from base expr
    _var_src = JP;
  }
}

void ClangASTConsumer::ArrayAccessContext::updateOperator( TU_Builtin *op ) {
  op->forwarded_src( _var_src ); // remeber the dependency chain
}

void ClangASTConsumer::AssignmentContext::leaveSubExpr( clang::Expr *sub ) {
  bool done = ( getCurSubExprIndex() == SEI_LHS );

  BinaryClassificationContext::leaveSubExpr( sub );

  if( done )
    setMode( Mode_Done );
}

void ClangASTConsumer::AssignmentContext::pushEntityRefNode( clang::Expr *RE ) {
  // we only collect data for the assigned value
  if( getCurSubExprIndex() != SEI_LHS )
    return;

  // null values are not allowed, skip calling in that case
  assert( RE );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_info.ref_node );

  _var_info.ref_node = RE;
}

void ClangASTConsumer::AssignmentContext::pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *VD ) {
  // we only collect data for the assigned value
  if( getCurSubExprIndex() != SEI_LHS )
    return;

  // null values are not allowed, skip calling in that case
  assert( var );
  assert( VD );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_info.element );
  assert( ! _var_info.decl );

  _var_info.element = var;
  _var_info.decl = VD;
}

void ClangASTConsumer::AssignmentContext::pushEntityForwardingJP( TU_Builtin *JP ) {
  // we only collect data for the assigned value
  if( getCurSubExprIndex() != SEI_LHS )
    return;

  // null values are not allowed, skip calling in that case
  assert( JP );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_src );

  _var_src = JP;
}

void ClangASTConsumer::AssignmentContext::registerJPforContext() {
  if( _var_info.ref_node ) { // check if we found some reference ...
    ClangModelBuilder::JoinpointContext ctx = _consumer._context_stack.getJPContext();
    if( _op_call ) // on implicit joinpoints
      ctx.parent = _op_call; // change parent element to call node

    // ... check for compound assignment and add a get joinpoint before ...
    if( _is_compound ) {
      if( ! _consumer._dummy_mode )
        _consumer._model.register_get( _var_info, ctx, _op_call );

      // create new context (old one can't be reused
      ctx = _consumer._context_stack.getJPContext();
      if( _op_call ) // on implicit joinpoints
        ctx.parent = _op_call; // change parent element to call node
    }

    if( ! _consumer._dummy_mode )
      _consumer._model.register_set( _var_info, ctx, _op_call ); // ... and register
  }

  // forward findings down the stack, as other joinpoints need the info too
  if( getParent() != 0 ) { // seems we have somebody interested in the information
    if( _var_info.ref_node )
      getParent()->pushEntityRefNode( _var_info.ref_node );
    if( _var_info.decl )
      getParent()->pushEntityInfo( _var_info.element, _var_info.decl );
    if( _op_call )
      getParent()->pushEntityForwardingJP( _op_call );
  }
}

void ClangASTConsumer::AssignmentContext::updateOperator( TU_Builtin *op ) {
  _op_call = op; // remember for implicit joinpoints

  op->forwarded_src( _var_src ); // remeber the dependency chain
}

void ClangASTConsumer::CallContext::enterSubExpr( clang::Expr *sub ) {
  if( isSearching() ) {
    if( _next_arg == 0 ) {
      if( _call->getCallee() == sub )
        setCurSubExprIndex( 0 );
    }
    else {
      unsigned int a;
      for( a = _next_arg - 1; a < getNumArgs(); a++ ) {
        if( getArg( a ) == sub )
          setCurSubExprIndex( a + 1 );
      }
      if( a >= getNumArgs() && _next_arg <= getNumArgs() )
        for( a = 0; a < _next_arg - 1; a++ ) {
          if( getArg( a ) == sub )
            setCurSubExprIndex( a + 1 );
        }

      _curarg_is_ref = false;
      if( _func_decl && ( a < _func_decl->getNumParams() ) )
        _curarg_is_ref = _func_decl->getParamDecl( a )->getType().getTypePtr()->isReferenceType();
      _var_info.tree_node = sub;
    }
  }
}

void ClangASTConsumer::CallContext::leaveSubExpr( clang::Expr *sub ) {
  _next_arg++;
  setMode( Mode_Search );
  clearVarInfo();
}

void ClangASTConsumer::CallContext::pushEntityRefNode( clang::Expr *RE ) {
  assert( getCurSubExprIndex() >= 0 );

  // null values are not allowed, skip calling in that case
  assert( RE );

  // check if we see the callee or an arg
  if( getCurSubExprIndex() == 0 ) {
    if( clang::DeclRefExpr *dre = llvm::dyn_cast<clang::DeclRefExpr>( RE ) )
      _func_decl = llvm::dyn_cast<clang::FunctionDecl>( dre->getDecl() ); // remember callee decl for type checking
    else if( clang::MemberExpr *me = llvm::dyn_cast<clang::MemberExpr>( RE ) )
      _func_decl = llvm::dyn_cast<clang::FunctionDecl>( me->getMemberDecl() ); // remember callee decl for type checking
  }
  else {
    if( _curarg_is_ref ) {
      // double calling is not allowed, it indicates some error in the AST traversal
      assert( ! _var_info.ref_node );

      _var_info.ref_node = RE;
    }
  }
}

void ClangASTConsumer::CallContext::pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD ) {
  assert( getCurSubExprIndex() >= 0 );

  // we only collect data for ref args
  if( getCurSubExprIndex() == 0 || ! _curarg_is_ref )
    return;

  // null values are not allowed, skip calling in that case
  assert( var );
  assert( DD );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_info.element );
  assert( ! _var_info.decl );

  _var_info.element = var;
  _var_info.decl = DD;
}

void ClangASTConsumer::CallContext::pushEntityForwardingJP( TU_Builtin *JP ) {
  assert( getCurSubExprIndex() >= 0 );

  // we only collect data for ref args
  if( getCurSubExprIndex() == 0 || ! _curarg_is_ref )
    return;

  // null values are not allowed, skip calling in that case
  assert( JP );

  // double calling is not allowed, it indicates some error in the AST traversal
  assert( ! _var_src );

  _var_src = JP;
}

void ClangASTConsumer::CallContext::registerJPforContext() {
  // no joinpoint registration yet

  // ignore invalid scope
  if( ! _consumer._context_stack.isCurrentContextValid() )
    return;

  assert( _call || _constr );
  // only pushing info to parent
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
  if( getParent() && getParent()->isValidSubExpr() && _func_decl && _func_decl->getResultType().getTypePtr()->isReferenceType() )
#else // C++ 11 interface
  if( getParent() && getParent()->isValidSubExpr() && _func_decl && _func_decl->getReturnType().getTypePtr()->isReferenceType() )
#endif
    getParent()->pushEntityRefNode( _call ? llvm::dyn_cast<clang::Expr>( _call ) : llvm::dyn_cast<clang::Expr>(_constr) );
}

void ClangASTConsumer::CallContext::registerJPforSubExpr() {
  if( _var_info.element && _var_info.decl && _var_info.ref_node && _curarg_is_ref ) { // check if we found some reference ...
    ClangModelBuilder::JoinpointContext ctx = _consumer._context_stack.getJPContext();

    if( ! _consumer._dummy_mode )
      _consumer._model.register_ref( _var_info, ctx, _var_src ); // ... and register
  }
}
