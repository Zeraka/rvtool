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

#ifndef __ClangASTConsumer_h__
#define __ClangASTConsumer_h__

#include "ACConfig.h"

#include "ModelBuilder.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "ClangAnnotation.h"

class ClangASTConsumer : public clang::ASTConsumer,
                         public clang::RecursiveASTVisitor<ClangASTConsumer> {
  ModelBuilder &_model;
  clang::SourceManager *_sm;

  llvm::DenseMap<const clang::DeclContext *, ACM_Name *> _parent_map;
  llvm::SmallPtrSet<clang::ClassTemplateSpecializationDecl *, 4> _seen_specs;
  llvm::SmallVector<std::pair<clang::Decl *, unsigned>, 4> _decl_stack;
  unsigned _last_fn_local_id;

  bool _dummy_mode; // if enabled, do anything but register_* calls to model

  // shortening the type name
  typedef ClangModelBuilder::JoinpointContext JPContext;
  // wrapper around the existing context stack
  // later we hold the information in this object
  class DeclContextStack {
    ClangASTConsumer &_outer;

    llvm::DenseMap<const clang::Decl *, ACM_Name *> _decl_map;
  public:
    DeclContextStack( ClangASTConsumer &o )
      : _outer ( o ) {};

    void updateContextWithCurrentElement( clang::Decl *decl, ACM_Name *element );

    bool isCurrentContextValid();
    JPContext getJPContext(); // don't reuse for multiple joinpoints, as it contains a local_id
    clang::Decl *getCurrentDecl();
  } _context_stack;

  // objects of this class contain the state when searching for data joinpoints
  class ExprClassificationContext {
    ExprClassificationContext *_parent;
    int _cur_subexpr_index; // either positive index or negative mode
    clang::TypeSourceInfo *_ignoredTypeInfo; // we need to ignore some trees below type decl, but not the whole decl tree
  protected:
    ClangASTConsumer &_consumer;
    enum Mode {
      Mode_Search = -1,
      Mode_Done = -2
    };

    inline int getCurSubExprIndex() { return _cur_subexpr_index; };
    inline void setCurSubExprIndex( int i ) { assert( i >= 0 ); _cur_subexpr_index = i; };
    inline int getMode() { return _cur_subexpr_index; };
    inline void setMode( int m ) { assert( m < 0 ); _cur_subexpr_index = m; };
  public:
    ExprClassificationContext( ClangASTConsumer &con ) : _parent( 0 ), _cur_subexpr_index( Mode_Search ), _ignoredTypeInfo( 0 ), _consumer( con ) {};
    virtual ~ExprClassificationContext() {};

    inline void setParent( ExprClassificationContext *parent ) { assert( _parent == 0 ); _parent = parent; };
    inline ExprClassificationContext *getParent() { return _parent; };

    inline void ignoreTypeInfo( clang::TypeSourceInfo* tsi ) { assert( _ignoredTypeInfo == 0 ); _ignoredTypeInfo = tsi; }
    inline void unignoreTypeInfo( clang::TypeSourceInfo* tsi ) { assert( _ignoredTypeInfo == tsi ); _ignoredTypeInfo = 0; }
    inline bool shouldIgnoreTypeInfo( clang::TypeLoc &TL ) {
      if( _ignoredTypeInfo == 0 ) return false;
      else return _ignoredTypeInfo->getTypeLoc() == TL;
    }

    inline bool isSearching() { return ( getMode() == Mode_Search ); };
    inline bool isValidSubExpr() { return ( getCurSubExprIndex() >= 0 ); };

    virtual void enterSubExpr( clang::Expr *sub ) = 0;
    virtual void leaveSubExpr( clang::Expr *sub ) = 0;

    virtual void pushEntityRefNode( clang::Expr *RE ) = 0;
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD ) = 0;
    virtual void pushEntityForwardingJP( TU_Builtin *JP ) = 0;

    virtual void registerJPforContext() {};
    virtual void registerJPforSubExpr() {};

    virtual bool suppressDecayJPs() { return false; };
  };

  // wrapper class around the stack integrated into the elements
  class ExprContextStack {
    ExprClassificationContext *_top;

    unsigned int _ignoreTreeGuard;
  public:
    ExprContextStack() : _top( 0 ), _ignoreTreeGuard( 0 ) {};

    inline void beginIgnoreTree() { _ignoreTreeGuard++; };
    inline void endIgnoreTree() { _ignoreTreeGuard--; };

    inline bool empty() { return ( _top == 0 ); };
    inline void clear() { _top = 0; _ignoreTreeGuard = 0; };
    inline ExprClassificationContext &peek() { assert( ! empty() ); return *_top; };

    inline bool considerSubtree() { return ( _ignoreTreeGuard == 0 ); };
    inline void push( ExprClassificationContext &e ) { assert( considerSubtree() ); e.setParent( _top ); _top = &e; };

    inline void drop() { _top = _top->getParent(); };
    inline ExprClassificationContext &pop() { ExprClassificationContext &res = peek(); drop(); return res; };
  } _expr_stack;

  bool isInProject(clang::Decl *D);
