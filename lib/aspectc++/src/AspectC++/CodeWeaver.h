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

#ifndef __CodeWeaver_h__
#define __CodeWeaver_h__

// The code weaver allows AspectC++ specific code manipulation transactions.
// All access to the syntax tree nodes for the transformation is 
// encapsulated here.
// The class does not know about class JoinPoint. It only knows join point
// locations (JPL_*).

#include <iostream>
#include <set>
#include <map>
using namespace std;

#include "ACToken.h"
#include "ACErrorStream.h"
#include "ACProject.h"
#include "ACConfig.h"
#include "ACResultBuffer.h"
#include "WeaverBase.h"
#include "AspectIncludes.h"
#include "SyntacticContext.h"

namespace Puma {
  class CTree;
  class CClassInfo;
  class CRecord;
  class CProject;
} // namespace Puma

#ifdef FRONTEND_CLANG
namespace clang {
  class DeclContext;
}
#endif

class ACM_Any;
class ACM_Class;
class ACM_Execution;
class ACM_Call;
class ACM_Code;
class ACM_CodePlan;

class AspectInfo;
class AdviceInfo;
class ThisJoinPoint;
class TI_MethodCall;

class CodeWeaver : public WeaverBase {

  AspectIncludes _aspect_includes;
  BackEndProblems _problems;
  ACConfig &_conf;
  std::set<void*> _handled_access_nodes;

  set<const WeavePos *> _bypass_blacklist;

  void make_check_function (bool in_checked,
      Puma::CRecord *in, Puma::CRecord *check_for);
  void make_tjp_typename( ostream &out, ACM_Code *loc, int depth, ACResultBuffer &rb, const char *wormhole_type = 0 );

  void make_proceed_code (ostream &out, ACM_Code *loc, bool action,
                          vector<string> *arg_names = 0);
  void make_proceed_to_implicit( ostream &out, ACM_Code *loc, bool action, vector<string> *arg_names = 0 );
  void make_advice_call(ostream &out, ACM_Code *loc, AdviceInfo *ad,
                        bool inter, int depth);
  void make_advice_calls (ostream &out, ACM_CodePlan *plan,
                          ACM_Code *loc, bool inter = false, vector<string> *arg_names = 0);
  void make_action_wrapper(ostream &impl, ACM_Code *jpl, int depth);
  void make_proceed_func( ostream &impl, ACM_Code *loc, ACM_CodePlan *plan, bool has_wormhole = false );
  string wrapper_function_signature (ACM_Code *loc,
      const SyntacticContext &sctxt, bool def);
  void gen_special_member_function (ACM_Code *loc);
  void wrap_function (ACM_Code *loc);
  void wrap_function_def (ACM_Code *loc, const SyntacticContext &sctxt,
    bool wrapped_decl);
  bool wrap_function_decl (ACM_Code *loc, const SyntacticContext &sctxt);
  void wrap_attribute_array (const SyntacticContext &attr_ctxt, list<SyntacticContext> &attrs);
  string gen_wrapped_array (const SyntacticContext &attr_ctxt);
  void gen_binding_templates (ostream &out, ACM_CodePlan *plan, const char *jpname);

#ifdef FRONTEND_PUMA
  void setup_tjp (ThisJoinPoint &tjp, Puma::CTree *node);
#else
  void setup_tjp (ThisJoinPoint &tjp, clang::Stmt* node);
#endif
  void insert_id_class (ACM_Call *loc, ACM_Name *src_obj, const WeavePos &pos,
      ACM_Function &dst_func);

  // scope functions
#ifdef FRONTEND_PUMA
  Puma::CClassInfo *cscope (Puma::CObjectInfo *obj);
#else
  clang::CXXRecordDecl *cscope(clang::DeclContext *obj);
#endif
  ACM_Class *cscope (ACM_Name *obj);
  ACM_Name *nscope (ACM_Function *func);
  string name_qualifier( ACM_Name *entity );
  string cleanup_name(const string& in);
  string strip_template_parameters (const string &in);


#ifdef FRONTEND_PUMA
  // not a very good function
  void print_tree (ostream &out, Puma::CTree *node, bool expand_implicit_calls = false);

  void rename_args (const SyntacticContext &sctxt, const char * new_name,
      vector<string> &arg_names);
  void rename (Puma::CArgumentInfo *arg, const string &new_name);
  Puma::CT_SimpleName *is_name (Puma::CTree *node);
  Puma::CT_FctDeclarator *fct_declarator (Puma::CT_Declarator *declarator);
  Puma::CT_SimpleName *name (Puma::CT_Declarator *&declarator);

