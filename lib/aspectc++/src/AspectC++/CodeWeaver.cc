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

#include "CodeWeaver.h"
#include "AspectInfo.h"
#include "AdviceInfo.h"
#include "ACModel/Utils.h"
#include "Naming.h"
#include "IntroductionUnit.h"
#include "NamespaceAC.h"
#include "ACResultBuffer.h"
#include "ModelBuilder.h"

#ifdef FRONTEND_CLANG
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;
#else
#include "Puma/ErrorSink.h"
#include "Puma/CProtection.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CProject.h"
#include "Puma/FileUnit.h"
#include "Puma/CSemDatabase.h"
using namespace Puma;
#endif

#ifdef FRONTEND_PUMA
void CodeWeaver::setup_tjp (ThisJoinPoint &tjp, CFunctionInfo* func) {
  assert (func->Tree ()->NodeName () == CT_FctDef::NodeId ());

  CT_CmpdStmt *body = ((CT_FctDef*)func->Tree ())->Body ();
  for (int i = 0; i < body->Entries (); i++)
    setup_tjp (tjp, body->Entry (i));
}

void CodeWeaver::setup_tjp (ThisJoinPoint &tjp, CTree *node) {

  const char *nodename = node->NodeName ();

  if (nodename == CT_QualName::NodeId () ||
      nodename == CT_RootQualName::NodeId()) {
    CT_QualName *qual_name = (CT_QualName*)node;
    CT_SimpleName *first_name = (CT_SimpleName*)qual_name->Entry (0);
    if (first_name->NodeName () == CT_SimpleName::NodeId () &&
        tjp.check_type (first_name->Text ())) {
      if (qual_name->Entries () == 2) {
        const char *text = qual_name->Text ();
        tjp.check_field (text);
      }
    }
  }
  else if (nodename == CT_SimpleName::NodeId ()) {
    CT_SimpleName *simple_name = (CT_SimpleName*)node;
    if (!tjp.check_obj (simple_name->Text ()))
      tjp.check_type (simple_name->Text ());
  }
  else if (nodename == CT_MembPtrExpr::NodeId () &&
      node->Son (0)->NodeName() == CT_SimpleName::NodeId () &&
      tjp.check_obj (node->Son (0)->token ()->text ())) {
    if (node->Son (2)->IsSimpleName ())
      tjp.check_field (node->Son (2)->IsSimpleName ()->Text (), true);
  }

  for (int s = 0; s < node->Sons (); s++)
    setup_tjp (tjp, node->Son (s));
}
#else
namespace {
// Visitor to find and register references to the TJP struct.
class TJPVisitor : public RecursiveASTVisitor<TJPVisitor> {
  typedef RecursiveASTVisitor<TJPVisitor> Base;
  ThisJoinPoint &_tjp;

public:
  explicit TJPVisitor(ThisJoinPoint &tjp) : _tjp(tjp) {}

  bool VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *E) {
    // for typedef C<&JoinPoint::signature> Foo
    if (const Type *ty = E->getQualifier()->getAsType()) {
      if (_tjp.check_type(TI_Type::get_type_text(QualType(ty, 0), 0, 0, TSEF_DONOTCHANGE, true, TSEF_DONOTCHANGE, false, true, false))) {
        _tjp.check_field(E->getDeclName().getAsString().c_str(), true);
      }
    }
    return true;
  }

  bool VisitType(Type *T) {
    // JoinPoint::Foo
    if (const DependentNameType *DNT = T->getAs<DependentNameType>()) {
      if (const Type *ty = DNT->getQualifier()->getAsType()) {
        if (_tjp.check_type(TI_Type::get_type_text(QualType(ty, 0), 0, 0, TSEF_DONOTCHANGE, true, TSEF_DONOTCHANGE, false, true, false)))
          _tjp.check_field(DNT->getIdentifier()->getNameStart(), true);
      }
    }
    return true;
  }

  bool TraverseCXXDependentScopeMemberExpr(CXXDependentScopeMemberExpr *DSME) {
    // tjp->foo
    if (!DSME->isImplicitAccess()) {
      if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(DSME->getBase()))
        if (_tjp.check_obj(DRE->getDecl()->getNameAsString()))
          _tjp.check_field(DSME->getMember().getAsString().c_str());
    }

    // JoinPoint::foo
    if (DSME->getQualifier()) {
      if (const Type *ty = DSME->getQualifier()->getAsType()) {
        if (_tjp.check_type(TI_Type::get_type_text(QualType(ty, 0), 0, 0, TSEF_DONOTCHANGE, true, TSEF_DONOTCHANGE, false, true, false)))
          _tjp.check_field(DSME->getMember().getAsString().c_str());
      }
    }

    return Base::TraverseCXXDependentScopeMemberExpr(DSME);
  }
};
}

void CodeWeaver::setup_tjp(ThisJoinPoint &tjp, clang::FunctionDecl *func) {
  Stmt *body = func->getBody();
  setup_tjp(tjp, body);
}

void CodeWeaver::setup_tjp(ThisJoinPoint &tjp, clang::Stmt *node) {
  TJPVisitor(tjp).TraverseStmt(node);
}
#endif

void CodeWeaver::type_check (const ACM_Class *cls, const string &name, bool result) {

  // TODO: type checks in unions might be necessary for call advice - not handled
  // TODO: aggregate initialization might be broken by adding virtual functions
  const TI_Class *ti = TI_Class::of(*cls);

  // create the code
  ostringstream code;
  code << endl;
  if (ti->is_class ())
    code << "public:" << endl;
  code << "  virtual bool " << name << " () const { return "
       << (result ? "true" : "false") << "; }" << endl;
  if (ti->is_class ())
    code << "private:" << endl;

  // paste the function after "{" of the class/struct definition
  insert (ti->body_start_pos(*this), code.str (), true);
}

void CodeWeaver::add_aspect_include (ACM_Any *jpl, AspectInfo &aspect_info,
  AspectRef::Kind kind) {
  if (jpl && is_pseudo(*jpl))
    return;

  // find the unit (file, macro, or intro) in which the joinpoint is located
#ifdef FRONTEND_PUMA
  Unit *iu = TransformInfo::unit (*jpl);

  // if the 'insert unit' is an 'introduction unit', find the intro target unit
  IntroductionUnit *intro_unit = 0;
  while ((intro_unit = IntroductionUnit::cast (iu)) != 0)
    iu = intro_unit->target_unit ();

  _aspect_includes.insert (iu, &aspect_info, kind);
#else
  clang::Decl *decl = TransformInfo::decl (*jpl);
  // TODO: define a real interface for both, Puma and Clang variant -> ugly code here
  if( jpl->type_val() & JPT_Access )
    decl = TI_Access::of( *( static_cast<ACM_Access *>( jpl ) ) )->origin();

  if (!decl)
    return;

  // if the 'insert unit' is an 'introduction unit', find the intro target unit
  clang::SourceManager &sm = getRewriter().getSourceMgr();
  SourceLocation result_loc = sm.getExpansionLoc(decl->getLocation());
  const llvm::MemoryBuffer *iu = sm.getBuffer(sm.getFileID(result_loc));
  while (IntroductionUnit *intro_unit = IntroductionUnit::cast(iu)) {
    iu = intro_unit->target_unit ();
    result_loc = intro_unit->location ();
  }

  result_loc = sm.getExpansionLoc(result_loc);

//  _aspect_includes.insert(sm.getFileID(result_loc), &aspect_info, kind);
  _aspect_includes.insert(sm.getFileEntryForID (sm.getFileID (result_loc)),
      &aspect_info, kind);
#endif
}

void CodeWeaver::insert_aspect_includes () {

  // weave the aspect includes in all registered unit
  for (AspectIncludes::const_iterator iter = _aspect_includes.begin ();
    iter != _aspect_includes.end (); ++iter) {

    // generate the includes for the current unit
    string includes = _aspect_includes.generate (iter, problems ());

    // TODO: why do we use clang::FileID here? Shouldn't ACFileID do it?
    // determine the insertion position for the current unit
    ACFileID unit = _aspect_includes.unit (iter);

    // paste the includes (and optional guard head) at the beginning of the unit
    insert (header_pos (unit), includes);
  }
}


void CodeWeaver::insert_invocation_functions (ACM_Aspect *jpl_aspect,
    const string &defs) {

  const TI_Aspect *ti = TI_Aspect::of(*jpl_aspect);
#ifdef FRONTEND_CLANG
  // FIXME: This is a hack. In the Clang configuration objdecl_end_pos returns
  //        the position behind die closing bracket, not the semi colon,
  //        because Clang doesn't store the SourceLocation in the AST.
  string hack_defs = string (";") + defs;
  insert (ti->objdecl_end_pos(*this), hack_defs);
#else
  // insert the code after the ";" after the aspect definition
  insert (ti->objdecl_end_pos(*this), defs);
#endif
}


#ifdef FRONTEND_PUMA
void CodeWeaver::open_namespace (ostream &out, const SyntacticContext &ctxt) {
  CObjectInfo *obj = ctxt.object ();
  CClassInfo *cls = obj->ClassInfo();
  if (cls && cls->TemplateInfo ())
    obj = (CObjectInfo*)cls->TemplateInfo ();

  Array<CObjectInfo*> namespaces;
  while (!obj->Scope ()->GlobalScope () && obj->Scope ()->NamespaceInfo ()) {
    obj = obj->Scope ();
    namespaces.append (obj);
  }

  for (int i = namespaces.length () - 1; i >= 0; i--) {
    if (namespaces[i]->isAnonymous ())
      out << "namespace {" << endl;
    else
      out << "namespace " << namespaces[i]->Name () << " {" << endl;
  }
}
#else
void CodeWeaver::open_namespace (ostream &out, const SyntacticContext &ctxt) {
  const DeclContext *obj = cast<DeclContext> (ctxt.decl ());
  std::vector<const NamespaceDecl *> namespaces;
  while (!obj->getParent()->isTranslationUnit()) {
    obj = obj->getParent();
    if (obj->isNamespace())
      namespaces.push_back(cast<NamespaceDecl>(obj));
  }

  for (int i = namespaces.size () - 1; i >= 0; i--) {
    if (namespaces[i]->isAnonymousNamespace())
      out << "namespace {" << endl;
    else
      out << "namespace " << namespaces[i]->getNameAsString() << " {" << endl;
  }
}
#endif

#ifdef FRONTEND_PUMA
void CodeWeaver::close_namespace (ostream &out, const SyntacticContext &ctxt) {
  CObjectInfo *obj = ctxt.object ();
  CClassInfo *cls = obj->ClassInfo();
  if (cls && cls->TemplateInfo ())
    obj = (CObjectInfo*)cls->TemplateInfo ();

  ostringstream ns_close;
  while (!obj->Scope ()->GlobalScope () && obj->Scope ()->NamespaceInfo ()) {
    obj = obj->Scope ();
    out << "} // closed " << (obj->isAnonymous () ? "" : obj->QualName ()) << endl;
  }
}
#else
void CodeWeaver::close_namespace (ostream &out, const SyntacticContext &ctxt) {
  const DeclContext *obj = ctxt.decl ()->getLexicalDeclContext ();
  ostringstream ns_close;
  while (obj->isNamespace()) {
    const NamespaceDecl *nd = cast<NamespaceDecl>(obj);
    out << "} // closed " << (nd->isAnonymousNamespace () ? "" : nd->getNameAsString())
        << endl;
    obj = obj->getParent();
  }
}
#endif