public:
  ClangASTConsumer(ModelBuilder &model)
    : _model( model ), _last_fn_local_id( 0 ), _dummy_mode( false ), _context_stack( *this ) {}

  virtual void Initialize(clang::ASTContext &Context);
  virtual bool HandleTopLevelDecl(clang::DeclGroupRef DG);

  // handlers for simple model node creation
  bool VisitType(clang::Type *Ty);
  bool VisitNamespaceDecl(clang::NamespaceDecl *D);
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);
  bool VisitFunctionDecl(clang::FunctionDecl *D);
  bool VisitVarDecl(clang::VarDecl *VD);
  bool VisitFieldDecl( clang::FieldDecl *FD );

  // handlers for access joinpoint node creation

  // recognizing the referenced entities
  bool VisitDeclRefExpr(clang::DeclRefExpr *RE); // find referenced entities
  bool TraverseMemberExpr(clang::MemberExpr *ME); // find references to member entities, additional handle traversesal of base expr
  bool TraverseArraySubscriptExpr(clang::ArraySubscriptExpr *ASE); // we need to forward data from base and remember info about base itself

  // subexpr determination
  bool TraverseStmt(clang::Stmt* S); // needed to determinate current subexpr

  // context handling
  bool TraverseImplicitCastExpr(clang::ImplicitCastExpr *ICE); // simple Get
  bool TraverseUnaryAddrOf(clang::UnaryOperator *UO); // simple Ref
  bool TraverseReturnStmt(clang::ReturnStmt *RS); // maybe an Ref depending on expr and return type

  bool TraverseUnaryDeref(clang::UnaryOperator *UO); // pass the reference info
  bool TraverseBinComma(clang::BinaryOperator *BO); // forward the RHS

  // handling of assign nodes
  bool TraverseBinAssign(clang::BinaryOperator *BO); // set on LHS

  // we work with hook functions for the compound and increment/decrement as they are handled similar in each case
private:
#if FRONTEND_CLANG < 38
  typedef bool (clang::RecursiveASTVisitor<ClangASTConsumer>::* const FwdCall_CAssign)(clang::CompoundAssignOperator *CAO);
  bool hookTraverseCompoundAssign( FwdCall_CAssign fwd, clang::CompoundAssignOperator *CAO );
  typedef bool (clang::RecursiveASTVisitor<ClangASTConsumer>::* const FwdCall_PrePost)(clang::UnaryOperator *UO);
  bool hookTraversePrePost( FwdCall_PrePost fwd, clang::UnaryOperator *UO );
public:
#define GEN_CompoundAssignTraverse(NAME) \
  bool TraverseBin##NAME##Assign(clang::CompoundAssignOperator *CAO) { \
    return hookTraverseCompoundAssign( &clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseBin##NAME##Assign, CAO ); \
  };