  // return LinkageSpec node for extern "C" <decl> like declarations
  Puma::CTree *linkage_adjust (Puma::CT_Decl *decl);
#else
  void rename_args (const SyntacticContext &sctxt, const char * new_name,
                    vector<string> &arg_names);
  void rename (const clang::ParmVarDecl *arg, const string &new_name);
#endif

public:

  CodeWeaver( ACProject& prj, LineDirectiveMgr &ldm, ACConfig &c ) :
    WeaverBase( prj, ldm ), _aspect_includes( prj ), _conf( c ) {}

  // set weaver parameters
  void problems (const BackEndProblems &problems) {
    _problems = problems;
#ifdef FRONTEND_PUMA
    _warn_macro_expansion = problems._warn_macro;
#endif // FRONTEND_PUMA
  }

  // get weaver parameters
  const BackEndProblems &problems () const { return _problems; }

  void open_namespace (ostream &out, const SyntacticContext &ctxt);
  void close_namespace (ostream &out, const SyntacticContext &ctxt);
#ifdef FRONTEND_PUMA
  void setup_tjp (ThisJoinPoint &tjp, Puma::CFunctionInfo* func);
#else
  void setup_tjp (ThisJoinPoint &tjp, clang::FunctionDecl *func);
#endif
  void insert_invocation_functions (ACM_Aspect *jpl_aspect, const string &defs);

  void make_tjp_struct(ostream &out, ACM_Code *loc,
                       ACM_CodePlan *plan, const ThisJoinPoint &tjp);
  void access_join_point (ACM_Access *loc);
#ifdef FRONTEND_CLANG
  void splice_shortcircuit_expr( ACM_Builtin *loc );
  void expand_arrow_operator_chain(TI_CommonCall &ti, const WeavePos &before_pos, const WeavePos &after_pos);
#endif
  bool generate_access_wrapper( ACM_Access *loc, ostream &new_call, ostream &trail_call, const char *name_that = "this",
      int wrapper_number = -1);
  bool splice_access_expr( ACM_Access *loc );
#ifdef FRONTEND_CLANG
  void gen_packed_resulttype( ostream &code, ACM_Access *loc );
  void gen_packed_result_return( ostream &code, ACM_Access *loc );

  void splice_entity_expr( ACM_Access *loc );
  void pack_entity_expr( ACM_Access *loc );
  static bool has_complex_entity( ACM_Access *loc );
  void splice_member_expr( ACM_Access *loc, const char *separator = ", " );
  bool is_wrapped_array( ACM_Name *ent );

  void preplanTransform( ACM_Code &code_jpl );
#endif

  void exec_join_point (ACM_Execution *loc);
  void cons_join_point (ACM_Construction *loc);
  void dest_join_point (ACM_Destruction *loc);
  void add_aspect_include (ACM_Any *jpl, AspectInfo &aspect_info,
                           AspectRef::Kind kind);
  void insert_aspect_includes ();
  AspectIncludes &aspect_includes () { return _aspect_includes; }
  void type_check (const ACM_Class *in, const string &name, bool result);

  void bypass_info_clear ();
  bool bypass_in_blacklist (ACM_Class *cls);
  void bypass_insert (ACM_Class *cls);

  // code transformation functions need for phase 1:
  struct TypeUse {
    bool that;
    bool target;
    bool result;
    bool entity;
    bool memberptr;
    bool array;

    inline TypeUse() : that( false ), target( false ), result( false ), entity( false ), memberptr( false ), array( false ) {};
    inline bool any() { return that || target || result || entity || memberptr || array; }
  };

  void transform_aspect(ACToken start);
  void transform_pointcut_def(ACToken start, ACToken virtual_token,
      ACToken assignment_token, ACToken pct_end_token);
  void transform_advice_code (int no, std::string kind, AdviceCodeContext context, bool has_args, TypeUse &uses_type,
      ACToken arg_begin, ACToken body_begin, ACToken body_end, const string &prot);
  void transform_aspectof (AdviceCodeContext context, TypeUse &uses_type,
      ACToken start, ACToken name, ACToken arg_begin,
      ACToken body_begin, ACToken body_end, const string &prot);
  void transform_delete (ACToken from, ACToken to);
  void transform_anon_class (ACToken name_pos, const string &name);
  void transform_anon_member_class (ACToken end_token, const string &anon_members);
};

#endif // __CodeWeaver_h__