// generate the code, which proceeds an intercepted flow of control, e.g.
// continues a call or execution
void CodeWeaver::make_proceed_code (ostream &out, ACM_Code *loc,
                                    bool action, vector<string> *arg_names) {
  TI_Code &ti = *TI_Code::of( *loc );
  TI_Code &cast_ti = ti;

  // find out which entity has to be used for proceeding
  ACM_Name *entity = 0;
  if( has_entity( *loc ) )
    entity = get_entity( *loc );
  ACM_Function *func = 0;
  if( has_entity_func( *loc ) )
    func = get_entity_func( *loc );

  // set flags that describe the kind of join point
  enum {
    PM_NONE,
    PM_DIRECT,
    PM_INDIRECT_ENTITY,
    PM_INDIRECT_MEMBER,
    PM_BYPASS,
    PM_ARGS_ONLY,
    PM_OLD
  } mode = PM_NONE;
  bool wrap = false;
  std::string wrap_begin, wrap_end;
  bool use_obj = entity && needs_this( *entity );
  bool expr_is_dependent = false;
  bool is_obj_const = false;
  bool is_inner = false, is_outer = false;
  bool need_FQN = ! use_obj;
#ifdef FRONTEND_CLANG
  // only used for clang-only features
  bool need_idx = false;
#endif
  enum {
    AM_NONE,
    AM_FUNCTION,
    AM_OPERATORCALL,
    AM_ASSIGN
  } args_mode = AM_FUNCTION;

  JoinPointType jpType = loc->type_val();
  if( jpType & JPT_Access ) {
#ifdef FRONTEND_CLANG
    TI_Access &ti = static_cast<TI_Access &>(cast_ti);
#else
    // the generalisation were only done for the clang variant
    // so fall back to original call API
    TI_MethodCall &ti = static_cast<TI_MethodCall &>(cast_ti);
#endif

    is_outer = true;
    mode = PM_DIRECT;

    is_obj_const = ti.target_is_const();

    if( ti.needs_rights() )
      mode = PM_INDIRECT_ENTITY;

    if( jpType == JPT_Call ) {
      TI_MethodCall &ti = static_cast<TI_MethodCall &>(cast_ti);

      if( ti.needs_rights() )
        mode = PM_BYPASS;

#ifdef FRONTEND_CLANG
      // TODO: implement this check for Puma as well
      if (ti.uses_ADN_lookup ()) {
        need_FQN = false;
        if (ti.is_operator_call ()) {
          mode = PM_ARGS_ONLY;
          args_mode = AM_OPERATORCALL;
        }
      }
#endif
    }
#ifdef FRONTEND_CLANG
    // only supported in Clang variant
    else if( jpType == JPT_Builtin ) {
      mode = PM_ARGS_ONLY;
      args_mode = AM_OPERATORCALL;
    }
    else if( jpType == JPT_Get ) {
      args_mode = AM_NONE;
      need_idx = true;
    }
    else if( jpType == JPT_Set ) {
      args_mode = AM_ASSIGN;
      need_idx = true;
    }
    else if( jpType == JPT_GetRef ) {
      args_mode = AM_NONE;
      mode = PM_INDIRECT_ENTITY;
    }
    else if( jpType == JPT_SetRef ) {
      args_mode = AM_ASSIGN;
      mode = PM_INDIRECT_ENTITY;
    }
    else if( jpType == JPT_Ref ) {
      TI_Ref &ti = static_cast<TI_Ref &>(cast_ti);

      args_mode = AM_NONE;
      need_idx = true;
      if( ti.result_is_ptr() ) {
        wrap = true;
        wrap_begin = "&( ";
        wrap_end = " )";
      }
    }
#endif
    else {
      assert( false && "Unknown JPType !" );
      return;
    }

    if( ti.has_target_expr() && ! action && mode != PM_BYPASS )
      use_obj = true; // use obj if we can and have an expression even if not strictly necessary
    if( ti.is_qualified() )
      need_FQN = true; // keep existing qualification
#ifdef FRONTEND_CLANG
    if( need_idx && ti.entity_index_count() == 0 )
      need_idx = false; // suppress handling if there are no indices
#endif
  }
  else if ( jpType & ( JPT_Execution | JPT_Construction | JPT_Destruction ) ) {
    is_inner = true;
    mode = PM_OLD;

    // nothing is done to proceed implicitly defined special member functions
    if( ( jpType & ( JPT_Construction | JPT_Destruction ) ) && func->get_builtin() )
      mode = PM_NONE;
  }
  else {
    assert( false && "Unknown JPType !" );
    return;
  }
  // some consistency checks
  assert( is_inner || is_outer );
  if( mode == PM_INDIRECT_ENTITY ) {
    need_FQN = false;
    use_obj = false;
#ifdef FRONTEND_CLANG
    need_idx = false;
#endif
  }
  else if( mode == PM_INDIRECT_MEMBER ) {
    need_FQN = false;
  }
  else if( mode == PM_BYPASS ) {
    need_FQN = false;
  }
  else if( mode == PM_ARGS_ONLY ) {
    need_FQN = false;
  }

  // if nothing to do return
  if( mode == PM_NONE )
    return;

  // find out if the entity is a member of a class
  ACM_Class *dst_cls = entity ? cscope( entity ) : 0;

  // create a local class that enters/leaves the cflow triggers
  const CFlowList &cflows = ti.cflows();
  if (cflows.size () > 0) {
    out << "{" << endl;
    cflows.gen_trigger_obj (out);
  }

  // generate the expression that calculates the result
  stringstream res;
  int args = 0;

  if( wrap )
    res << wrap_begin;

  if( mode == PM_BYPASS ) {
    // check privileged calls to external functions
    if (!in_project(*func)) {
      // calls to "private" externals are not supported, thus the
      // target function is "protected"
      // => we have to use the target object's class bypass
      assert( loc->type_val() == JPT_Call );
      if (((ACM_Call*)loc)->has_target_class())
        dst_cls = ((ACM_Call*)loc)->get_target_class();
      else
        dst_cls = (ACM_Class*)lexical_scope(*loc);
    }

    res << "::" << signature(*dst_cls) << "::"
        << Naming::bypass_caller_class(dst_cls) << "<";
    Naming::bypass_id_class (res, (ACM_Call*)loc);
    res << ">::call";
    if (use_obj) {
      res << "((::" << signature(*dst_cls) << "*)";
      args++;
    }
  }

  // if an object is needed for the call, issue 'obj[->]'
  if( use_obj ) {
    if( is_outer ) {
      if (action)
        res << "__TJP::target ()";
      else {
        res << "((";
        if( is_obj_const )
          res << "const ";
        res << "TTarget*)&dst)";
        expr_is_dependent = true;
      }
    }
    else if( is_inner ) {
      if (action)
        res << "__TJP::that ()";
      else
        res << "this";
    }

    if( args == 0 ) { // check if we already have args, which means obj is passed on, else emit member access
      res << "->";
      if( expr_is_dependent && mode == PM_DIRECT && jpType == JPT_Call ) {
        TI_MethodCall &ti = static_cast<TI_MethodCall &>( cast_ti );
        if (ti.has_explicit_template_params())
          res << "template ";
      }
    }
  }

  // Generate the name and arguments of the destination function or built-in operator

  // print the namespace names if needed
  if( need_FQN ) {
     res << name_qualifier( entity );
  }

  // generate the entity name
  if( mode == PM_OLD ) {
    // use __old_<name> for execution join points
    Naming::exec_inner (res, loc);
  }
  else if ( mode == PM_DIRECT ) {
    // destination entity name
    if (func && TI_Function::of(*func)->is_conversion_operator ())
      res << entity->get_name ();
    else
      res << strip_template_parameters( entity->get_name () );
  }
  else if( mode == PM_INDIRECT_ENTITY ) {
    if( action )
      res << "*(__TJP::entity())";
    else
      res << "ent";
  }
  else if( mode == PM_INDIRECT_MEMBER ) {
    if( action )
      res << "__TJP::memberptr()";
    else
      res << "member";
  }

  if( mode == PM_DIRECT && jpType == JPT_Call ) {
    TI_MethodCall &ti = static_cast<TI_MethodCall &>( cast_ti );
    // add the template arguments if it is a function template instance
    bool call_uses_template_params = ti.has_explicit_template_params();
#ifdef FRONTEND_PUMA
    CFunctionInfo *dstfunc = ((TI_Function*)func->transform_info())->func_info ();
    CTemplateInstance *instance = dstfunc->TemplateInstance ();
    if (instance && call_uses_template_params) {
      res << "< ";
      for (unsigned a = 0; a < instance->DeducedArgs (); a++) {
        if (a > 0) res << ",";
        DeducedArgument *arg = instance->DeducedArg (a);
        if (arg->Type ())
          arg->Type ()->TypeText (res, "", true, true);
        else if (arg->Value ()) {
          if (arg->Value ()->isSigned ())
            res << arg->Value ()->convert_to_int ();
          else if (arg->Value ()->isUnsigned ())
            res << arg->Value ()->convert_to_uint ();
          else if (arg->Value ()->isFloat ())
            res << arg->Value ()->convert_to_float ();
        }
        else
          res << "*invalid template arg*";
      }
      res << " >";
    }
#else
    if (call_uses_template_params) {
      clang::ASTContext &ctx = ti.called()->getASTContext();
      res << "< ";
      const TemplateArgumentLoc *template_args = ti.get_explicit_template_params();
      unsigned num_args = ti.num_explicit_template_params();
      for (unsigned a = 0; a < num_args; a++) {
        if (a > 0) res << ",";
        res << TI_Type::get_templ_arg_text(template_args[a].getArgument (), &ctx, TSEF_ENABLE, true, TSEF_DONOTCHANGE, false, false);
      }
      res << " >";
    }
#endif
  }

  // necessary indices (only supported in clang variant)
#ifdef FRONTEND_CLANG
  if( need_idx ) {
    assert( jpType & JPT_Access );
    TI_Access &ti = static_cast<TI_Access &>( cast_ti );

    unsigned int idx_count = ti.entity_index_count();
    for( unsigned int i = 0 ; i < idx_count; i++ ) {
      res << "[";
      // Insert current idx:
      if( action )
        res << "__TJP::template idx<" << i << ">()";
      else
        res << "idx" << i;
      res << "]";
    }
  }
#endif

  // destination argument expression
  if( args_mode != AM_NONE ) {
    // Because the built-in-operator-syntax (e.g. arg0 = arg1) differs from
    // function-/method-call-syntax (<name>(<args>)), we need a new way to handle all
    // these different cases as identically as possible. For that purpose we determine
    // delimiter-strings, which will be located between the argument-string, for every
    // case. After that we can handle all cases the same way.

    int arg_count = get_arg_count(*loc);
    std::vector<std::string> arg_delimiters;
    // Determine the delimiters depending on the current case:
    if( args_mode == AM_FUNCTION ) {
      // if no built-in operator:

      // Generate delimiters:
      // Case: "normal" function/method call
      // Schema: <functionname (already inside "res")>(arg0, arg1, ..., argN)
      // differentiate, depending on weather arguments are already inside "res":
      if( args == 0 )
        arg_delimiters.push_back( "(" );
      else if( arg_count > 0 )
        arg_delimiters.push_back( "," );
      else
        arg_delimiters.push_back( "" );
      for(int i = 1; i < arg_count; i++) {
        arg_delimiters.push_back(", ");
      }
      arg_delimiters.push_back(")");
    }
    // features only supported in Clang variant
#ifdef FRONTEND_CLANG
    else if( args_mode == AM_OPERATORCALL ) {
      // Case: built-in operator or user-defined operator with ADN-lookup
      assert( loc->type_val() == JPT_Builtin || loc->type_val() == JPT_Call);
      TI_CommonCall &ti = static_cast<TI_CommonCall &>( cast_ti );
      // Operator-kind as std::string:
      std::string operator_kind = ti.operator_kind_string();

      // Surround the args with parentheses to preserve the correct operator precedence
      // even if a arg will equate to a string like "a, b" (theoretically possible
      // because of parameter "arg_names[]").
      if( ti.is_postfix_expr() ) {
        // Special case: increment- or decrement-POSTFIX-operator
        // Schema: "(arg0<Operator>)"
        arg_delimiters.push_back("((");
        arg_delimiters.push_back(")" + operator_kind + ")");
        arg_count--; // ignore the dummy argument
      }
      else if( ti.is_unary_expr() ) {
        // Unary built-in operator (*, &, +, -, !, ~, ...)
        // Schema: "(<Operator>arg0)"
        arg_delimiters.push_back("(" + operator_kind + "(" );
        arg_delimiters.push_back("))");
      }
      else if( ti.is_binary_expr() ) {
        // Binary built-in operator
        // Schema: "(arg0 <Operator> arg1)"
        arg_delimiters.push_back("((");
        arg_delimiters.push_back(") " + operator_kind + " (");
        arg_delimiters.push_back("))");
      }
      else if( ti.is_index_expr() ) {
        // Special case: array subscript
        // Schema: "(arg0[arg1])"
        arg_delimiters.push_back("((");
        arg_delimiters.push_back(")[");
        arg_delimiters.push_back("])");
      }
      else if( ti.is_ternary_expr() ) {
        // Special case: conditional operator
        // Schema: "(arg0 ? arg1 : arg2)"
        arg_delimiters.push_back("((");
        arg_delimiters.push_back(") ? (");
        arg_delimiters.push_back(") : (");
        arg_delimiters.push_back("))");
      }
      else
        assert( false && "unknow built-in operator type in proceed_code" );
    }
    else if( args_mode == AM_ASSIGN ) {
      arg_delimiters.push_back( " = (" ); // entity is already inside
      arg_delimiters.push_back( ")" );
    }
#endif // FRONTEND_CLANG

    // Add the delimiter- and arg-strings:
    // Insert first delimiter (necessary due to the case where argument-count equals
    // zero or bypass is needed)
    res << arg_delimiters.front();
    for (int a = 0 ; a < arg_count; a++, args++) {
      // Insert current delimiter:
      if(a > 0) { // Since the first delimiter was inserted already
          res << arg_delimiters.at(a);
      }
      // Insert current arg:
      if (action) {
        // make sure that if the argument type was a reference we use the
        // referred type here
        res << "*(typename __TJP::template Arg<" << a
            << ">::ReferredType*)__TJP::arg(" << a << ")";
      }
      else {
        if (arg_names)
          res << (*arg_names)[a];
        else
          res << "arg" << a;
      }
    }
    // Insert last delimiter (generally the closing parenthesis):
    res << arg_delimiters.back();
  }

  if( wrap )
    res << wrap_end;

  // generate the 'result =' code
  // TODO: better pass result_buffer as parameter
  ACResultBuffer result_buffer (project (), loc, is_outer);
#ifdef FRONTEND_CLANG
  // inform buffer about const forced resulttype
  if( loc->type_val() == JPT_Builtin && static_cast<TI_Builtin &>( ti ).is_forwarding() )
    result_buffer.has_const_result();
#endif

  out << "  ";
  if (action)
    out << result_buffer.action_result_assignment(res.str ());
  else
    out << result_buffer.result_assignment(res.str ());
  out << ";" << endl;

  if (cflows.size () > 0)
    out << "}" << endl;
}

#ifdef FRONTEND_CLANG
// only required for features of the Clang variant
void CodeWeaver::make_proceed_to_implicit( ostream &out, ACM_Code *loc, bool action, vector<string> *arg_names ) {
  out << "  /* Implicit joinpoint calls */" << endl;

  assert( loc->type_val() == JPT_Builtin );
  ACM_Builtin &parent_jpl = *( static_cast<ACM_Builtin *>( loc ) );
  TI_Builtin &parent_ti = *TI_Builtin::of( parent_jpl );
  std::string op_string = parent_ti.operator_kind_string();

  typedef ACM_Container<ACM_Access, true> Container;
  Container &implicit = parent_jpl.get_implicit_access();
  Container::const_iterator it = implicit.begin();

  bool packed_entity = has_complex_entity( *it );

  string ent;
  string new_val;
  string that;
  string packed;
  if( action ) {
    // make sure that if the argument type was a reference we use the
    // referred type here
    ent = "*(typename __TJP::template Arg<0>::ReferredType*)__TJP::arg(0)";
    new_val = "*(typename __TJP::template Arg<0>::ReferredType*)__TJP::arg(1)";
    that = "__TJP::that ()";
    packed = "*__TJP::__wormhole";
  }
  else {
    if( arg_names ) {
      ent = (*arg_names)[0];
      new_val = (*arg_names)[1];
    }
    else {
      ent = "arg0";
      new_val = "arg1";
    }
    that = "srcthis";
    packed = "__packed";
  }

  // make the nested wrapper calls
  if( parent_ti.is_compound_assignment() ) {
    assert( *it && ( *it )->type_val() & ( JPT_Get | JPT_GetRef ) );
    ACM_Access *jpl = static_cast<ACM_Access *>( *it++ );
    TI_Access *ti = TI_Access::of( *jpl );

    out << "  ";
    out << ti->result_type_string() << " __new_value = ";

    if( jpl->has_plan() ) {
      stringstream close;
      bool args = generate_access_wrapper( jpl, out, close, that.c_str() );

      if( args )
        out << ", ";
      out << ( packed_entity ? packed : ent );

      out << close.str();
    }
    else {
      // no advice to weave for this joinpoint
      // fallback to direct eval
      out << ent;
    }
    out << ";" << endl;

    if( parent_ti.is_postfix_expr() ) {
      // generate the 'result =' code
      // TODO: better pass result_buffer as parameter
      ACResultBuffer result_buffer( project(), loc, true );
      // inform buffer about const forced resulttype
      if( parent_ti.is_forwarding() )
        result_buffer.has_const_result();

      out << "  ";
      if( action )
        out << result_buffer.action_result_assignment( "__new_value" );
      else
        out << result_buffer.result_assignment( "__new_value" );
      out << ";" << endl;
    }

    // perform the compound operation
    assert( parent_ti.is_binary_expr() || parent_ti.is_unary_expr() || parent_ti.is_postfix_expr() );
    if( parent_ti.is_binary_expr() )
      out << "  __new_value " << op_string << " " << new_val << ";" << endl;
    else if( parent_ti.is_unary_expr() )
      out << "  " << op_string << "__new_value;" << endl;
    else if( parent_ti.is_postfix_expr() )
      out << "  __new_value" << op_string << ";" << endl;
    new_val = "__new_value"; // use the computed value for set
  }

  if( ( op_string == "=" ) || parent_ti.is_compound_assignment() ) {
    assert( *it && ( *it )->type_val() & ( JPT_Set | JPT_SetRef ) );
    ACM_Access *jpl = static_cast<ACM_Access *>( *it++ );

    if( jpl->has_plan() ) {
      out << "  ";

      stringstream close;
      bool args = generate_access_wrapper( jpl, out, close, that.c_str() );

      if( args )
        out << ", ";
      out << ( packed_entity ? packed : ent );
      out << ", ";
      out << new_val;

      out << close.str() << ";" << endl;
    }
    else {
      // no advice to weave for this joinpoint
      // fallback to direct eval
      out << "  " << ent << " = " << new_val << ";" << endl;
    }

    if( ! parent_ti.is_postfix_expr() ) {
      // generate the 'result =' code
      // TODO: better pass result_buffer as parameter
      ACResultBuffer result_buffer( project(), loc, true );
      // inform buffer about const forced resulttype
      if( parent_ti.is_forwarding() )
        result_buffer.has_const_result();

      out << "  ";
      if( action )
        out << result_buffer.action_result_assignment( ent );
      else
        out << result_buffer.result_assignment( ent );
      out << ";" << endl;
    }
  }
  else if( op_string == "&" ) {
    assert( *it && ( *it )->type_val() == JPT_Ref );
    ACM_Ref *jpl = static_cast<ACM_Ref *>( *it++ );

    stringstream wrapper;
    stringstream close;
    bool args = generate_access_wrapper( jpl, wrapper, close, that.c_str() );

    if( args )
      wrapper << ", ";
    wrapper << ( packed_entity ? packed : ent );

    wrapper << close.str();

    // generate the 'result =' code
    // TODO: better pass result_buffer as parameter
    ACResultBuffer result_buffer( project(), loc, true );
    // inform buffer about const forced resulttype
    if( parent_ti.is_forwarding() )
      result_buffer.has_const_result();

    out << "  ";
    if( action )
      out << result_buffer.action_result_assignment( wrapper.str() );
    else
      out << result_buffer.result_assignment( wrapper.str() );
    out << ";" << endl;
  }
}
#endif

// create an action wrapper function, which may be invoked by 'proceed()'
void CodeWeaver::make_action_wrapper(ostream &impl, ACM_Code *loc, int depth) {
  // generate the action function, which is used to obtain an action object
  impl << "  AC::Action &action() {" << endl;
  impl << "    this->__TJP::_wrapper = &";
  Naming::action_wrapper (impl, loc, depth);
  impl << ";" << endl;
  impl << "    return *this;" << endl;
  impl << "  }" << endl;

  // make the function static if it is defined in class scope
  impl << "  static void ";
  Naming::action_wrapper (impl, loc, depth);
  impl << " (AC::Action &action) {" << endl;
  // convert the action object ref into a TJP_... object ref and call proceed
  impl << "    ((__TJP&)action).proceed ();" << endl;

  impl << "  }" << endl;
}

// create the 'proceed()' function for the current level
void CodeWeaver::make_proceed_func( ostream &impl, ACM_Code *loc, ACM_CodePlan *plan, bool has_wormhole ) {

  const ThisJoinPoint &tjp =
      TI_AdviceCode::of (*plan->get_around()->get_advice())->this_join_point ();

  // the signature
  impl << "  ";
  if (tjp.proceed_calls() == 1 && _problems._use_always_inline)
    impl << "__attribute__((always_inline)) ";
  impl << "void proceed () {" << endl;

  if (depth(*plan) == 0) {
    // this is a proceed function for the last around advice
    // the implicit joinpoints are only supported in the clang variant
#ifdef FRONTEND_CLANG
    if( TI_Code::of( *( static_cast<ACM_Code *>( loc ) ) )->has_implicit_joinpoints() )
      make_proceed_to_implicit( impl, loc, true );
    else
#endif
      make_proceed_code( impl, loc, true );
  }
  else {
    // generate __TJP type (for the next level)
    impl << "    typedef ";
    Naming::tjp_struct (impl, (ACM_Code*)loc, depth (*plan) - 1);
    impl << "<TResult, TThat, TTarget, ";
#ifdef FRONTEND_CLANG
    // Entity is only supported in the Clang variant
      impl << "TEntity, ";
      if (loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) {
        impl << "TDims, ";
      }
#endif
    impl << "TArgs";
    if( has_wormhole )
      impl << ", TWORMHOLE";
    impl << "> __TJP;" << endl;

    // generate advice calls
    make_advice_calls (impl, plan->get_next_level(), loc, true);
  }
  impl << "  }" << endl;
}