#else
  typedef bool (clang::RecursiveASTVisitor<ClangASTConsumer>::* const FwdCall_CAssign)(clang::CompoundAssignOperator *CAO, DataRecursionQueue *Q);
  bool hookTraverseCompoundAssign( FwdCall_CAssign fwd, clang::CompoundAssignOperator *CAO, DataRecursionQueue *Q );
  typedef bool (clang::RecursiveASTVisitor<ClangASTConsumer>::* const FwdCall_PrePost)(clang::UnaryOperator *UO, DataRecursionQueue *Q);
  bool hookTraversePrePost( FwdCall_PrePost fwd, clang::UnaryOperator *UO, DataRecursionQueue *Q );
public:
#define GEN_CompoundAssignTraverse(NAME) \
  bool TraverseBin##NAME##Assign(clang::CompoundAssignOperator *CAO, DataRecursionQueue *Q = nullptr) { \
    return hookTraverseCompoundAssign( &clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseBin##NAME##Assign, CAO, Q ); \
  };
#endif
  GEN_CompoundAssignTraverse( Mul );
  GEN_CompoundAssignTraverse( Div );
  GEN_CompoundAssignTraverse( Rem );
  GEN_CompoundAssignTraverse( Add );
  GEN_CompoundAssignTraverse( Sub );
  GEN_CompoundAssignTraverse( Shl );
  GEN_CompoundAssignTraverse( Shr );
  GEN_CompoundAssignTraverse( And );
  GEN_CompoundAssignTraverse( Or );
  GEN_CompoundAssignTraverse( Xor );
#undef GEN_CompoundAssignTraverse
#if FRONTEND_CLANG < 38
#define GEN_PrePostTraverse(NAME) \
  bool TraverseUnary##NAME(clang::UnaryOperator *UO) { \
    return hookTraversePrePost( &clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseUnary##NAME, UO ); \
  };
#else
#define GEN_PrePostTraverse(NAME) \
  bool TraverseUnary##NAME(clang::UnaryOperator *UO, DataRecursionQueue *Q = nullptr) { \
    return hookTraversePrePost( &clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseUnary##NAME, UO, Q ); \
  };
#endif
  GEN_PrePostTraverse( PostInc );
  GEN_PrePostTraverse( PostDec );
  GEN_PrePostTraverse( PreInc );
  GEN_PrePostTraverse( PreDec );
#undef GEN_PrePostTraverse

  // dummy handlers to block off wrong search path
  //   most will be replaced later on with real handlers for the cases
  //   but the will still define borders for the DeclRef search

  // this is particulary difficult
  bool TraverseConditionalOperator(clang::ConditionalOperator *CO);

  bool TraverseBinPtrMemD(clang::BinaryOperator *BO);
  bool TraverseBinPtrMemI(clang::BinaryOperator *BO);

  // ignore this subtrees completely they are never evaluated
  bool TraverseUnaryExprOrTypeTraitExpr(clang::UnaryExprOrTypeTraitExpr *subtree);

  // building decl stack
  bool TraverseVarDecl(clang::VarDecl *VD);
  bool TraverseFunctionDecl(clang::FunctionDecl *FD);
  bool TraverseCXXMethodDecl(clang::CXXMethodDecl *FD);
  bool TraverseCXXConstructorDecl(clang::CXXConstructorDecl *FD);
  bool TraverseCXXConversionDecl(clang::CXXConversionDecl *FD);
  bool TraverseCXXDestructorDecl(clang::CXXDestructorDecl *FD);
  bool TraverseRecordDecl(clang::RecordDecl *RD);
  bool TraverseCXXRecordDecl(clang::CXXRecordDecl *RD);

  // enable dummy mode if ParmVar is assosiated with already handled function tree
  bool TraverseParmVarDecl( clang::ParmVarDecl *VD );

  // ignore exprs that are part of a type decl
  bool TraverseTypeLoc( clang::TypeLoc TL );

  // handling call joinpoints
  void RegisterCallExpr(clang::CallExpr *CE);
  bool TraverseCallExpr(clang::CallExpr *CE);
  bool TraverseCXXMemberCallExpr(clang::CXXMemberCallExpr *CE);
  bool TraverseCXXOperatorCallExpr(clang::CXXOperatorCallExpr *CE);
  bool TraverseCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *TOE);

  // Built-in operators:
  // Using Traverse- instead of Visit-Methods guarantees that nested calls are registered in their
  // binding-precedence-order. Thus the advice are woven in the correct order.

  // Define Traverse-methods for each built-in-operator separately, because
  // TraverseUnaryOperator-/TraverseBinaryOperator-methods do not get called and the
  // TraverseStmt-method does not work for every built-in operator (e.g. binary plus; Clang bug?).
  // TODO: ->, ., new, new[], delete, delete[], implicit conversion and ,

// Define the macro function, that creates the traverse-unary-built-in-operator-method:
#define GEN_UnaryOP_Traverse(NAME) \
  bool TraverseUnary##NAME(clang::UnaryOperator *UO) { \
    bool base_class_result = clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseUnary##NAME( UO ); \
    handle_built_in_operator_traverse( UO ); \
    return base_class_result; \
  };

  // Create Traverse-Method for each unary built-in operator:
  GEN_UnaryOP_Traverse(Plus)           // integer promotion: +a
  GEN_UnaryOP_Traverse(Minus)          // additive inverse: -a
  GEN_UnaryOP_Traverse(Not)            // bitwise negation: ~a
  GEN_UnaryOP_Traverse(LNot)           // logical(bool) negation: !a
  //GEN_UnaryOP_Traverse(Real)
  //GEN_UnaryOP_Traverse(Imag)
  //GEN_UnaryOP_Traverse(Extension)

// Define the macro function, that creates the traverse-binary-built-in-operator-method:
#define GEN_BinOP_Traverse(NAME) \
  bool TraverseBin##NAME(clang::BinaryOperator *BO) { \
    bool base_class_result = clang::RecursiveASTVisitor<ClangASTConsumer>::TraverseBin##NAME( BO ); \
    handle_built_in_operator_traverse( BO ); \
    return base_class_result; \
  };

  // Create Traverse-Method for each binary built-in operator:
  GEN_BinOP_Traverse(Mul)           // a * b
  GEN_BinOP_Traverse(Div)           // a / b
  GEN_BinOP_Traverse(Rem)           // a % b
  GEN_BinOP_Traverse(Add)           // a + b
  GEN_BinOP_Traverse(Sub)           // a - b
  GEN_BinOP_Traverse(Shl)           // a << b
  GEN_BinOP_Traverse(Shr)           // a >> b
  GEN_BinOP_Traverse(LT)            // a < b
  GEN_BinOP_Traverse(GT)            // a > b
  GEN_BinOP_Traverse(LE)            // a <= b
  GEN_BinOP_Traverse(GE)            // a >= b
  GEN_BinOP_Traverse(EQ)            // a == b
  GEN_BinOP_Traverse(NE)            // a != b
  GEN_BinOP_Traverse(And)           // a & b
  GEN_BinOP_Traverse(Xor)           // a ^ b
  GEN_BinOP_Traverse(Or)            // a | b
  // Short-circuit-operators:
  GEN_BinOP_Traverse(LAnd)          // a && b
  GEN_BinOP_Traverse(LOr)           // a || b

// Undefine the functions:
#undef GEN_UnaryOP_Traverse
#undef GEN_BinOP_Traverse

  // This method handles a traverse over a built-in operator.
  TU_Builtin *handle_built_in_operator_traverse(clang::Expr*);

  // code selection callbacks
  bool shouldVisitImplicitCode() const { return false; }
  bool shouldVisitTemplateInstantiations() const { return false; }