// generate the name of the joinpoint-specific TJP type
void CodeWeaver::make_tjp_typename( ostream &out, ACM_Code *loc, int depth, ACResultBuffer &rb, const char *wormhole_type ) {
  TI_Code *ti = (TI_Code*)TransformInfo::of (*loc);
  Naming::tjp_struct (out, loc, depth);
  out << "< " << rb.tjp_result_type() << ", ";
#ifdef FRONTEND_PUMA
  ti->that_type ()->TypeText (out, "", true, true);
#else
  out << ti->that_type_string();
#endif
  out << ", ";
#ifdef FRONTEND_PUMA
  if( loc->type_val() == JPT_Call && static_cast<TI_MethodCall*>( ti )->has_target_expr() ) {
    // TODO: is this case needed at all?
    if (((TI_MethodCall*)ti)->target_is_const())
#else
  if( ( loc->type_val() & JPT_Access ) && static_cast<TI_Access*>( ti )->has_target_expr() ) {
    // TODO: is this case needed at all?
    if( static_cast<TI_Access *>( ti )->target_is_const() )
#endif
      out << "const ";
    out << "TTarget";
  }
  else
#ifdef FRONTEND_PUMA
    ti->target_type ()->TypeText (out, "", true, true);
#else
    out << ti->target_type_string();

  // entity functionallity is only supported in Clang variant
    out << ", ";
    // TODO: check get/set stuff
    if (loc->type_val() & JPT_Call) {
      out << rb.tjp_result_type() << " (";
      list<ACM_Type*> arg_types;
      get_arg_types (*loc, arg_types);
      unsigned a = 0;
      for (list<ACM_Type*>::iterator i = arg_types.begin(); i != arg_types.end(); ++i) {
        if (a > 0)
          out << ",";
        out << "TArg" << a;
        a++;
      }
      out << ")";
      ACM_Function *function = (static_cast<ACM_Call*>(loc))->get_target();
      switch (function->get_cv_qualifiers()) {
      case CVQ_CONST:          out << " const";          break;
      case CVQ_VOLATILE:       out << " volatile";       break;
      case CVQ_CONST_VOLATILE: out << " const volatile"; break;
      default: break;
      }
    }
    else
      out << ti->entity_type_string();

    if (loc->type_val() & ( JPT_Access & ~ ( JPT_Call | JPT_Builtin ) ) ) {
      TI_Access *tiA = static_cast<TI_Access *>( ti );
      const unsigned int idx_count = tiA->entity_index_count();

      out << ", ";
      for( unsigned int i = 0; i < idx_count; i++ )
        out << "AC::DIL< " << tiA->entity_index_dimension( i ) << ", " << tiA->entity_index_type( i ) << ", ";
      out << "AC::DILE";
      for( unsigned int i = 0; i < idx_count; i++ )
        out << " >";
    }
#endif
  // argument types
  unsigned arg_count = get_arg_count (*loc);
  out << ", ";
  list<ACM_Type*> arg_types;
  get_arg_types (*loc, arg_types);
  unsigned a = 0;
  for (list<ACM_Type*>::iterator i = arg_types.begin(); i != arg_types.end(); ++i) {
    out << " AC::TL< ";
#ifdef FRONTEND_PUMA
    if( loc->type_val() != JPT_Call ) {
      CTypeInfo *argtype = TI_Type::of (**i)->type_info ();
      argtype->TypeText (out, "", true, true);
    }
#else
    if( (loc->type_val() & JPT_Access) == 0 )
      out << ti->arg_type_string( a );
#endif
    else
      out << "TArg" << a;
    out << ",";
    a++;
  }
  out << " AC::TLE";
  for (unsigned a = 0; a < arg_count; a++)
    out << " >";

  if( wormhole_type )
    out << ", " << wormhole_type;

  out << " >";
 }

// insert TJP class definition before join point
void CodeWeaver::make_tjp_struct(ostream &out, ACM_Code *loc,
    ACM_CodePlan *plan, const ThisJoinPoint &tjp) {

  int depth = ::depth (*plan);
  // recursively call the function for the previous TJP classes
  if (depth > 0) {
    make_tjp_struct (out, loc, plan->get_next_level(), tjp);
  }

  // generate the definition
  out << endl;
  tjp.gen_tjp_struct (out, loc, _problems, depth);

  // generate the proceed function for the current structure
  if (plan->has_around()) {
    const ThisJoinPoint &curr_tjp =
      TI_AdviceCode::of (*plan->get_around()->get_advice())->this_join_point ();
    if (curr_tjp.proceed () || curr_tjp.action() ||
        !TI_CodeAdvice::of(*plan->get_around())->get_advice_info()->pointcut().cflow_triggers ().empty ())
      make_proceed_func( out, loc, plan, tjp.has_wormhole() );

    // generate action wrapper if needed
    if (curr_tjp.action())
      make_action_wrapper (out, loc, depth);
  }

  if (depth == 0) {
    stringstream jpname;
    Naming::tjp_struct(jpname, loc, 0);
    gen_binding_templates (out, loc->get_plan(), jpname.str ().c_str ());
  }

  // closing bracket of the TJP class! Opened in gen_tjp_struct.
  out << "};" << endl;

  out << endl;
}


// generates the signature of a wrapper function for exec/cons/dest join pts
string CodeWeaver::wrapper_function_signature (ACM_Code *loc,
    const SyntacticContext &sctxt, bool def) {

  ACM_Function *function = (ACM_Function*)loc->get_parent ();

  // TODO: implement for Clang variant
#ifdef FRONTEND_PUMA
  set<string> used_args;
  bool search_args = (def && function->get_kind() == FT_CONSTRUCTOR);
  if (search_args)
    sctxt.search_used_args (used_args);
#endif // FRONTEND_PUMA

  stringstream name_args;
  if (sctxt.qualified_scope() != "" &&
      sctxt.syntactical_scope() != sctxt.qualified_scope())
    name_args << sctxt.qualified_scope() << "::";
  Naming::exec_inner (name_args, loc);
  name_args << "(";
  for (unsigned a = 0; a < sctxt.args(); a++) {
    if (sctxt.arg_type(a) == "void")
      break;
    if (a > 0) name_args << ",";
    string name = sctxt.arg_name(a);
#ifdef FRONTEND_PUMA
    if (name != "" && search_args && used_args.find (name) == used_args.end ())
      name = "";
#endif // FRONTEND_PUMA
    name_args << sctxt.arg_type(a, name);
  }
  name_args << ")";

  ostringstream wrapped;

  if (_problems._use_always_inline) // GNU extension
    wrapped << "__attribute__((always_inline)) ";
  wrapped << "inline "; // the wrapped function should always be inlined
  if (function->get_kind() == FT_STATIC_MEMBER && sctxt.is_in_class_scope())
    wrapped << "static ";

  if (function->get_kind() == FT_CONSTRUCTOR ||
      function->get_kind() == FT_DESTRUCTOR ||
      function->get_kind() == FT_VIRTUAL_DESTRUCTOR ||
      function->get_kind() == FT_PURE_VIRTUAL_DESTRUCTOR)
    wrapped << "void " << name_args.str ().c_str ();
  else {
    wrapped << sctxt.result_type (name_args.str ());
    switch (function->get_cv_qualifiers()) {
    case CVQ_CONST:          wrapped << " const";          break;
    case CVQ_VOLATILE:       wrapped << " volatile";       break;
    case CVQ_CONST_VOLATILE: wrapped << " const volatile"; break;
    default: break;
    }
  }

  return wrapped.str ();
}

void CodeWeaver::cons_join_point (ACM_Construction *loc) {
  // if you change this check, please update Transformer::join_points too
  if (((ACM_Name*)loc->get_parent ())->get_builtin())
    gen_special_member_function (loc);
  else
    wrap_function (loc);
}

void CodeWeaver::dest_join_point (ACM_Destruction *loc) {
  if (((ACM_Name*)loc->get_parent ())->get_builtin())
    gen_special_member_function (loc);
  else
    wrap_function (loc);
}

void CodeWeaver::gen_special_member_function (ACM_Code *loc) {

  assert (((ACM_Name*)loc->get_parent ())->type_val () == JPT_Function);
  ACM_Function *func = (ACM_Function*)loc->get_parent ();
  assert (((ACM_Name*)func->get_parent ())->type_val () & (JPT_Class|JPT_Aspect));
  ACM_Class *cls = (ACM_Class*)func->get_parent ();

  // check if the special member would have to call a private member
  if (func->get_kind () == FT_CONSTRUCTOR && get_arg_count (*func) == 0) {
    if (!TI_Class::of (*cls)->may_have_implicit (TI_Class::CONSTRUCTOR))
      return;
  }
  else if (func->get_kind () == FT_CONSTRUCTOR && get_arg_count (*func) == 1) {
    // if you change this check, please update TI_Class::remember_builtin_copyconstructor_advice too
    if (!TI_Class::of (*cls)->may_have_implicit (TI_Class::COPY_CONSTRUCTOR))
      return;
  }
  else if (func->get_kind () == FT_DESTRUCTOR ||
      func->get_kind () == FT_VIRTUAL_DESTRUCTOR ||
      func->get_kind () == FT_PURE_VIRTUAL_DESTRUCTOR) {
    if (!TI_Class::of (*cls)->may_have_implicit (TI_Class::DESTRUCTOR))
      return;
  }

  ostringstream code;
  // create thisJoinPoint class
  ThisJoinPoint tjp;
  tjp.merge_flags(*loc->get_plan());
#ifdef FRONTEND_CLANG
  // only required for features in the Clang variant
  tjp.enable_entity();
#endif
  if (tjp.type_needed ()) {
    ostringstream wrappers;
    make_tjp_struct(wrappers, loc, loc->get_plan(), tjp);
    insert (TI_Class::of (*cls)->body_end_pos(*this), wrappers.str ());
  }

  // generate the function itself
  code << endl << "public:" << endl << "inline " << func->get_name();

  // constructors or destructors don't have more than one argument
  assert (get_arg_count(*func) <= 1);

  if (get_arg_count(*func) == 0) {
    // default constructor or destructor
    code << " () {" << endl;
  }
  // if you change this check, please update Transformer::join_points too
  else {
    // get information on all attributes of this class
    list<SyntacticContext> attrs;
    TI_Class::of (*cls)->get_member_contexts (attrs);

    // generate a unique wrapper class for all attributes that are arrays
    for (list<SyntacticContext>::iterator i = attrs.begin (); i != attrs.end (); ++i) {
      // if you change this check, please update CodeWeaver::is_wrapped_array too
      if ((*i).is_array () && !(*i).is_anon_union_member ())
        wrap_attribute_array (*i, attrs);
    }

    // generate the copy constructor
    ostringstream copy_cons;
    bool arg_needed = tjp.arg_needed ((ACM_Code*)loc);
    // copy the baseclass members
    bool first_initializer = true;
    typedef ACM_Container<ACM_Class, false> Bases;
    Bases &bases = cls->get_bases();
    for (Bases::iterator b = bases.begin (); b != bases.end (); ++b) {
      if (first_initializer) {
        first_initializer = false;
        copy_cons << ": ";
      }
      else
        copy_cons << ", ";
      copy_cons << signature (**b) << " (arg0)";
      arg_needed = true;
    }

    for (list<SyntacticContext>::iterator i = attrs.begin (); i != attrs.end (); ++i) {

      // make sure members of anonymous unions in this class are handled differently
      if ((*i).is_anon_union_member ())
        continue;

      // write ':' or ','
      if (first_initializer) {
        first_initializer = false;
        copy_cons << ": ";
      }
      else
        copy_cons << ", ";
      // initialize the member by copy-constructor
      copy_cons << (*i).name () << " (arg0." << (*i).name () << ")";
      arg_needed = true;
    }
    copy_cons << " {" << endl;

    // copy anonymous unions
    set<SyntacticContext> unions; // to remembers unions that are already handled
    for (list<SyntacticContext>::iterator i = attrs.begin (); i != attrs.end (); ++i) {

      // only members of anonymous unions are handled here
      if (!(*i).is_anon_union_member ())
        continue;

      SyntacticContext union_context = (*i).get_anon_union_context ();
      if (unions.find (union_context) == unions.end ()) {
        // create a named local copy of the union definition
        const int count = unions.size ();
        copy_cons << "  union __ac_union_" << count << " ";
        string u = union_context.get_as_string (1);
        copy_cons << u.substr (u.find ("{"));
        copy_cons << ";" << endl;
        copy_cons << "  *(union __ac_union_" << count << "*)&" << (*i).name ()
             << " = *(union __ac_union_" << count << "*)&arg0."
             << (*i).name () << ";" << endl;
        arg_needed = true;
        unions.insert (union_context);
      }
    }

    list<ACM_Type*> arg_types;
    get_arg_types (*func, arg_types);
    code << " (";
#ifdef FRONTEND_PUMA
      // TODO: Fix Puma-variant: better use a TransformInfo function for printing
      // e.g. "<unnamed>::classname& arg0" must be avoided!
    if (arg_needed)
      code << format_type (*arg_types.front (), "arg0");
    else
      code << format_type (*arg_types.front ());
#else
    code << TI_Code::of(*loc)->arg_type_string(0);
    if (arg_needed)
      code << " arg0";
#endif
    code << ") " << copy_cons.str ();
  }

  ACResultBuffer result_buffer (project (), loc);
  // generate __TJP type (for this level)
  if (tjp.type_needed ()) {
    code << "  typedef ";
    make_tjp_typename (code, loc, depth (*loc->get_plan()), result_buffer);
    code << " __TJP;" << endl;
  }

  // generate common JoinPoint initialization
  tjp.gen_tjp_init(code, loc, _problems, depth (*loc->get_plan()), false, 0);

  // generate calls to advice code or original function
  make_advice_calls(code, loc->get_plan(), loc);

  code << endl << "}" << endl;

  // insert the definition
  insert (TI_Class::of (*cls)->body_end_pos(*this), code.str ());
}

void CodeWeaver::wrap_attribute_array (const SyntacticContext &attr_ctxt,
    list<SyntacticContext> &attrs) {
  bool first = true;
  list<SyntacticContext>::iterator last = attrs.end ();
  for (list<SyntacticContext>::iterator i = attrs.begin (); i != attrs.end (); ++i) {
    if (attr_ctxt.objdecl_start_pos (*this) == (*i).objdecl_start_pos (*this)) {
      if (!first)
        insert ((*i).objdecl_start_pos (*this), "; ");
      else
        first = false;
      if (attr_ctxt.name () == (*i).name ())
        insert ((*i).objdecl_start_pos (*this), gen_wrapped_array(*i));
      else
        insert ((*i).objdecl_start_pos (*this), (*i).get_as_string ());
      last = i;
    }
  }
  if (last != attrs.end ())
    kill ((*last).objdecl_start_pos (*this), (*last).objdecl_end_pos (*this));
}

string CodeWeaver::gen_wrapped_array (const SyntacticContext &attr_ctxt) {
  ostringstream out;
  out << endl;
  out << "  struct __ac_wrapper_" << attr_ctxt.name () << " {" << endl;
  out << "    struct _P { typedef " << attr_ctxt.get_as_string () << "; };" << endl;
  out << "    template <typename T> struct _T {};" << endl;
  out << "    template <typename T, int D> struct _T<T[D]> { typedef T Type; };" << endl;
  out << "    typedef _P::"<< attr_ctxt.name () << " _A;" << endl;
  out << "    typedef _T<_P::" << attr_ctxt.name () << ">::Type _E;" << endl;
  // the offset of _data inside the wrapper struct has to be 0, else many things break
  // ( especially passing of member pointers on array access, but it is more on the line )
  out << "    _A _data;" << endl;
  out << "    operator _A& () const { return (_A&)_data; }" << endl;
  out << "    operator const _A& () const { return (const _A&)_data; }" << endl;
  out << "    _A* operator &() { return &_data; }" << endl;
  out << "    const _A * operator &() const { return &_data; }" << endl;
  out << "    operator void* () const { return (void*)&_data; }" << endl;
  out << "    operator const void* () const { return (const void*)&_data; }" << endl;
  out << "    template <typename I> _E& operator [] (I i) "
      << "{ return _data[i]; } // for VC++ 2003" << endl;
  out << "    template <typename I> const _E& operator [] (I i) const "
      << "{ return _data[i]; } // for VC++ 2003" << endl;
  out << "  } " << attr_ctxt.name ();
  return out.str ();
}

void CodeWeaver::exec_join_point (ACM_Execution *loc) {
  wrap_function (loc);
}

void CodeWeaver::wrap_function (ACM_Code *loc) {

  ACM_Function *function = (ACM_Function*)loc->get_parent ();
  
  const vector<SyntacticContext> &decl_contexts =
      TI_Function::of(*function)->syntactic_contexts();
  if (function->get_variadic_args ()) {
#ifdef FRONTEND_PUMA
    _err << sev_warning << decl_contexts.begin()->error_pos()->location ()
         << "can't weave execution advice for function '"
         << signature(*function).c_str() << "' with variable arguments"
         << endMessage;
#endif
         return;
    /* 
     * Functions with variable arguments require a dedicated code
     * transformation. This could be implemented via function-template
     * wrappers.
     * 
     * Example:
     * 
     * void printf(char*, ...);
     * 
     *        ||
     *        \/
     * 
     * template<typename T1>
     * void printf(char* s, T1 t1) {
     *   [...]; __exec_old_printf(s, t1); }
     * 
     * template<typename T1, typename T2>
     * void printf(char* s, T1 t1, T2 t2) {
     *   [...]; __exec_old_printf(s, t1, t2); }
     * 
     * 
     * These template wrappers need to be generated for a fixed amount
     * of parameters, which could be made configurable via a command
     * line option. For C++11, a single variadic template would be
     * sufficient.
     * 
     * However, this code transformation does not fit so well into the
     * current transformation scheme. As opposed to it, these template
     * wrapper have to be generated at each function declaration (a
     * preprocessor guard could be used to avoid multiple definitions).
     * Additionally, separate JoinPoint structures have to be generated
     * for each number of template parameters, since the proceed()
     * function needs to know the amount of arguments to pass to
     * __exec_old_printf.
     * 
     * At the functions definition, these wrappers have to be generated
     * as well, for instance if no declaration was found before. The
     * body of these template wrappers needs to be generated in both
     * cases (at each function declaration and its definition). The
     * original function just has to be renamed (__exec_old_printf).
     * 
     * Finally, the generation of the separate JoinPoint structures,
     * for each number of template parameters, would require to extend
     * the current (already complicatad) JoinPoint-structure generation.
     * 
     * Caveats:
     *  - C linkage of the the function is lost (template wrappers)
     *  - it has be ensured that no similar function template exists
     *    in the same scope (as the wrapper), which would otherwise lead
     *    to multiple definitions
     *  - gcc won't inline the __exec_old vararg function,
     *    see: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=10980
     *    -> __attribute__((always_inline)) must not be used here
     * 
     */
  }

  // handle each declaration and the optional definition separately
  // the transform info object yield the necessary syntactic context
  bool wrapped_decl = false;
  const SyntacticContext *def_context = 0;
  for (vector<SyntacticContext>::const_iterator i = decl_contexts.begin ();
      i != decl_contexts.end (); ++i) {
    if ((*i).is_definition ())
      def_context = &(*i);
    else {
      if (wrap_function_decl (loc, *i))
        // friend declarations have an assigned scope. They are no real
        // declarations. Another declaration has to be generated.
        if (!(*i).has_assigned_scope ())
          wrapped_decl = true;
    }
  }
  // if a definition has been found, wrap it
  if (def_context)
    wrap_function_def (loc, *def_context, wrapped_decl);

}

static string close_extern_c_block (int extern_c_depth) {
  ostringstream result;
  for (int i = 0; i < extern_c_depth; i++)
    result << "} ";
  result << endl << endl;
  return result.str ();
}

static string reopen_extern_c_block (int extern_c_depth) {
  ostringstream result;
  result << endl;
  for (int i = 0; i < extern_c_depth; i++)
    result << "extern \"C\" { ";
  result << endl;
  return result.str ();
}

// create a new function implementation and paste it behind the original
void CodeWeaver::wrap_function_def (ACM_Code *loc, const SyntacticContext &sctxt,
  bool wrapped_decl) {

  ACM_Function *function = (ACM_Function*)loc->get_parent ();
  ostringstream pre_sig, pre_body;

  bool have_pre_sig = false;
  pre_sig << endl;

  // the first is the declaration of the inner function (the original)
  // not necessary for methods and if there is a declaration anyway
  if (!wrapped_decl &&
      (function->get_kind() == FT_NON_MEMBER ||
      function->get_kind() == FT_STATIC_NON_MEMBER)) {
    have_pre_sig = true;
    pre_sig << wrapper_function_signature (loc, sctxt, true) << ";" << endl;
  }

  // generate the JoinPoint class
  ThisJoinPoint tjp;
  tjp.merge_flags(*loc->get_plan());
#ifdef FRONTEND_CLANG
  // only required for features in the Clang variant
  tjp.enable_entity();
#endif

  if (tjp.type_needed ()) {
    have_pre_sig = true;
    int extern_c_depth = sctxt.is_in_extern_c_block();
    if (extern_c_depth > 0)
      pre_sig << close_extern_c_block(extern_c_depth);
    make_tjp_struct(pre_sig, loc, loc->get_plan(), tjp);
    if (extern_c_depth > 0)
      pre_sig << reopen_extern_c_block(extern_c_depth);
  }

  // paste pre-signature code in front of the old function signature
  if (have_pre_sig) // check only for better readability
    insert (sctxt.wrapper_pos(*this), pre_sig.str ());

  // rename the arguments of the original declaration
  vector<string> arg_names;
  rename_args (sctxt, "arg", arg_names);

  // if the function is the "main()" function and if it does not contain a
  // return statement at the end, generate one ("return 0;")
  if (function->get_name() == "main" &&
      signature(*(ACM_Name*)function->get_parent()) == "::" &&
      !sctxt.ends_with_return())
    insert (sctxt.fct_body_end_pos (*this), "return 0;\n");
  // generate the body of the wrapper function
  ACResultBuffer result_buffer (project (), loc);

  pre_body << "{" << endl;

#ifdef FRONTEND_CLANG
  bool have_Member = needs_this( *function );
  // feature is supported in Clang only
  // compute/generate missing parameters
  // typedefs for missing types first ...
  if (tjp.entity_needed(loc) || tjp.memberptr_needed(loc)) {
    ostringstream out;
    out << "TEntity (";
    for( int a = 0; a < get_arg_count(*function); a++ ) {
      if( a > 0 )
        out << ",";
      out << sctxt.arg_type (a);
    }
    out << ")";
    pre_body << "  typedef " << sctxt.result_type (out.str()) << ";" << endl;

    bool init_needed = false;
    if (have_Member) {
      if (tjp.memberptr_needed(loc)) {
        pre_body << "  TEntity " << name_qualifier( function ) << "*member = &";
        init_needed = true;
      }
    }
    else {
      if (tjp.entity_needed(loc)) {
        pre_body << "  TEntity &ent = ";
        init_needed = true;
      }
    }
    if (init_needed)
      pre_body << name_qualifier( function ) << function->get_name() << ";" << endl;
  }
#endif

  // generate __TJP type (for this level)
  if (tjp.type_needed ()) {
    pre_body << "  typedef ";
    make_tjp_typename (pre_body, loc, depth (*loc->get_plan()), result_buffer);
    pre_body << " __TJP;" << endl;
  }

  // declare the result object
  pre_body << "  " << result_buffer.result_declaration ();

  // generate common JoinPoint initialization
  tjp.gen_tjp_init(pre_body, loc, _problems, depth (*loc->get_plan()),
      false, &arg_names);

  // generate calls to advice code or original function
  make_advice_calls (pre_body, loc->get_plan(), loc, false, &arg_names);

  pre_body << "  " << result_buffer.result_return ();
  pre_body << endl << "}" << endl;

  // generate the signature of the inner function (the original)
  // special treatment for operator new/delete/new[]/delete[]
  if (function->get_kind () == FT_MEMBER && !needs_this(*function) && !wrapped_decl)
    pre_body << "static ";
  // add a wrapper function declaration
  pre_body << wrapper_function_signature (loc, sctxt, true);

  // insert the new body and new signature in front of the old body
  insert(sctxt.fct_body_start_pos (*this), pre_body.str ());
}


// create a new declaration for a wrapped function
bool CodeWeaver::wrap_function_decl (ACM_Code *loc, const SyntacticContext &sctxt) {

  ACM_Function *function = (ACM_Function*)loc->get_parent ();

  // check if the function declaration belongs to the project and is not built-in
  // if it does not, we cannot insert the wrapper declaration
  if (!sctxt.is_in_project())
    return false;

  string inner_decl;
  if (sctxt.is_in_class_scope ())
    inner_decl += "public: ";
  // special treatment for friend declarations
  if (sctxt.has_assigned_scope ())
    inner_decl += "friend ";
  // special treatment for operator new/delete/new[]/delete[] => implicitly static
  if (function->get_kind () == FT_MEMBER && !needs_this(*function))
    inner_decl += "static ";
  // add the wrapper function declaration
  inner_decl += wrapper_function_signature (loc, sctxt, false);
  inner_decl += ";";
  // switch to the right protection if needed
  inner_decl += "\n";
  if (sctxt.protection() != "" && sctxt.protection() != "public")
    inner_decl += sctxt.protection() + ":\n";

  // move the generated declaration in front of the original declaration
  insert (sctxt.wrapper_pos(*this), inner_decl);

  return true;
}


void CodeWeaver::make_advice_call(ostream &out, ACM_Code *loc,
                                  AdviceInfo *ai, bool inter, int depth) {
  assert((loc->type_val () & JPT_Access) || loc->type_val () == JPT_Execution ||
          loc->type_val () == JPT_Construction || loc->type_val () == JPT_Destruction );

  // Get the correct JoinPoint object:
  PointCut &pc = ai->pointcut ();
  PointCut::iterator jp_iter = pc.find (loc);
  assert (jp_iter != pc.end ());
  const JoinPoint &jp = *jp_iter;

  // Name of the TJP-instance:
  ostringstream tjp_instance_name;
  Naming::tjp_instance(tjp_instance_name, loc);

  // Determine helper strings:
  string tjp_tp = "__TJP";
  string tjp_obj = inter ? "this" : "&" + tjp_instance_name.str();

  // If there should be a condition, then insert the check:
  string this_name;
  string tjp_access_prefix;
  if (jp.condition ()) {
    // Determine helper strings:
    if(inter) {
      tjp_access_prefix = tjp_tp + "::";

      this_name = tjp_access_prefix + "_that";
    }
    else {
      tjp_access_prefix = tjp_instance_name.str() + ".";

      if (loc->type_val () & JPT_Access) {
        this_name = "srcthis";
      }
      else { // Execution, Construction or Destruction
        this_name = "this";
      }
    }
    // Insert check:
    out << "  if(";
    jp.condition ().gen_check (out, ai, this_name.c_str(), &tjp_access_prefix);
    out << ") {" << endl << "  ";
  }

  // Generate the call that invokes the advice code
  out << "  ";
  ai->gen_invocation_func_call (out, tjp_tp.c_str(), tjp_obj.c_str());
  out << endl;

  // Insert end of condition check
  if (jp.condition ()) {
    out << "  }" << endl;
    if (ai->type () == ACT_AROUND) {
      // Make sure the following advice get invoked even if
      // the condition is not fulfilled
      out << "  else {" << endl;
      out << "    " << tjp_access_prefix << "proceed ();" << endl;
      out << "  }" << endl;
    }
  }
}


void CodeWeaver::make_advice_calls (ostream &out, ACM_CodePlan *plan,
    ACM_Code *loc, bool inter, vector<string> *arg_names) {

  typedef ACM_Container<ACM_CodeAdvice, true> Container;
  Container &before = plan->get_before();
  for (Container::iterator i = before.begin (); i != before.end (); ++i)
    make_advice_call (out, loc, TI_CodeAdvice::of(**i)->get_advice_info(), inter, depth (*plan));

  if (plan->has_around())
    make_advice_call (out, loc, TI_CodeAdvice::of(*plan->get_around())->get_advice_info(), inter, depth (*plan));
#ifdef FRONTEND_CLANG
  // this feature is only supported in the Clang variant
  else if( TI_Code::of( *( static_cast<ACM_Code *>( loc ) ) )->has_implicit_joinpoints() )
    make_proceed_to_implicit( out, loc, inter, arg_names );
#endif
  else
    make_proceed_code (out, loc, inter, arg_names);

  Container &after = plan->get_after();
  for (Container::iterator i = after.begin (); i != after.end (); ++i)
    make_advice_call (out, loc, TI_CodeAdvice::of(**i)->get_advice_info(), inter, depth (*plan));
}


// this function weaves the invocation code for a tree of advice functions
// at a access join point
void CodeWeaver::access_join_point( ACM_Access *loc ) {
#ifdef FRONTEND_CLANG
  TI_Access &ti = *( TI_Access::of( *loc ) );
  const SyntacticContext &sctxt = ti.access_context();
#else
  // fall back to call API as generalisation is not done for Puma
  assert( loc->type_val() == JPT_Call );
  TI_MethodCall &ti = *( TI_MethodCall::of( *( static_cast<ACM_Call *>( loc ) ) ) );
  const SyntacticContext &sctxt = ti.call_context();
#endif

  typedef const WeavePos WP;

  // determine the weaving position for wrapper code
  int extern_c_depth = sctxt.is_in_extern_c_block();
  const WeavePos &pos = sctxt.wrapper_pos( *this );

  // close extern "C" block if we are in any
  if( extern_c_depth > 0 ) {
    ostringstream cblock;
    cblock << close_extern_c_block( extern_c_depth );
    insert( pos, cblock.str() );
  }

#ifdef FRONTEND_CLANG
  // Weaving on operators with short-circuit-evaluation only available with Clang-frontend.
  if( loc->type_val() == JPT_Builtin && static_cast<TI_Builtin&>( ti ).is_short_circuiting() ) {
    // Special handling for short-circuit-evaluating operators:
    splice_shortcircuit_expr( static_cast<ACM_Builtin *>( loc ) );

    if( extern_c_depth > 0 ) {
      ostringstream cblock;
      cblock << reopen_extern_c_block( extern_c_depth );
      insert( pos, cblock.str() );
    }
    return; // we are done
  }
#endif // FRONTEND_CLANG

  // create the wrapper
  stringstream new_call;
  stringstream trail_call;
  bool generated_args = CodeWeaver::generate_access_wrapper( loc, new_call, trail_call );

  if( extern_c_depth > 0 ) {
    ostringstream cblock;
    cblock << reopen_extern_c_block( extern_c_depth );
    insert( pos, cblock.str() );
  }

  // replace call with call to replacement function
  WP &before_pos = ti.before_pos(*this);
  WP &after_pos  = ti.after_pos(*this);

  // provide wrapper call arguments by transforming the expression
  bool provided_args = splice_access_expr( loc );

  if( generated_args && provided_args )
    new_call << ", ";

  // paste the trailing part of the wrapper call (in back)
  insert (after_pos, trail_call.str ());

  // paste the first (generated) part of the call (in front)
  insert (before_pos, new_call.str (), true);
}

#ifdef FRONTEND_CLANG
void CodeWeaver::splice_shortcircuit_expr( ACM_Builtin *loc ) {
  // Weaving on operators with short-circuit-evaluation only available with Clang-frontend.
  TI_Builtin &ti = *( TI_Builtin::of( *loc ) );
  assert( ti.is_short_circuiting() );

  // Transform the original expr
  // "a X1 b X2 c" (X1 and X2 are placeholders for operators) will be transformed to:
  //   (( a ) ? call_0(<other args>, true, ( b ) , <deduce>) : call_1(<other args>, false, ( c ), <deduce>))
  //   ^1^  ^--------------2---------------^   ^--------------------3----------------------^   ^-----4-----^
  //   with 1 = new_call, 2 and 3 = replacements for operators X1 and X2, 4 = trail_call,
  //     <other args> = args for target, that,..., <deduce> = dummy argument for result type deduction

  // Different cases and how they match to "a X1 b X2 c":
  // "d && e":    a = d, b = e, c = false /*dummy*/
  // "d || e":    a = d, b = true /*dummy*/, c = e
  // "a ? b : c": ...

  // If the second argument is not available, we will set the dummy to the value of a, so we can
  // return arg0 inside the wrapper in every case.

  // Parenthesis around the second argument (b) are necessary in the case of the ternary operator,
  // because most(?) compilers ignore the operator precedence while scanning for the ":".
  // Thus e.g. "a ? 1, 2 : 3" otherwise would lead to wrong code ("call_0(<other args>, 1, 2, <deduce>)").

  // TODO:
  // If the user changes the first argument in an advice, the generation of the result will lead to the
  // dereference of a null-pointer, because a not available argument will be accessed. To reduce
  // this problem, we plan to add the "const"-qualifier to the type of the first argument, if it is
  // accessed inside the advice.

  // Determine the parts 1 to 4 and create the wrappers
  ostringstream first_part, second_part, third_part, fourth_part;
  first_part << "((";
  second_part << ") ? ";
  third_part << ")";

  // create the frist wrapper
  bool generated_args = CodeWeaver::generate_access_wrapper( loc, second_part, third_part, "this", 0 );

  second_part << ( generated_args ? ", " : "" ) << "true, (";
  third_part << " : ";
  fourth_part << ")";

  // Generate second wrapper:
  generated_args = CodeWeaver::generate_access_wrapper( loc, third_part, fourth_part, "this", 1 );

  third_part<< ( generated_args ? ", " : "" ) << "false, (";
  fourth_part << ")";

  // provide wrapper call arguments by transforming the expression
  if( clang::BinaryOperator * bo = llvm::dyn_cast<clang::BinaryOperator>( ti.TI_Access::tree_node() ) ) {
    if( bo->getOpcode() == BO_LAnd ) {
      // Set fourth part to third + dummy + fourth:
      third_part << "(false /* dummy */)" << fourth_part.str();
      // move content to fourth_part
      fourth_part.clear();
      fourth_part.str( third_part.str() );
      fourth_part.seekp( 0, std::ios_base::end );
    }
    else if( bo->getOpcode() == BO_LOr ) {
      // Set second_part to second + dummy + third:
      second_part << "(true /* dummy */)" << third_part.str();
    }
    else {
      assert(false && "Unknown short-circuit-operator");
      return;
    }
    // Use second part as replacement for && or ||
    replace( ti.op_before_pos( *this ), ti.op_after_pos( *this ), second_part.str() );
  }
  else if( llvm::isa<clang::ConditionalOperator>( ti.TI_Access::tree_node() ) ) {
    replace( ti.ternary_op_first_delim_before_pos( *this ), ti.ternary_op_first_delim_after_pos( *this ), second_part.str() );
    replace( ti.ternary_op_second_delim_before_pos( *this ), ti.ternary_op_second_delim_after_pos( *this ), third_part.str() );
  }
  else {
    assert(false && "Unknown short-circuit-operator");
    return;
  }

  // paste the trailing part of the wrapper call (in back)
  insert( ti.after_pos(*this), fourth_part.str() );

  // paste the first (generated) part of the call (in front)
  insert( ti.before_pos(*this), first_part.str(), true );
}
#endif // FRONTEND_CLANG

// generate wrapper body and call
//
//   wrapper_number: Number of the wrapper this member-function will generate. Currently only
//                   needed for short-circuit-evaluation. This argument is optional and defaults
//                   to -1, which indicates that the wrapper-number should not be used.
bool CodeWeaver::generate_access_wrapper( ACM_Access *loc, ostream &new_call, ostream &trail_call,
    const char *name_that, int wrapper_number) {
  // determine the weaving parameters
  ACM_CodePlan &plan = *( loc->get_plan() );

#ifdef FRONTEND_CLANG
  TI_Access &ti = *( TI_Access::of( *loc ) );
  const SyntacticContext &sctxt = ti.access_context();
  ACM_Name *src_obj = get_explicit_parent( *loc );
#else
  assert( loc->type_val() == JPT_Call );
  TI_MethodCall &ti = *( TI_MethodCall::of( *( static_cast<ACM_Call *>( loc ) ) ) );
  const SyntacticContext &sctxt = ti.call_context();
  ACM_Name *src_obj = static_cast<ACM_Name *>( loc->get_parent() );
#endif
  ACM_Function *src_func = ((src_obj->type_val() == JPT_Function) ?
      (ACM_Function*)src_obj : (ACM_Function*)0);
  ACM_Name *dst_entity = 0;
  if( has_entity( *loc ) )
    dst_entity = get_entity( *loc );
  ACM_Function *dst_func = 0;
  if( has_entity_func( *loc ) )
    dst_func = get_entity_func( *loc );
#ifdef FRONTEND_CLANG
  // only required for features in the Clang variant
  ACM_Variable *dst_var = 0;
  if( has_entity_var( *loc ) )
    dst_var = get_entity_var( *loc );
#endif
  bool deduce_result_type = ti.has_result() && ! is_implicit( *loc ); // TODO: don't do it always, Markus: for implicit joinpoint don't do it, as codecopy is not reliable there
  bool override_isdep = is_implicit( *loc );
#ifdef FRONTEND_CLANG
  if( loc->type_val() == JPT_Ref && static_cast<TI_Ref &>( ti ).is_implicit_arraydecay() ) {
    deduce_result_type = false; // for JPs on implicit AST nodes, codecopy does not work as the do not exist in the copied code
    override_isdep = true;
  }
#endif

  // determine the weaving position for wrapper code
  const WeavePos &pos = sctxt.wrapper_pos(*this);

  ostringstream code;

  // generate and insert a forward declaration for the target function if needed:
  // direct recursive call of a non-member function that has no forward declaration
  if (src_func && src_func == dst_func &&
      (dst_func->get_kind() == FT_NON_MEMBER ||
      dst_func->get_kind() == FT_STATIC_NON_MEMBER) &&
      dst_func->get_source().size() == 1) {
    // TODO: a forward declaration has to be generated and default argument
    //       initializers have to be moved into the forward declaration
//    cout << "FORWARD" << endl;
  }

  ThisJoinPoint tjp;
  tjp.merge_flags( plan );
#ifdef FRONTEND_CLANG
  // only required for features in the Clang variant
  tjp.enable_entity();

  if( ti.has_implicit_joinpoints() )
    tjp.merge_implicit( *loc );
#endif

  bool proceed_generated = tjp.proceed() || tjp.action();

  // assure prerequisites are meet if non direct proceed code is used
  if( ti.needs_rights() ) {
    // generate JoinPoint-ID-Class and caller function of necessary
    if( loc->type_val() == JPT_Call )
      insert_id_class( static_cast<ACM_Call *>( loc ), src_obj, pos, *dst_func );
    else {
      if( proceed_generated ) // only request the entity() function when a proceed() func exists where we need it
        tjp.require_entity();
    }
  }

  bool pass_That = src_func && needs_this(*src_func) && tjp.that_needed( loc );

#ifdef FRONTEND_CLANG
  // Clang Variant only features
  bool proceed_uses_entity = dst_var && ti.needs_rights();

  bool wormhole = false;
  bool pack_Arg0 = false;
  bool pack_Result = false;
  int skip_Arg = -1;

  if( loc->type_val() == JPT_Builtin ) {
    TI_Builtin &tiC = static_cast<TI_Builtin &>( ti );

    // guarding against manipulation of return value on forwarding operators, as this would break various assumptions in the weaver
    if( tiC.is_forwarding() )
      tjp.force_const_result();
    // guard the first arg of array subscript too, as it ight be a pointer to an array
    if( tiC.is_index_expr() )
      tjp.force_const_arg( 0 );
    // guard the first arg of short-circuit operators, as it decides which args are valid
    if( tiC.is_short_circuiting() )
      tjp.force_const_arg( 0 );

    // pack arg0 and result if needed
    if( tiC.forwarded_is_packed() ) {
      pack_Arg0 = true;
      pack_Result = true;

      if( tiC.forwarded_needs_packing() )
        pack_entity_expr( tiC.packed_forward_requester() );
    }

    // skip the unevaluated arg for conditional operators
    if( llvm::isa<clang::ConditionalOperator>( tiC.TI_Access::tree_node() ) ) {
      assert( tiC.is_short_circuiting() );
      assert( wrapper_number == 0 || wrapper_number == 1 );
      skip_Arg = 2 - wrapper_number;
    }
  }

  if( ti.has_implicit_joinpoints() ) {
    // it is suffient to consider only the first, as all share the same entity
    assert( loc->type_val() == JPT_Builtin );
    ACM_Access *implicit = *( static_cast<ACM_Builtin *>( loc )->get_implicit_access().begin() );
    if( has_complex_entity( implicit ) ) {
      if( ! pack_Arg0 ) { // it might be already packed
        pack_Arg0 = true;

	if( static_cast<TI_Builtin &>( ti ).forwarded_needs_packing() )
          pack_entity_expr( implicit );
      }

      // if we have implicit joinpoint calls inside a proceed function
      // that have a complex entity, we need to tunnel the information in
      if( proceed_generated ) {
        tjp.request_wormhole();
        wormhole = true;
      }
    }
  }
#endif

  bool have_Member = dst_entity && needs_this( *dst_entity );
  bool this_Target = have_Member && ! ti.has_target_expr();
  bool have_Target = ti.has_target_expr() || this_Target;
#ifdef FRONTEND_CLANG
  // Clang Variant only features
  bool have_Entity = ! (
                   ( dst_func && needs_this( *dst_func ) ) // member functions are not representable as simple function pointers
                || ( loc->type_val() == JPT_Builtin ) // for builtin operators there is no function to point to
                );
  bool have_Idx = ti.entity_index_count() > 0;
  bool have_Base = have_Idx;

  if( have_Idx && proceed_generated && ! proceed_uses_entity ) // request api generation for use in proceed
    tjp.require_idx();

  bool need_Entity = have_Entity && ( proceed_uses_entity || tjp.entity_needed( loc ) );
#endif
  // proceed needs Target if it reuses the target_expr or to access member entities
  // Exception: it is not used to access private data members (entity is used then)
  // tjp needs it only if entity is a member
#ifdef FRONTEND_CLANG
  bool need_Target = have_Target && ( ! proceed_uses_entity || tjp.target_needed( loc ) );
  bool need_Member = have_Member && tjp.memberptr_needed( loc );
  bool need_Base = have_Base && tjp.array_needed( loc );
  bool need_Idx = have_Idx && ( ! proceed_uses_entity || tjp.idx_needed( loc ) );
#else
  bool need_Target = have_Target && tjp.target_needed( loc );
#endif

#ifdef FRONTEND_CLANG
  // Clang Variant only features
  bool pack_Entity = false;
  if( has_complex_entity( loc ) ) {
    if( is_implicit( *loc ) )
      pack_Entity = true;
    else {
      TI_Access::SpliceMode mode = ti.entity_src_splice_mode();
      if( mode == TI_Access::SpliceMode_Packed || mode ==  TI_Access::SpliceMode_Pack )
        pack_Entity = true;
    }
  }
#endif

  bool pass_Target = false;
#ifdef FRONTEND_CLANG
  // Clang Variant only features
  bool pass_Entity = false;
  bool comp_Entity = false;
  bool genr_Entity = false;
  bool pass_Member = false;
  bool genr_Member = false;
  bool pass_Base = false;
  bool comp_Base = false;
  bool pass_Idx = false;

  if( dst_func ) { // For historical reasons and some specifics functions are handled a bit differently
    pass_Target = have_Target; // we need to pass it, even if we don't need it, as the expr exists already
    if( have_Member ) // for member functions ...
      genr_Member = need_Member; // .. generate the member info as needed, there is no entity defineable
    else // .. for normal functions
      genr_Entity = need_Entity; // .. generate the entity info as needed, there is no member
  }
  else if( ! pack_Entity ) {
    // decide from where to get Entity:
    if( have_Base && have_Idx ) {
      comp_Entity = need_Entity; // compute as needed
      if( comp_Entity ) { // request deps
        need_Base = true;
        need_Idx = true;
      }
    }
    else if( have_Target && have_Member ) {
      comp_Entity = need_Entity; // compute as needed
      if( comp_Entity ) { // request deps
        need_Target = true;
        need_Member = true;
      }
    }
    else
      pass_Entity = have_Entity; // we need to pass it, even if we don't need it, as the expr exists already

    // decide from where to get Base:
    if( have_Target && have_Member ) {
      comp_Base = need_Base; // compute as needed
      if( comp_Entity ) { // request deps
        need_Target = true;
        need_Member = true;
      }
    }
    else
      pass_Base = have_Base; // we need to pass it, even if we don't need it, as the expr exists already
    // Idx:
    pass_Idx = have_Idx; // we need to pass it, even if we don't need it, as the expr exists already

    pass_Target = have_Target; // we need to pass it, even if we don't need it, as the expr exists already
    pass_Member = have_Member; // we need to pass it, even if we don't need it, as the expr exists already
  }
  // packed entities contain all, we don't need to pass, compute or generate anything
#else // PUMA is simple ...
  pass_Target = have_Target; // we need to pass it, even if we don't need it, as the expr exists already
#endif

  // consistency checks
#ifdef FRONTEND_CLANG
  // Clang Variant only features
  if( ! pack_Entity ) {
    assert( ! need_Entity || ( pass_Entity || comp_Entity || genr_Entity ) );
    assert( 1 >= ( pass_Entity + comp_Entity + genr_Entity ) ); // allow maximum one of the options
#endif
    assert( ! need_Target || pass_Target );
#ifdef FRONTEND_CLANG
  // Clang Variant only features
    assert( ! need_Member || ( pass_Member || genr_Member ) );
    assert( 1 >= ( pass_Member + genr_Member ) ); // allow maximum one of the options
    assert( ! need_Base || ( pass_Base || comp_Base ) );
    assert( 1 >= ( pass_Base + comp_Base ) ); // allow maximum one of the options
    assert( ! need_Idx || pass_Idx );
  }
  else {
    assert( ! ( pass_Entity || comp_Entity || genr_Entity ) );
    assert( ! pass_Target );
    assert( ! ( pass_Member || genr_Member ) );
    assert( ! ( pass_Base || comp_Base ) );
    assert( ! pass_Idx );
  }

  bool entity_is_const = have_Entity && ti.entity_is_const();
#endif

  // generate and insert the JoinPoint class
  // If this is the generation of the second wrapper, the struct was already inserted.
  if( tjp.type_needed() && wrapper_number < 1 ) {
    // insert the definition
    code << string ("/* ") + signature (*src_obj) + " -> " + signature (*loc) + " */\n";
    make_tjp_struct (code, loc, &plan, tjp);
  }

  // generate replacement function for dstfunc
  code << endl;

  // argument list
  int arg_count = get_arg_count (*loc);
#ifdef FRONTEND_CLANG
  unsigned int idx_count = ti.entity_index_count();
#endif
  bool generated_args = false;

#ifdef FRONTEND_CLANG
  bool wrapper_is_template = ( ti.has_result() || pass_That || pass_Target || pass_Entity || pass_Base || pass_Idx || pack_Entity || arg_count > 0 );
#else
  bool wrapper_is_template = ( ti.has_result() || pass_That || pass_Target || arg_count > 0 );
#endif
  if (wrapper_is_template) {
    code << "template <";
    int targs = 0;
    if( pass_That ) {
      if (targs > 0)
        code << ", ";
      code << "typename TSrc";
      targs++;
    }
    if (arg_count > 0) {
      for (int a = 0; a < arg_count; a++, targs++) {
        if (targs > 0)
          code << ", ";
        code << "typename TArg" << a;
      }
    }
    if (ti.has_result ()) {
      if (targs > 0)
        code << ", ";
      code << "typename TResult";
      targs++;
    }
    if( pass_Target ) {
      if (targs > 0)
        code << ", ";
      code << "typename TTarget";
      targs++;
    }
#ifdef FRONTEND_CLANG
    // this feature is only available in the Clang variant
    if( pass_Entity || pass_Member || pass_Base ) {
      if( targs > 0 )
        code << ", ";
      code << "typename TEntity";
      targs++;
    }
    if( pass_Idx ) {
      for( unsigned int i = 0; i < idx_count; i++, targs += 2 ) {
        if( targs > 0 )
          code << ", ";
        code << "unsigned int TDim" << i;
        code << ", typename TIdx" << i;
      }
    }
    if( pack_Entity || pack_Arg0 ) {
      if( targs > 0 )
        code << ", ";
      code << "typename TPacked";
      targs++;
    }
#endif
    code << ">" << endl;
  }

  // if the lexical scope in which the associated object of the call was
  // defined is a class scope (not global or namespace) the function definition
  // has to be preceded by a 'static' keyword to avoid an implicit 'this'.
  if (sctxt.is_in_class_scope())
      code << "static ";
  if (_problems._use_always_inline) // GNU extension
    code << "__attribute__((always_inline)) ";
  code << "inline ";

  // add result type
#ifdef FRONTEND_CLANG
  // this feature is only available in the Clang variant
  if( pack_Result )
    gen_packed_resulttype( code, loc );
  else {
#endif
    code << (ti.has_result() ? "TResult " : "void ");
#ifdef FRONTEND_PUMA
    CTypeInfo *result_type = ti.result_type_info ();
    if (result_type->isAddress()) {
      if (result_type->BaseType ()->isConst())
#else
    const clang::Type *type = ti.result_type().getTypePtr();
    if (type->isReferenceType ()) {
      const clang::ReferenceType *ref_type = clang::dyn_cast<clang::ReferenceType>(type);
      if (ref_type->getPointeeType ().isConstQualified ())
#endif
        code << "const ";
      code << "&";
    }
#ifdef FRONTEND_CLANG
  }
#endif

  // add function name
  Naming::access_wrapper (code, loc, depth (plan), wrapper_number);
  code << " (";
  Naming::access_wrapper (new_call, loc, depth (plan), wrapper_number);

  if (wrapper_is_template) {
    new_call << "< ";
    int targs = 0;
    if( pass_That ) {
      if (targs++ > 0)
        new_call << ", ";
      string scope = string("::") + signature(*cscope (src_func));
      new_call << cleanup_name (scope) << "*";
    }
    if (arg_count > 0) {
      // argument types
      list<ACM_Type*> arg_types;
      get_arg_types (*loc, arg_types);
      int a = 0;
      for( list<ACM_Type*>::iterator i = arg_types.begin(); i != arg_types.end(); ++i, ++a ) {
        if (targs++ > 0)
          new_call << ", ";
#ifdef FRONTEND_PUMA
        CTypeInfo *type = TI_Type::of (**i)->type_info ();
        type->TypeText (new_call, "", true, true);
#else
        //new_call << format_type (**i);
        new_call << ti.arg_type_string( a );
#endif
      }
    }
    if (ti.has_result () && !deduce_result_type) {
      if (targs++ > 0)
        new_call << ", ";
#ifdef FRONTEND_PUMA
      CTypeInfo *t = result_type;
      if (t->isAddress()) t = t->BaseType ();
      bool is_const = false;
      if (t->TypeQualified() && t->TypeQualified()->isConst()) {
        is_const = true;
        t->TypeQualified()->isConst(false);
      }
      t->TypeText(new_call, "", true, true);
      if (is_const)
        t->TypeQualified()->isConst(true);
#else
      // use function in TI
      new_call << ti.result_type_string();
#endif
    }
    new_call << " >";
  }
  new_call << " (";

  unsigned argnum = 0;

  // source and destination this pointer arguments (if needed)
  if( pass_That ) {
    string scope = string("::") + signature(*cscope (src_func));
//    string scope = string("::") + cscope (srcfunc)->QualName();
    code << "TSrc srcthis";
    new_call << "(" << cleanup_name (scope) << "*)" << name_that;
    generated_args = true;
    argnum++;
  }

  if( pass_Target ) {
    if (argnum > 0)
      code << ", ";
    code << "const TTarget &dst";

    // move/paste the target object pointer
    if( this_Target && dst_func ) { // FIXME this should be done in entity splicing or other central stage
      if( argnum > 0 )
        new_call << ", ";
      new_call << "*this";
      generated_args = true;
    }

    argnum++;
  }

#ifdef FRONTEND_CLANG
  // feature is supported in Clang only
  if( pack_Entity ) {
    if( argnum > 0 )
      code << ", ";
    code << "TPacked __packed";
    argnum++;
  }

  if( pass_Entity || pass_Member || pass_Base ) {
    if( argnum > 0 )
      code << ", ";
    if( entity_is_const )
      code << "const ";
    code << "TEntity ";
    if( pass_Entity )
      code << "&ent";
    else {
      if( pass_Member )
        code << "(TTarget::* member)";
      else // pass_Base
        code << "(&base)";

      for( unsigned int i = 0; i < idx_count; i++ )
        code << "[TDim" << i << "]";
    }
    argnum++;
  }

  if( pass_Idx )
    for( unsigned int i = 0; i < idx_count; i++ ) {
      if( argnum > 0 )
        code << ", ";
      code << "TIdx" << i << " idx" << i;
      argnum++;
    }
#endif

  // argument list
  for (int a = 0 ; a < arg_count; a++) {
#ifdef FRONTEND_CLANG
    if( a == skip_Arg ) {
      code << " /* Skipped: TArg" << a << " arg" << a << " */ ";
      continue;
    }
#endif
    if (argnum > 0)
      code << ", ";
#ifdef FRONTEND_CLANG
    // feature is supported in Clang only
    if( a == 0 && pack_Arg0 )
      code << "TPacked __packed";
    else
#endif
      code << "TArg" << a << " arg" << a;
    argnum++;
  }

  if (deduce_result_type) {
    if (argnum > 0)
      code << ", ";
    code << "AC::RT<TResult>";
    argnum++;
  }
  code << ")";

  ACResultBuffer result_buffer (project (), loc, wrapper_is_template);

  // replacement function body
  code << "{" << endl;

  // --- wrapper-body-begin ---

#ifdef FRONTEND_CLANG
  // feature is supported in Clang only
  // compute/generate missing parameters
  // typedefs for missing types first ...
  if( genr_Entity || genr_Member ) {
    code << "  typedef " << ( ti.has_result() ? "TResult" : "void " ) << " TEntity";
    if( dst_func ) {
      code << "(";
      for( int a = 0; a < arg_count; a++ ) {
        if( a > 0 )
          code << ",";
        code << "TArg" << a;
      }
      code << ")";
    }
    code << ";" << endl;
  }

  // ... parameter emulation second
  if( genr_Entity || genr_Member ) {
    if( genr_Entity )
      code << "  TEntity &ent = ";
    else { // genr_Member
      code << "  ";
      if( entity_is_const )
        code << "const ";
      code << "TEntity TTarget::*member = &";
    }
    code << name_qualifier( dst_entity ) << dst_entity->get_name() << ";" << endl; 
  }
  if( comp_Base ) {
    assert( ! pass_Base ); // Be sure it is not already there
    code << "  TEntity (&base)";
    for( unsigned int i = 0; i < idx_count; i++ )
      code << "[TDim" << i << "]";
    code << " = const_cast<TTarget &>( dst ).*member;" << endl;
  }
  if( comp_Entity ) {
    assert( ! pass_Entity ); // Be sure it is not already there
    assert( have_Member || have_Base ); // Be sure one of the src options exists
    code << "  TEntity &ent = ";
    if( have_Base ) {
      code << "base";
      for( unsigned int i = 0; i < idx_count; i++ )
        code << "[idx" << i << "]";
    }
    else // have_Member
      code << "const_cast<TTarget &>( dst ).*member";
    code << ";" << endl;
  }

  // ... or do on block unpacking
  if( pack_Entity ) {
    assert( ! ( pass_Entity || comp_Entity || genr_Entity ) ); // Be sure it is not already there
    if( have_Entity )
      code << "  typedef typename TPacked::Entity TEntity;" << endl;
    if( need_Entity )
      code << "  TEntity &ent = __packed.entity();" << endl;
    assert( ! pass_Target ); // Be sure it is not already there
    if( have_Target )
      code << "  typedef typename TPacked::BI::Target TTarget;" << endl;
    if( need_Target ) {
      code << "  TTarget &dst = __packed._base._target;" << endl;
    }
    assert( ! ( pass_Member || genr_Member ) ); // Be sure it is not already there
    if( need_Member )
      code << "  " << ( have_Base ? "typename TPacked::Base" : "TEntity" ) << " TTarget::*member = __packed._base._member;" << endl;
    assert( ! ( pass_Base || comp_Base ) ); // Be sure it is not already there
    if( need_Base )
      code << "  typename TPacked::Base &base = __packed.base();" << endl;
    assert( ! pass_Idx ); // Be sure it is not already there
    if( have_Idx )
      for( unsigned int i = 0; i < idx_count; i++ )
        code << "  typedef typename TPacked::template Dim<" << i << ">::Idx TIdx" << i << ";" << endl;
    if( need_Idx )
      for( unsigned int i = 0; i < idx_count; i++ )
        code << "  TIdx" << i << " &idx" << i << " = __packed.template idx<" << i << ">();" << endl;
  }

  // unpacking of packed operator argument
  if( pack_Arg0 )
    code << "  TArg0 arg0 = __packed.entity();" << endl;

  // emulate unevaluated param for ?:
  if( loc->type_val() == JPT_Builtin ) {
    TI_Builtin& tiB = static_cast<TI_Builtin&>( ti );
    if( llvm::isa<clang::ConditionalOperator>( tiB.TI_Access::tree_node() ) ) {
      assert( tiB.is_short_circuiting() );
      assert( skip_Arg == 1 || skip_Arg == 2 );
      if( skip_Arg == 1 )
        code << "  TArg1 " << ( tiB.arg_is_ref( 1 ) ? "" : "&" ) << "arg1 = arg2;";
      else
        code << "  TArg2 " << ( tiB.arg_is_ref( 2 ) ? "" : "&" ) << "arg2 = arg1;";
      code << "\n";
    }
  }
#endif

  // generate __TJP type (for the next level)
  if (tjp.type_needed ()) {
    code << "  typedef ";
    const char *wormhole_type = 0;
#ifdef FRONTEND_CLANG
    // feature is supported in Clang only
    if( wormhole ) {
      if( pack_Arg0 )
        wormhole_type = "TPacked";
    }
#endif
    make_tjp_typename( code, loc, depth( plan ), result_buffer, wormhole_type );
    code << " __TJP;" << endl;
  }

  // declare the result store
  code << "  " << result_buffer.result_declaration();

  // generate common JoinPoint initialization
  tjp.gen_tjp_init(code, loc, _problems, depth (plan),
      ti.has_target_expr() || (arg_count > 0) || deduce_result_type || override_isdep, 0, wrapper_number );
#ifdef FRONTEND_CLANG
  // feature is supported in Clang only
  if( wormhole ) {
    if( pack_Arg0 )
      code << "  tjp.__wormhole = &__packed;" << endl;
  }
#endif

  // add calls to advice code
  make_advice_calls(code, &plan, loc);

  // return the results
#ifdef FRONTEND_CLANG
  // this feature is only available in the Clang variant
  if( pack_Result )
    gen_packed_result_return( code, loc );
  else
#endif
    code << "  " << result_buffer.result_return();

  // --- wrapper-body-end ---

  code << "}" << endl;

  // now we perform the code manipulation
  // insert the call wrapper body
  insert (pos, code.str ());

  // TODO: make Puma check for macro problems and warn if requested

  // if the result type needs to be deduced, add expression copy here
  if( deduce_result_type ) {
    if (argnum > 1)
      trail_call << ", ";
    trail_call << "__AC_TYPEOF(("; // the two brackets are needed, because the call might contain a ','
                                    // e.g. func<a,b>() => ACTYPEOF(func<a,b>()) would be regarded a
                                    // macro call with *two* arguments => use ACTYPEOF((...))!
#ifdef FRONTEND_CLANG
    trail_call << ti.code (); // TODO: what about implicit calls here; print_tree had special handling
#else
    print_tree (trail_call, ti.CallNode(), true);
#endif // FRONTEND_PUMA
    trail_call << "))";
  }

  // paste a closing bracket behind the call
  trail_call << " )";

  return generated_args;
}


// expand implicit calls to operator->() until we are done or hit an AST node with advice
#ifdef FRONTEND_CLANG
void CodeWeaver::expand_arrow_operator_chain(TI_CommonCall &ti, const WeavePos &before_pos, const WeavePos &after_pos) {
  if (!ti.has_target_expr())
    return;
  clang::Expr *expr = ti.target_expr();
  while (true) {
    expr = expr->IgnoreImpCasts();

    // check if this an unseen operator->() call
    clang::CXXOperatorCallExpr *call_expr = clang::dyn_cast_or_null<clang::CXXOperatorCallExpr>(expr);
    if (!call_expr || call_expr->getNumArgs() < 1)
      break;
    clang::FunctionDecl *fdecl = call_expr->getDirectCallee ();
    if (!fdecl || fdecl->getNameAsString() != "operator->")
      break;
    if (_handled_access_nodes.find(call_expr) != _handled_access_nodes.end())
      break;

    // expand the call in the source code
    insert( before_pos, "(", true );
    insert( after_pos, ").operator->()" );

    // go to the next nested call
    expr = call_expr->getArg(0);
  }
}
#endif


// transform the existing expression to provide the wrapper call arguments
bool CodeWeaver::splice_access_expr( ACM_Access *loc ) {
  bool provides_args = true; // most cases provide arguments for the wrapper
#ifdef FRONTEND_CLANG
  if( loc->type_val() == JPT_Call || loc->type_val() == JPT_Builtin) {
    TI_CommonCall &ti = static_cast<TI_CommonCall&>(*TI_Access::of( *loc ));

    //      insert (ti.callee_before_pos (*this), "/*CALLEE-BEFORE*/");
    //      insert (ti.callee_after_pos (*this), "/*CALLEE-AFTER*/");
    //      insert (ti.args_open_before_pos (*this), "/*ARGS-BEFORE1*/");
    //      insert (ti.args_open_after_pos (*this), "/*ARGS-BEFORE2*/");
    //      insert (ti.args_close_before_pos (*this), "/*ARGS-AFTER1*/");
    //      insert (ti.args_close_after_pos (*this), "/*ARGS-AFTER2*/");

    if( ti.is_binary_expr () )
      replace( ti.op_before_pos( *this ), ti.op_after_pos( *this ), "," );
    else if( ti.is_unary_expr () ) {
      if (ti.is_arrow_class_member_access_expr()) {
        // in case this is a chain of operator->() calls, expand nested (invisible) calls
        expand_arrow_operator_chain(ti, ti.before_pos( *this ), ti.after_pos( *this ));
      }
      else if ( ! ti.is_implicit_dereference() )
        kill( ti.op_before_pos( *this ), ti.op_after_pos( *this ) );
    }
    else if( ti.is_ternary_expr() ) {
      assert(false && "There are no user-defined ternary expressions and "
          "the built-in ternary operator is handled differently.");
    }
    else if( ti.is_postfix_expr () ) {
      // replace operator with the dummy argument
      replace( ti.op_before_pos( *this ), ti.op_after_pos( *this ), ", 0" );
    }
    else if( ti.is_index_expr () ) {
      // replace the opening '[' with ',' and remove the closing ']'
      replace( ti.index_open_before_pos( *this ), ti.index_open_after_pos( *this ), "," );
      kill( ti.index_close_before_pos( *this ), ti.index_close_after_pos( *this ) );
    }
    // the destination function is a conversion function
    else if( ti.is_implicit_conversion () ) {
    }
    else {
      // here we should have C-style function call, such as 'foo(1, 2)'
      assert(loc->type_val() == JPT_Call);
      TI_MethodCall& tiC = static_cast<TI_MethodCall&>(ti);

      provides_args = ( tiC.call_args() >  0 );
      if( tiC.target_expr() ) {
        // in case is target object expression is a chain of operator->() calls,
        // expand nested (invisible) calls
        expand_arrow_operator_chain(ti, tiC.before_pos( *this ), tiC.args_open_before_pos (*this));
        if( tiC.target_is_ptr() ) { // deref the target ptr as we pass an reference
          insert( tiC.before_pos( *this ), "*(", true );
          insert( tiC.args_open_before_pos (*this), ")" );
        }
        if( provides_args ) // provide argument seperator if necessary
          insert( tiC.args_open_before_pos (*this), ", " );
        provides_args = true;
      }
      if (!tiC.is_call_op())
        kill (tiC.callee_before_pos (*this), tiC.callee_after_pos (*this));
      kill (tiC.args_open_before_pos (*this), tiC.args_open_after_pos (*this));
      kill (tiC.args_close_before_pos (*this), tiC.args_close_after_pos (*this));
    }

    // remember that this AST node has been handle
    _handled_access_nodes.insert(ti.tree_node());
  }
  else if( loc->type_val() == JPT_Get ) {
    TI_Get &ti = *TI_Get::of( *( static_cast<ACM_Get *>( loc ) ) );

    // splice or pack the entity expr as it might contain indices or target parts
    if( has_complex_entity( loc ) ) { // only pack/splice if necessary
      switch( ti.entity_src_splice_mode() ) {
      case TI_Access::SpliceMode_None:
      case TI_Access::SpliceMode_Packed:
        break; // nothing to do
      case TI_Access::SpliceMode_Pack:
        pack_entity_expr( loc );
        break;
      case TI_Access::SpliceMode_FullSplice:
        splice_entity_expr( loc );
        break;
      default:
        assert( false && "Unknown expr splice mode" );
      }
    }
  }
  else if( loc->type_val() == JPT_Ref ) {
    TI_Ref &ti = *TI_Ref::of( *( static_cast<ACM_Ref *>( loc ) ) );
    // check if we have to remove an explicit operator
    // (this can happen when we weave for an explicit JP that is not child of the operator because the operator JP is suppressed)
    if( ti.is_explicit_operator() )
      kill( ti.op_before_pos( *this ), ti.op_after_pos( *this ) );

    // splice or pack the entity expr as it might contain indices or target parts
    if( has_complex_entity( loc ) ) // only pack/splice if necessary
      switch( ti.entity_src_splice_mode() ) {
      case TI_Access::SpliceMode_None:
      case TI_Access::SpliceMode_Packed:
        break; // nothing to do
      case TI_Access::SpliceMode_Pack:
        pack_entity_expr( loc );
        break;
      case TI_Access::SpliceMode_FullSplice:
        splice_entity_expr( loc );
        break;
      default:
        assert( false && "Unknown expr splice mode" );
      }

    // paste the implicit argument for ArrayToPointerDecay
    if( ti.is_implicit_arraydecay() )
      insert( ti.after_pos( *this ), ", 0 " );
  }
  else if( loc->type_val() == JPT_GetRef ) {
    // nothing to do, as the expr is just the passed entity
  }
  else {
    assert( false && "Unsupported joinpoint type while weaving an access joinpoint !" );
  }

#else
  assert( loc->type_val() == JPT_Call );
  ACM_Function *dst_func = static_cast<ACM_Call *>( loc )->get_target ();
  TI_MethodCall &ti = *( TI_MethodCall::of( *( static_cast<ACM_Call *>( loc ) ) ) );
  const char *calltype = ti.CallNode ()->NodeName ();
  CTree *node;
  // call was a binary operator call
  if (calltype == CT_BinaryExpr::NodeId ()) {
    node = ((CT_BinaryExpr*)ti.CallNode ())->Son (1);
    insert (weave_pos (node->end_token (), WeavePos::WP_AFTER), ",");
    kill (node);
  }
  // call was a unary operator call
  else if (calltype == CT_UnaryExpr::NodeId () ||
      calltype == CT_DerefExpr::NodeId ()) {
    node = ((CT_UnaryExpr*)ti.CallNode ())->Son (0);
    kill (node);
  }
  // call was a postfix operator call
  else if (calltype == CT_PostfixExpr::NodeId ()) {
    if (dst_func->get_name() == "operator ++" ||
        dst_func->get_name() == "operator --")
      insert( ti.after_pos(*this), ", 0");
    node = ((CT_BinaryExpr*)ti.CallNode ())->Son (1);
    kill (node);
  }
  // call was an index operator call
  else if (calltype == CT_IndexExpr::NodeId ()) {
    // replace the opening '[' with ','
    node = ((CT_BinaryExpr*)ti.CallNode ())->Son (1);
    insert (weave_pos (node->end_token (), WeavePos::WP_AFTER), ",");
    kill (node);
    // delete the closing ']'
    node = ((CT_BinaryExpr*)ti.CallNode ())->Son (3);
    kill (node);
  }
  // the destination function is a conversion function
  else if (calltype == CT_ImplicitCall::NodeId ()) {
  }
  // call was an ordinary function call
  else {
    bool target_is_ptr = ti.target_is_ptr();
    bool target_expr   = ti.has_target_expr();
    bool havedstthis = needs_this( *dst_func ) || target_expr;
    CT_ExprList *args = ((CT_CallExpr*)ti.CallNode ())->Arguments ();
    provides_args = ( args->Entries() >  0 );
    CTree *fctnode = ti.CallNode()->Son (0);
    // if the function is given in parenthesis, remove it
    while (fctnode->NodeName () == CT_BracedExpr::NodeId ()) {
      kill (fctnode->Son (0));
      kill (fctnode->Son (2));
      fctnode = fctnode->Son (1);
    }
    const char *fctnodetype = fctnode->NodeName ();
    // now perform the transformation
    if( target_expr ) {
      if( target_is_ptr &&
         (fctnodetype == CT_MembRefExpr::NodeId () ||
          fctnodetype == CT_MembPtrExpr::NodeId ())) {
        insert( weave_pos( fctnode->token(), WeavePos::WP_BEFORE), "*(", true);
        insert( weave_pos( args->token(), WeavePos::WP_BEFORE), ")" );
      }
      if( provides_args )
        insert( weave_pos( args->token(), WeavePos::WP_BEFORE), ", " );
      provides_args = true;
    }
    if (fctnodetype == CT_MembRefExpr::NodeId () && havedstthis) {
      kill (((CT_MembRefExpr*)fctnode)->Son (1));
      kill (((CT_MembRefExpr*)fctnode)->Son (2));
    }
    else if (fctnodetype == CT_MembPtrExpr::NodeId () && havedstthis) {
      kill (((CT_MembPtrExpr*)fctnode)->Son (1));
      kill (((CT_MembPtrExpr*)fctnode)->Son (2));
    }
    else {
      if (!ti.is_call_op())
        kill (fctnode);
    }
    kill (args->Son (0));
    kill (args->Son (args->Sons () - 1));
  }
#endif // FRONTEND_PUMA

  return provides_args;
}

#ifdef FRONTEND_CLANG
void CodeWeaver::gen_packed_resulttype( ostream &code, ACM_Access *loc ) {
  assert( loc->type_val() == JPT_Builtin );
  TI_Builtin &ti = *TI_Builtin::of( *static_cast<ACM_Builtin *>( loc ) );

  if( ti.is_index_expr() )
    code << "typename TPacked::template Extend<TArg1>::NewPacked ";
  else if( ( ti.is_binary_expr() && ( ti.operator_kind_string() == "=" ) ) || ti.is_compound_assignment() )
    code << "TPacked ";
  else
    assert( false && "unhandled nodetype in packed result type generation" );
}

void CodeWeaver::gen_packed_result_return( ostream &code, ACM_Access *loc ) {
  assert( loc->type_val() == JPT_Builtin );
  TI_Builtin &ti = *TI_Builtin::of( *static_cast<ACM_Builtin *>( loc ) );

  if( ti.is_index_expr() )
    code << "  return __packed[ arg1 ];" << endl;
  else if( ( ti.is_binary_expr() && ( ti.operator_kind_string() == "=" ) ) || ti.is_compound_assignment() )
    code << "  return __packed;" << endl;
  else
    assert( false && "unhandled nodetype in packed result return generation" );
}

void CodeWeaver::splice_entity_expr( ACM_Access *loc ) {
  TI_Access &ti = *( TI_Access::of( *loc ) );

  TI_Builtin *src = ti.entity_src();
  string sep = ", ";
  while( src ) {
    const clang::Expr *check_expr = 0;
    if( src->is_index_expr() ) {
      // replace the opening '[' with ',' and wrap the index in brackets as it might contain a , operator
      if( sep.size() == 2 ) {
        replace( src->index_open_before_pos( *this ), src->index_open_after_pos( *this ), ", (" );
        replace( src->index_close_before_pos( *this ), src->index_close_after_pos( *this ), ")" );
      }
      else {
        replace( src->index_open_before_pos( *this ), src->index_open_after_pos( *this ), sep.c_str() );
        kill( src->index_close_before_pos( *this ), src->index_close_after_pos( *this ) ); // already wrapped
      }

      check_expr = src->arg( 0 )->IgnoreImplicit();
    }
    else
      assert( false && "unexpected expr while splicing joinpoint entity" );

    src = src->forwarded_src();

    const clang::Expr *end_search = ( src ? src->TI_Access::tree_node() : ti.ref_node() );
    while( check_expr && check_expr != end_search ) {
      if( const clang::ParenExpr *p = llvm::dyn_cast<clang::ParenExpr>( check_expr ) ) {
        sep = ")" + sep + "(";
        check_expr = p->getSubExpr()->IgnoreImplicit();
      }
      else if( const clang::BinaryOperator *bo = llvm::dyn_cast<clang::BinaryOperator>( check_expr ) ) {
        if( bo->getOpcode() == clang::BO_Comma )
          check_expr = bo->getRHS()->IgnoreImplicit();
        else {
          assert( false && "unexpected expr while splicing joinpoint entity" );
          break;
        }
      }
      else {
        assert( false && "unexpected expr while splicing joinpoint entity" );
        break;
      }
    }
  }

  splice_member_expr( loc, sep.c_str() );
}

void CodeWeaver::pack_entity_expr( ACM_Access *loc ) {
  TI_Access &ti = *( TI_Access::of( *loc ) );

  splice_member_expr( loc );

  insert( ti.entity_before_pos( *this ), "AC::PEInit<>( ", true );
  insert( ti.entity_after_pos( *this ), " )" );
}

// an Entity is complex if it needs more information than the primary entity symbol/source
// to be meaningful. This is either a class obj or indices to access an array
// access to static member are special for historical reasons:
//  if an target_expr is used, it is used in proceed too,
//  so they are handled like others that need a class obj
bool CodeWeaver::has_complex_entity( ACM_Access *loc ) {
  TI_Access &ti = *( TI_Access::of( *loc ) );
  ACM_Name *entity = has_entity( *loc ) ? get_entity( *loc ) : 0;

  bool have_Target = ti.has_target_expr() || ( entity && needs_this( *entity ) );
  bool have_Idx = ti.entity_index_count() > 0;
  return have_Target || have_Idx;
}

void CodeWeaver::splice_member_expr( ACM_Access *loc, const char *separator ) {
  TI_Access &ti = *( TI_Access::of( *loc ) );
  ACM_Name *entity = get_entity( *loc );
  if( ! needs_this( *entity ) && ! ti.has_target_expr() )
    return; // nothing to do, as we have no targetclass

  string before, replacement, after;

  bool has_dst = false;
  if( ti.has_target_expr() ) {
    if( ti.target_is_ptr() ) {
      before += "*(";
      replacement = ")" + replacement;
    }
    has_dst = true;
  }
  else if( needs_this( *entity ) ) { // only option is implicit this
    replacement = "*this" + replacement;
    has_dst = true;
  }

  if( has_dst ) {
    if( ti.target_class() != ti.targetexpr_class() ) {
      before = "static_cast< " + ti.target_type_string() + " & >( " + before;
      replacement += " )";
    }
    replacement += separator;
  }

  if( needs_this( *entity ) ) {
    if( ti.entity_from_baseclass() ) {
      replacement += "AC::mptr_castT< " + ti.target_type_string() +" >( ";
      after = " )" + after;
    }
    if( is_wrapped_array( entity ) ) {
      replacement += "reinterpret_cast< " + name_qualifier( entity ) + "__ac_wrapper_" + entity->get_name() + "::_A";
      replacement += " (" + name_qualifier( entity ) + "*) >( ";
      after = " )" + after;
    }
    replacement += "&";
  }
  if( ! ti.is_qualified() ) {
    if( needs_this( *entity ) )
      replacement += ti.targetexpr_type_string() + "::";
    else
      replacement += name_qualifier( entity );
  }

  if( ! before.empty() )
    insert( ti.entity_before_pos( *this ), before.c_str(), true );
  replace( ( ti.has_target_expr() ? ti.entity_op_before_pos( *this ) : ti.entity_before_pos( *this ) ), ti.entity_op_after_pos( *this ), replacement.c_str() );
  if( ! after.empty() )
    insert( ti.entity_after_pos( *this ), after.c_str() );
}

bool CodeWeaver::is_wrapped_array( ACM_Name *ent ) {
  if( ent->type_val() != JPT_Variable )
    return false;

  assert( static_cast<ACM_Any *>( ent->get_parent() )->type_val() & ( JPT_Class | JPT_Aspect ) );
  ACM_Class *cls = static_cast<ACM_Class *>( ent->get_parent() );
  // check if the class has a woven builtin copy constructor
  // only in gen_special_member_function called for these the wrapping is done
  // so the class has replaced array types
  if( ! TI_Class::of( *cls )->has_replaced_arrays() )
    return false;

  SyntacticContext var( TI_Variable::of( static_cast<ACM_Variable &>( *ent ) )->decl() );
  // check parallel to CodeWeaver::gen_special_member_function
  if( var.is_array() && ! var.is_anon_union_member() )
    return true;
  else
    return false;
}

void CodeWeaver::preplanTransform( ACM_Code &code_jpl ) {
  if( ! ( code_jpl.type_val() & ( JPT_Get | JPT_Set | JPT_Ref ) ) )
    return; // nothing to do

  ACM_Access &jpl = static_cast<ACM_Access &>( code_jpl );
  if( has_complex_entity( &jpl ) ) {
    TI_Builtin *entity_src = TI_Access::of( jpl )->entity_src();

    if( is_implicit( jpl ) ) // for implicit joinpoint, skip the parent as it is handled differently
      entity_src = entity_src->forwarded_src();

    if( entity_src )
      entity_src->request_packed_forward( &jpl ); // the method itself is recursive, so no need to do it here
  }
}
#endif

void CodeWeaver::insert_id_class (ACM_Call *loc, ACM_Name *src_obj,
    const WeavePos &pos, ACM_Function &dst_func) {

  TI_MethodCall &ti = *(TI_MethodCall*)TransformInfo::of (*loc);
#ifdef FRONTEND_CLANG
  const SyntacticContext &sctxt = ti.access_context();
  const SyntacticContext &dfctxt = ti.entity_context();
#else
  // fall back to old functions in TI_MethodCall if using Puma and thus the new ones on TI_Access do not exist
  const SyntacticContext &sctxt = ti.call_context();
  const SyntacticContext &dfctxt = ti.dest_func_context();
#endif
  ostringstream bypass;
  const WeavePos *bypass_pos = &pos;
  ACM_Class *src_class = (sctxt.is_in_class_scope () ? cscope (src_obj) : 0);
  ACM_Class *dst_class = cscope (&dst_func);

  ACM_Class *target_class = dst_class;
  if (!in_project (dst_func)) {
    // calls to private externals are not supported
    if (dfctxt.protection() == "private") {
#ifdef FRONTEND_PUMA
      _err << sev_error << ti.tree()->token ()->location ()
           << "can't weave call advice for private external function '"
           << signature(dst_func).c_str () << "'" << endMessage;
#endif
      return;
    }
    // target function is protected
    if (ti.has_target_expr ())
      target_class = loc->get_target_class ();
    else
      target_class = cscope (src_obj);
    dst_class = target_class;
  }

//  cout << "---" << endl;
//  cout << (src_class ? signature(*src_class) : string("no source")) << endl;
//  cout << (dst_class ? signature(*dst_class) : string("no destination")) << endl;
//  cout << (target_class ? signature(*target_class) : string("no target")) << endl;

  bool before_class = (src_class && src_class != dst_class);
  if (before_class) {
    bypass_pos = &TI_Class::of (*src_class)->objdecl_start_pos (*this);
    close_namespace (bypass, TI_Class::of (*src_class)->get_def_context ());
  }
  else if (sctxt.is_in_namespace ())
    close_namespace (bypass, sctxt);

  string ns_open, ns_close, classname = Naming::bypass_caller_class(target_class);

  if ((src_class && src_class != dst_class) ||
      (!src_class && sctxt.syntactical_scope () != signature(*dst_class))) {
    ACM_Name *curr = target_class;
    while (curr->type_val () & (JPT_Class|JPT_Aspect)) {
      classname = curr->get_name () + "::" + classname;
      curr = (ACM_Name *)curr->get_parent ();
    }
    while (curr->get_name () != "::") {
      ns_open = string ("namespace ") + curr->get_name () + " { " + ns_open;
      ns_close += "} ";
      curr = (ACM_Name *)curr->get_parent ();
    }
  }
  if (ns_open != "")
    bypass << ns_open << endl;
  if (src_class && !before_class)
    bypass << "public:" << endl;
  bypass << "// unique ID-class, just a type" << endl;
  bypass << "struct ";
  Naming::bypass_id_class (bypass, loc);
  bypass << ";" << endl;
  bypass << "template<int X> struct " << classname << "<";
  Naming::bypass_id_class (bypass, loc);
  bypass << ", X> {" << endl;
  bypass << "  static ";
  if (_problems._use_always_inline) // GNU extension
    bypass << "__attribute__((always_inline)) ";
  bypass << "inline ";
  stringstream sig, impl;
  sig << "call (";
  int argnum = 0;
  bool dstthis_needed = needs_this (dst_func);
  // pass object pointer if needed
  if (dstthis_needed) {
    if (TI_Function::of (dst_func)->is_const ())
      sig << "const ";
    sig << cleanup_name (signature (*target_class)) << " *dstthis";
    argnum++;
  }
  // argument list
  unsigned call_args = get_arg_count(*loc); // IMPORTANT: get_arg_count does neither count default args nor "..."!
  for (unsigned a = 0; a < call_args; a++) {
    if (argnum > 0)
      sig << ", ";
    stringstream arg_name;
    arg_name << "arg" << a;
    sig << dfctxt.arg_type (a, arg_name.str ());
    argnum++;
  }
  sig << ")";
  // add result type
  bypass << dfctxt.result_type (sig.str ());
  bypass << "{" << endl;
  bypass << "    ";
  if (dfctxt.result_type () != "void")
    bypass << "return ";
  if (dstthis_needed)
    bypass << "dstthis->";
  if (ti.is_qualified())
    bypass << signature (*(ACM_Name*)dst_func.get_parent ()) << "::" << dst_func.get_name () << " (";
  else
    bypass << dst_func.get_name () << " (";
  for (unsigned a = 0; a < call_args; a++) {
    if (a > 0)
      bypass << ", ";
    bypass << "arg" << a;
  }
  bypass << ");" << endl;
  bypass << "  }" << endl;
  bypass << "};";
  if (ns_close != "")
    bypass << ns_close << endl;
  if (before_class) {
    open_namespace (bypass, TI_Class::of (*src_class)->get_def_context ());
  } else if (sctxt.is_in_namespace())
    open_namespace (bypass, sctxt);
  // switch to the right protection if needed
  if (src_class && !before_class && !TI_Class::of (*src_class)->is_struct() &&
      sctxt.protection() != "" && sctxt.protection() != "public")
    bypass << "\n" << sctxt.protection() << ":\n";

  // finally insert the generated code at the appropriate position
  insert (*bypass_pos, bypass.str ());
}

void CodeWeaver::bypass_info_clear () {
  _bypass_blacklist.clear ();
}

bool CodeWeaver::bypass_in_blacklist (ACM_Class *cls) {
  const TI_Class *ti = TI_Class::of (*cls);
  return !ti->valid() || !ti->is_defined() || ti->is_template_instance() || ti->is_extern_c() ||
      _bypass_blacklist.find (&ti->objdecl_start_pos(*this)) != _bypass_blacklist.end ();
}

void CodeWeaver::bypass_insert (ACM_Class *acm_cls) {
  const TI_Class *ti = TI_Class::of (*acm_cls);

  if (!ti->is_defined() || ti->is_template_instance() || ti->is_extern_c())
    return;

  string bypass =
      string("\n  template <typename, int = 0> struct ") +
      Naming::bypass_caller_class (acm_cls) +
      " {};\n  template <typename, int> friend struct " +
      Naming::bypass_caller_class (acm_cls) + ";";
  if (ti->is_class() && !ti->is_struct()) // don't weave this into structs
    bypass = string ("public:") + bypass + "\nprivate:";

  insert (ti->body_start_pos(*this), bypass, true);

  // put a class in the backlist if is defined inside a template instance
  // -> it can appear more than once in the model!
  if (ti->is_in_template_instance())
    _bypass_blacklist.insert (&ti->objdecl_start_pos(*this));
}

// Transform aspect declarations: Simply replace the keyword 'aspect' by 'class'
void CodeWeaver::transform_aspect(ACToken start) {
  const WeavePos &from = weave_pos (start, WeavePos::WP_BEFORE);
  const WeavePos &to   = weave_pos (start, WeavePos::WP_AFTER);
  replace(from, to, "class");
}

// Transform the definition of a named pointcut:
//
// pointcut [virtual] foo(int i) = call("C") && args(i);
// ==>
// void foo(int i);
void CodeWeaver::transform_pointcut_def(ACToken start, ACToken virtual_token,
    ACToken assignment_token, ACToken pct_end_token) {
  // TODO: at the moment the Puma parser checks whether base classes of aspects
  // are abstract. Therefore, a pure virtual pointcut has to become a pure virtual
  // function. Later this will not be necessary.
  insert (weave_pos(start, WeavePos::WP_BEFORE), "void");
//  transform_delete(start, virtual_token ? virtual_token : start);
  transform_delete(start, start);
  // FIXME: Don't use frontend internals.
#ifdef FRONTEND_PUMA
  if (string(pct_end_token.text()) != "0")
#else
  if (!pct_end_token.get().isLiteral() || string(pct_end_token.text()) != "0")
#endif
    transform_delete(assignment_token, pct_end_token);
}

// Transform the advice code into an ordinary C++ template function:
//
// advice call("C") && args(i) : before(int i) { ... }
// ==>
// template <typename __Dummy> void __a42_before (int i) { ... }
// template <typename JoinPoint> void __a42_before (JoinPoint *tjp, int i) { ... }
void CodeWeaver::transform_advice_code( int no, string kind, AdviceCodeContext context, bool has_args, TypeUse &uses_type,
    ACToken arg_begin, ACToken body_begin, ACToken body_end, const string &prot) {

  // create and insert the function name
  ostringstream func_decl;
  func_decl << endl << "public: ";
  if (context & ACC_TYPE)
    func_decl << "template <typename JoinPoint> ";
  func_decl << "void __a" << no << "_" << kind << " ";
  insert (weave_pos(arg_begin, WeavePos::WP_BEFORE), func_decl.str ());
  if (context & ACC_OBJ) {
    string tjp_param = "JoinPoint *tjp";
    if (has_args) tjp_param += ", ";
    insert (weave_pos(arg_begin, WeavePos::WP_AFTER), tjp_param);
  }
  if( uses_type.any() ) {
    // also generate some typedefs for That, Target, and Result
    // the corresponding typedef must replace each JoinPoint::(That|Target|Result)
    ostringstream typedefs;
    typedefs << endl;
    if( uses_type.that ) {
      typedefs << "  typedef typename JoinPoint::That ";
      Naming::tjp_typedef (typedefs, "That");
      typedefs << ";" << endl;
    }
    if( uses_type.target ) {
      typedefs << "  typedef typename JoinPoint::Target ";
      Naming::tjp_typedef (typedefs, "Target");
      typedefs << ";" << endl;
    }
    if( uses_type.result ) {
      typedefs << "  typedef typename JoinPoint::Result ";
      Naming::tjp_typedef (typedefs, "Result");
      typedefs << ";" << endl;
    }
    if( uses_type.entity ) {
      typedefs << "  typedef typename JoinPoint::Entity ";
      Naming::tjp_typedef (typedefs, "Entity");
      typedefs << ";" << endl;
    }
    if( uses_type.memberptr ) {
      typedefs << "  typedef typename JoinPoint::MemberPtr ";
      Naming::tjp_typedef (typedefs, "MemberPtr");
      typedefs << ";" << endl;
    }
    if( uses_type.array ) {
      typedefs << "  typedef typename JoinPoint::Array ";
      Naming::tjp_typedef (typedefs, "Array");
      typedefs << ";" << endl;
    }
    insert (weave_pos (body_begin, WeavePos::WP_AFTER), typedefs.str ());
  }

  if (prot != "public" && prot != "")
    insert (weave_pos (body_end, WeavePos::WP_AFTER), string ("\n") + prot + ":\n");
}

// Transform the aspect function into an ordinary C++ template function:
//
// static Foo *aspectof () { ... }
// ==>
// [template <typename JoinPoint>] static Foo *aspectof ([JoinPoint *tjp]) { ... }
void CodeWeaver::transform_aspectof( AdviceCodeContext context, TypeUse &uses_type,
    ACToken start, ACToken name, ACToken arg_begin, ACToken body_begin, ACToken body_end,
    const string &prot) {

  // create and insert the function name
  ostringstream func_decl;
  func_decl << endl << "public: ";
  if (context & ACC_TYPE)
    func_decl << "template <typename JoinPoint> ";
  insert (weave_pos(start, WeavePos::WP_BEFORE), func_decl.str ());
  // TODO: issue warning saying that deprecated name "aspectOf" is used
  if (string(name.text ()) == "aspectOf")
    replace (weave_pos(name, WeavePos::WP_BEFORE),
        weave_pos (name, WeavePos::WP_AFTER), "aspectof");
  if (context & ACC_OBJ) {
    string tjp_param = "JoinPoint *tjp";
    insert (weave_pos(arg_begin, WeavePos::WP_AFTER), tjp_param);
  }
  if( uses_type.any() ) {
    // also generate some typedefs for That, Target, and Result
    // the corresponding typedef must replace each JoinPoint::(That|Target|Result)
    ostringstream typedefs;
    typedefs << endl;
    if( uses_type.that ) {
      typedefs << "  typedef typename JoinPoint::That ";
      Naming::tjp_typedef (typedefs, "That");
      typedefs << ";" << endl;
    }
    if( uses_type.target ) {
      typedefs << "  typedef typename JoinPoint::Target ";
      Naming::tjp_typedef (typedefs, "Target");
      typedefs << ";" << endl;
    }
    if( uses_type.result ) {
      typedefs << "  typedef typename JoinPoint::Result ";
      Naming::tjp_typedef (typedefs, "Result");
      typedefs << ";" << endl;
    }
    if( uses_type.entity ) {
      typedefs << "  typedef typename JoinPoint::Entity ";
      Naming::tjp_typedef (typedefs, "Entity");
      typedefs << ";" << endl;
    }
    if( uses_type.memberptr ) {
      typedefs << "  typedef typename JoinPoint::MemberPtr ";
      Naming::tjp_typedef (typedefs, "MemberPtr");
      typedefs << ";" << endl;
    }
    if( uses_type.array ) {
      typedefs << "  typedef typename JoinPoint::Array ";
      Naming::tjp_typedef (typedefs, "Array");
      typedefs << ";" << endl;
    }
    insert (weave_pos (body_begin, WeavePos::WP_AFTER), typedefs.str ());
  }

  if (prot != "public" && prot != "")
    insert (weave_pos (body_end, WeavePos::WP_AFTER), string ("\n") + prot + ":\n");
}

void CodeWeaver::transform_delete (ACToken from, ACToken to) {
  const WeavePos &from_pos = weave_pos (from, WeavePos::WP_BEFORE);
  const WeavePos &to_pos   = weave_pos (to, WeavePos::WP_AFTER);
  kill(from_pos, to_pos);
}

void CodeWeaver::transform_anon_class (ACToken name_pos, const string &name) {
  // 'name_pos' is either ':' or '{'
  insert (weave_pos (name_pos, WeavePos::WP_BEFORE), string (" ") + name + " ");
}

void CodeWeaver::transform_anon_member_class (ACToken end_token,
    const string &anon_members) {
  string code = "";
  code += anon_members;
  code += "\n";
  insert (weave_pos (end_token, WeavePos::WP_AFTER), code);
}

ACM_Class *CodeWeaver::cscope (ACM_Name *obj) {
  ACM_Name *parent = (ACM_Name*)obj->get_parent();
  return (parent->type_val() == JPT_Class ? (ACM_Class*)parent : (ACM_Class*)0);
}

#ifdef FRONTEND_PUMA
CClassInfo *CodeWeaver::cscope(CObjectInfo *obj) {
  CRecord *crec = obj->ClassScope ();
  return crec ? crec->ClassInfo () : (CClassInfo*)0;
}
#else
clang::CXXRecordDecl *CodeWeaver::cscope(clang::DeclContext *obj) {
  return llvm::dyn_cast<clang::CXXRecordDecl>(obj->getParent());
}
#endif

ACM_Name *CodeWeaver::nscope (ACM_Function *func) {
  ACM_Name *result = (ACM_Name*)func->get_parent ();
  if (result->get_name() == "::" || result->get_name () == "<unnamed>")
    return 0;
  return result;
}

string CodeWeaver::name_qualifier( ACM_Name *entity ) {
  assert( entity->get_parent() && ( static_cast<ACM_Node *>( entity->get_parent() )->type_val() & JPT_Name ) );
  string scope_name = cleanup_name( signature( *static_cast<ACM_Name *>( entity->get_parent() ) ) );
  if( scope_name.size() == 0 )
    scope_name = "::";
  else if( scope_name.substr( 0, 2 ) != "::" )
    scope_name = "::" + scope_name + "::";
  else if( scope_name.size() > 2 )
    scope_name += "::";

  return scope_name;
}

string CodeWeaver::cleanup_name(const string& in) {
  string result = in;
  string::size_type pos = result.length ();
  while ((pos = result.rfind ("::<unnamed>", pos)) != string::npos)
    result = result.erase (pos, 11);
  if (result.find ("<unnamed>") == 0)
    result = result.erase (0, 9);
  return result;
}

string CodeWeaver::strip_template_parameters (const string &in) {
  int pos = in.size () - 1;
  if (in[pos] == '>') {
    int level = 0;
    do {
      if (in[pos] == '>')
        level++;
      else if (in[pos] == '<')
        level--;
      pos--;
    }
    while (pos >= 0 && level > 0);
  }
  return (pos >= 0 ? in.substr (0, pos + 1) : in);
}

#ifdef FRONTEND_PUMA
void CodeWeaver::print_tree (ostream &out, CTree *node, bool expand_implicit_calls) {

  if (expand_implicit_calls && node->NodeName () == CT_ImplicitCall::NodeId() &&
      !((CT_ImplicitCall*)node)->Object()->FunctionInfo()->isConstructor()) {
    out << "(";
    print_tree (out, node->Son (0), expand_implicit_calls);
    out << ")." << ((CT_ImplicitCall*)node)->Object()->Name().c_str () << "()";
  }
  else if (node->NodeName () == CT_Token::NodeId ())
    out << node->token ()->text () << " ";
  else
    for (int s = 0; s < node->Sons (); s++)
      print_tree (out, node->Son (s), expand_implicit_calls);
}
#endif // FRONTEND_PUMA

void CodeWeaver::rename_args (const SyntacticContext &sctxt, const char * new_name,
    vector<string> &arg_names) {
#ifdef FRONTEND_PUMA
  CFunctionInfo *func = (CFunctionInfo*)sctxt.object();

  // first rename the arguments in the argument list
  for (unsigned arg = 0; arg < func->Arguments (); arg++) {
    if (func->Argument (arg)->isAnonymous()) {
      // create a new name
      ostringstream name;
      name << new_name << arg;
      // insert the generated name
      rename (func->Argument (arg), name.str ());
      arg_names.push_back (name.str ());
    }
    else {
      arg_names.push_back (func->Argument (arg)->Name ().c_str());
    }
  }
#else
  const clang::FunctionDecl *decl =
      llvm::cast<clang::FunctionDecl>(sctxt.decl());

  for (unsigned i = 0, e = sctxt.args(); i != e; ++i) {
    std::string arg_name = sctxt.arg_name(i);
    if (arg_name.empty()) {
      // create a new name
      ostringstream name;
      name << new_name << i;
      // insert the generated name
      rename(decl->getParamDecl(i), name.str());
      arg_names.push_back(name.str());
    } else {
      arg_names.push_back(sctxt.arg_name(i));
    }
  }
#endif
}

#ifdef FRONTEND_PUMA
void CodeWeaver::rename (CArgumentInfo *arg, const string &new_name) {
  // don't change anything with void arguments
  if (arg->TypeInfo ()->is_void ())
    return;

  // find the syntax tree node of the argument name
  CT_ArgDecl *arg_decl = (CT_ArgDecl*)arg->Tree ();
  CT_Declarator *declarator = (CT_Declarator*)arg_decl->Declarator ();
  CT_SimpleName *arg_name = name (declarator);

  if (arg_name->NodeName () != CT_PrivateName::NodeId ()) {
    const WeavePos &lpos = weave_pos (arg_name->token (), WeavePos::WP_BEFORE);
    const WeavePos &rpos = weave_pos (arg_name->end_token (), WeavePos::WP_AFTER);
    replace (lpos, rpos, new_name);
  }
  else {
    // in the case of an anonymous argument I have to find an insert pos
    CTree *left = 0, *right = 0;
    if (declarator) {
      if (declarator->NodeName () == CT_InitDeclarator::NodeId ())
        right = ((CT_InitDeclarator*)declarator)->Initializer ();
      else if (declarator->NodeName () == CT_BracedDeclarator::NodeId ())
        right = declarator->Son (declarator->Sons () - 1);
      else if (declarator->NodeName () == CT_ArrayDeclarator::NodeId ())
        right = declarator->Son (1);
      else if (declarator->NodeName () == CT_FctDeclarator::NodeId ())
        right = declarator->Son (1);
      else if (declarator->NodeName () == CT_RefDeclarator::NodeId ())
        left = declarator->Son (0);
      else if (declarator->NodeName () == CT_PtrDeclarator::NodeId ())
        left = declarator->Son (declarator->Sons () - 2);
      else if (declarator->NodeName () == CT_MembPtrDeclarator::NodeId ())
        left = declarator->Son (declarator->Sons () - 2);
      else if (declarator->NodeName () == CT_BitFieldDeclarator::NodeId ())
        right = declarator->Son (1);
      else {
        _err << sev_fatal << declarator->token ()->location ()
             << "unknown declarator type" << endMessage;
        return;
      }
    }
    if (!left && !right) {
      // no declarator + anonymous => there must be at least a decl spec seq!
      left = arg_decl->DeclSpecs ();
    }
    if (left) {
      const WeavePos &lp = weave_pos (left->end_token (), WeavePos::WP_AFTER);
      insert (lp, " ");
      insert (lp, new_name);
    }
    else if (right) {
      const WeavePos &rp = weave_pos (right->token (), WeavePos::WP_BEFORE);
      insert (rp, " ");
      insert (rp, new_name);
    }
    else
      _err << sev_fatal << declarator->token ()->location ()
           << "found no hook to replace ananymous arg name" << endMessage;
  }
}
#else
void CodeWeaver::rename(const clang::ParmVarDecl *arg, const string &new_name) {
  // Get a string with the new name in the right place. It's important that this
  // is the same typedef'd type as the original.
  std::string s;
  llvm::raw_string_ostream os(s);
  clang::QualType ty = arg->getTypeSourceInfo()->getType();
  os << TI_Type::get_type_code_text(ty, &arg->getASTContext(), new_name.c_str());

  if (!arg->hasInit() || arg->hasInheritedDefaultArg()) {
    // No default argument. Replace the entire argument with the new type and
    // name.

    // ... we would like to use this ...
    //    replace(weave_pos(arg->getLocStart(), WeavePos::WP_BEFORE),
    //            get_pos_after_loc(arg->getLocEnd()), os.str());
    // ... but the result of getLocEnd with the current Clang version is nonsense
    // workaround:
    replace(weave_pos(arg->getLocStart(), WeavePos::WP_BEFORE),
        get_pos_after_loc(arg->getTypeSourceInfo()->getTypeLoc().getSourceRange().getEnd()),
        os.str ());
  } else {
    // Default argument. Kill from the beginning of the argument until the start
    // of the default parameter. Then insert the new type and argument. Since we
    // kill the equals sign, generate a new one.
    os << " = ";
    replace(weave_pos(arg->getLocStart(), WeavePos::WP_BEFORE),
            weave_pos(arg->getInit()->getLocStart(), WeavePos::WP_AFTER),
            os.str());
  }
}
#endif

#ifdef FRONTEND_PUMA
CT_SimpleName *CodeWeaver::is_name (CTree *node) {
  if (node->NodeName () == CT_QualName::NodeId () ||
      node->NodeName () == CT_SimpleName::NodeId () ||
      node->NodeName () == CT_OperatorName::NodeId () ||
      node->NodeName () == CT_ConversionName::NodeId () ||
      node->NodeName () == CT_RootQualName::NodeId () ||
      node->NodeName () == CT_PrivateName::NodeId ())
    return (CT_SimpleName*)node;
  else
    return (CT_SimpleName*)0;
}

CT_FctDeclarator *CodeWeaver::fct_declarator (CT_Declarator *declarator) {
  do {
    CT_Declarator *next = (CT_Declarator*)declarator->Declarator ();
    if (declarator->NodeName () == CT_FctDeclarator::NodeId () &&
  is_name (next))
      return (CT_FctDeclarator*)declarator;
    declarator = next;
  } while (!is_name (declarator));
  return (CT_FctDeclarator*)0;
}

CT_SimpleName *CodeWeaver::name (CT_Declarator *&declarator) {
  CTree *node = declarator;
  declarator  = (CT_Declarator*)0;
  while (!is_name (node)) {
    declarator = (CT_Declarator*)node;
    node = declarator->Declarator ();
  }

  return (CT_SimpleName*)node;
}

// return LinkageSpec node for extern "C" <decl> like declarations
CTree *CodeWeaver::linkage_adjust (CT_Decl *decl) {
  CT_LinkageSpec *linkage = decl->Linkage ();
  return (!linkage || linkage->isList ()) ? decl : linkage;
}
#endif // FRONTEND_PUMA

void CodeWeaver::gen_binding_templates (ostream &out, ACM_CodePlan *plan,
    const char *jpname) {
  // later the individual bindings have to be passed from here, too.
  typedef ACM_Container<ACM_CodeAdvice, true> Container;
  Container &before = plan->get_before();
  for (Container::iterator i = before.begin (); i != before.end (); ++i)
    TI_CodeAdvice::of(*(*i))->get_advice_info()->gen_binding_template (out, jpname, _problems);
  if (plan->has_around())
    TI_CodeAdvice::of(*plan->get_around())->get_advice_info()->gen_binding_template (out, jpname, _problems);
  if (plan->has_next_level())
    gen_binding_templates (out, plan->get_next_level(), jpname);
  Container &after = plan->get_after();
  for (Container::iterator i = after.begin (); i != after.end (); ++i)
    TI_CodeAdvice::of(*(*i))->get_advice_info()->gen_binding_template (out, jpname, _problems);
}