private:
  // specialized classes to handle the different cases while searching for data joinpoints

  // dummy context for ignoring data from subtrees
  class DummyContext : public ExprClassificationContext {
  public:
    DummyContext( ClangASTConsumer &con ) : ExprClassificationContext( con ) {
      setMode( Mode_Done );
    };

    virtual void enterSubExpr( clang::Expr *sub ) {};
    virtual void leaveSubExpr( clang::Expr *sub ) {};
    virtual void pushEntityRefNode( clang::Expr *RE ) {};
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD ) {};
    virtual void pushEntityForwardingJP( TU_Builtin *JP ) {};
  };

  // base to classify the given child as a single Access Type
  class SingleAccessContext : public ExprClassificationContext {
  protected:
    ClangModelBuilder::VarAccessInfo _var_info;
    TU_Builtin *_var_src;
  private:
    clang::Expr *_child;
  public:
    SingleAccessContext( ClangASTConsumer &con, clang::Expr *tree, clang::Expr *child ) : ExprClassificationContext( con ), _var_src( 0 ), _child( child ) {
      _var_info.element = 0;
      _var_info.decl = 0;
      _var_info.tree_node = tree;
      _var_info.ref_node = 0;
    }

    virtual void enterSubExpr( clang::Expr *sub );
    virtual void leaveSubExpr( clang::Expr *sub );

    virtual void pushEntityRefNode( clang::Expr *RE );
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD );
    virtual void pushEntityForwardingJP( TU_Builtin *JP );
  };

  // classify the given child as Get
  class SingleGetContext : public SingleAccessContext {
  public:
    SingleGetContext( ClangASTConsumer &con, clang::Expr *tree, clang::Expr *child ) : SingleAccessContext( con, tree, child ) {}

    virtual void registerJPforContext();
  };

  // classify the given child as Ref
  class SingleRefContext : public SingleAccessContext {
  private:
    TU_Builtin *_op_call;
  public:
    SingleRefContext( ClangASTConsumer &con, clang::Expr *tree, clang::Expr *child ) : SingleAccessContext( con, tree, child ), _op_call( 0 ) {}

    virtual void registerJPforContext();

    void updateOperator( TU_Builtin *op );
  };

  class BinaryClassificationContext : public ExprClassificationContext {
  public:
    enum SubExpressionIndex {
      SEI_LHS,
      SEI_RHS
    };
  private:
    clang::Expr *_lhs;
    clang::Expr *_rhs;
  public:
    BinaryClassificationContext( ClangASTConsumer &con, clang::Expr *lhs, clang::Expr *rhs ) : ExprClassificationContext( con ), _lhs( lhs ), _rhs( rhs ) {};

    virtual void enterSubExpr( clang::Expr *sub );
    virtual void leaveSubExpr( clang::Expr *sub );
  };

  // Forward the data from the chosen subexpr to the parent (used for BinComma for example)
  class BinaryForwardContext : public BinaryClassificationContext {
  private:
    SubExpressionIndex _forward;
  public:
    BinaryForwardContext( ClangASTConsumer &con, clang::Expr *lhs, clang::Expr *rhs, SubExpressionIndex forward ) : BinaryClassificationContext( con, lhs, rhs ), _forward( forward ) {};

    // setting Mode_Done if nothing to do (anymore)
    virtual void enterSubExpr( clang::Expr *sub );
    virtual void leaveSubExpr( clang::Expr *sub );

    virtual void pushEntityRefNode( clang::Expr *RE );
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD );
    virtual void pushEntityForwardingJP( TU_Builtin *JP );
  };

  // forward the data from the base of the array access and remember the forwarding
  class ArrayAccessContext : public BinaryForwardContext {
  private:
    TU_Builtin *_var_src;
  public:
    ArrayAccessContext( ClangASTConsumer &con, clang::ArraySubscriptExpr *ref ) : BinaryForwardContext( con, ref->getBase(), ref->getIdx(), BinaryForwardContext::SEI_LHS ), _var_src( 0 ) {};

    virtual void pushEntityForwardingJP( TU_Builtin *JP );

    void updateOperator( TU_Builtin *op );
    virtual bool suppressDecayJPs() { return true; };
  };

  // adds implicit joinpoints on any operator with assignment meaning ( =, +=, ++, ...)
  class AssignmentContext : public BinaryClassificationContext {
  private:
    ClangModelBuilder::VarAccessInfo _var_info;
    TU_Builtin *_var_src;
    bool _is_compound;

    TU_Builtin *_op_call;

    void commonInit( clang::Expr *tree ) {
      _var_info.element = 0;
      _var_info.decl = 0;
      _var_info.tree_node = tree;
      _var_info.ref_node = 0;

      _var_src = 0;
      _op_call = 0;
    };
  public:
    AssignmentContext( ClangASTConsumer &con, clang::BinaryOperator *AO ) : BinaryClassificationContext( con, AO->getLHS(), 0 ), _is_compound( AO->isCompoundAssignmentOp() ) { // we don't need information about the rhs
      commonInit( AO );
    }

    AssignmentContext( ClangASTConsumer &con, clang::UnaryOperator *PPO ) : BinaryClassificationContext( con, PPO->getSubExpr(), 0 ), _is_compound( true ) { // there is no rhs in this case
      assert( PPO->isIncrementDecrementOp() );
      commonInit( PPO );
    }

    // setting Mode_Done if nothing to do anymore
    virtual void leaveSubExpr( clang::Expr *sub );

    virtual void pushEntityRefNode( clang::Expr *RE );
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD );
    virtual void pushEntityForwardingJP( TU_Builtin *JP );

    virtual void registerJPforContext();

    void updateOperator( TU_Builtin *op );
  };

  // adds ref joinpoints for reference arguments and pushes information to parent on ref return type
  class CallContext : public ExprClassificationContext {
  private:
    clang::CallExpr *_call;
    clang::CXXConstructExpr *_constr;
    clang::FunctionDecl *_func_decl;
    ClangModelBuilder::VarAccessInfo _var_info;
    TU_Builtin *_var_src;

    bool _curarg_is_ref;
    unsigned int _next_arg;

    void clearVarInfo() {
      _var_info.element = 0;
      _var_info.decl = 0;
      _var_info.tree_node = 0;
      _var_info.ref_node = 0;

      _var_src = 0;
    }
  public:
    CallContext( ClangASTConsumer &con, clang::CallExpr *call ) : ExprClassificationContext( con ), _call( call ), _constr( 0 ), _func_decl( 0 ), _curarg_is_ref( false ), _next_arg( 0 ) {
      clearVarInfo();
    }

    CallContext( ClangASTConsumer &con, clang::CXXConstructExpr *constr ) : ExprClassificationContext( con ), _call( 0 ), _constr( constr ), _func_decl( 0 ), _curarg_is_ref( false ), _next_arg( 1 ) {
      clearVarInfo();
      _func_decl = constr->getConstructor();
    }

    unsigned int getNumArgs() {
      if( _call )
        return _call->getNumArgs();
      else if( _constr )
        return _constr->getNumArgs();
      else {
        assert( false && "Programming error, inconsistent code" );
        return 0;
      }
    }

    clang::Expr *getArg( unsigned int a ) {
      if( _call )
        return _call->getArg( a );
      else if( _constr )
        return _constr->getArg( a );
      else {
        assert( false && "Programming error, inconsistent code" );
        return 0;
      }
    }

    // checking which subexpr we are in
    virtual void enterSubExpr( clang::Expr *sub );
    virtual void leaveSubExpr( clang::Expr *sub );

    // collect info
    virtual void pushEntityRefNode( clang::Expr *RE );
    virtual void pushEntityInfo( ACM_Variable *var, clang::DeclaratorDecl *DD );
    virtual void pushEntityForwardingJP( TU_Builtin *JP );

    virtual void registerJPforContext();
    virtual void registerJPforSubExpr();
  };
};

#endif // __ClangASTConsumer_h__
