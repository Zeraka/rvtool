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

#include "AdviceInfo.h"
#include "AspectInfo.h"
#include "Binding.h"
#include "BackEndProblems.h"
#include "TransformInfo.h"

#include <string.h>
#include <sstream>
using std::ostringstream;

#ifdef FRONTEND_PUMA
#include "Puma/CSemDatabase.h"
#include "Puma/CArgumentInfo.h"
#endif

using namespace Puma;

AdviceInfo::AdviceInfo (AspectInfo &ai, ACM_AdviceCode &c) :
  _aspect_info (ai), _aspect (ai.loc ()), _code (c) {}

void AdviceInfo::gen_invocation_func (ostream &out, bool def,
                                      const BackEndProblems &bep) {

#ifdef FRONTEND_PUMA
  CFunctionInfo *ad_func = TI_AdviceCode::of (_code)->function ();
  CStructure *ad_cls = ad_func->ClassScope ();
#else
  clang::FunctionDecl *ad_func = TI_AdviceCode::of (_code)->decl ();
  clang::NamedDecl *ad_cls = llvm::cast<clang::NamedDecl>(ad_func->getParent ());
#endif
  
  const ThisJoinPoint &tjp = TI_AdviceCode::of (_code)->this_join_point ();
  const ThisJoinPoint &aspectof_tjp = _aspect_info.aspectof_this_join_point ();
    
  // determine whether the invocation functions needs JoinPoint, tjp, or Bind.
  int first_context_arg = ((_code.get_context() & ACC_OBJ) ? 1 : 0);
  bool has_context_var = (_code.get_context() & ACC_VARS);
  bool type_needed = tjp.type_advice_needed () || aspectof_tjp.type_advice_needed () ||
                     has_context_var;
  bool pointer_needed = tjp.pointer_advice_needed () || has_context_var ||
                        aspectof_tjp.pointer_advice_needed ();
    
  if (type_needed)
    out << "  template <class JoinPoint>" << endl;

  out << "  ";
  if (bep._use_always_inline)
    out << "__attribute((always_inline)) ";

  ostringstream suffix;
  suffix << signature(_aspect) << "_" << scope_name () << name ();
  out << "inline void invoke_" << suffix.str () << " (";

  if (pointer_needed)
    out << "JoinPoint *tjp";
  out << ")" << (def ? " {" : ";") << endl;

  // the declaration ends here
  if (!def)
    return;
      
  // generate typedefs
  // new implementation might have one argument (tjp)
#ifdef FRONTEND_PUMA
  for (int a = first_context_arg; a < (int)ad_func->Arguments (); a++)
#else
  for (int a = first_context_arg; a < (int)ad_func->getNumParams (); a++)
#endif
//    out << "    typedef typename Binding::template Arg<" << a << "> Arg" << a 
//        << ";" << endl;
    out << "    typedef typename JoinPoint::Binding_" << suffix.str ()
        << "::template Arg<" << (a - first_context_arg) << "> Arg"
        << (a - first_context_arg) << ";" << endl;

  // generate advice call
  out << "    ";
#ifdef FRONTEND_PUMA
  CClassInfo *aspect_cls = TI_Aspect::of (aspect ())->class_info();
  if (ad_cls != aspect_cls)
    out << "((::" << ad_cls->QualName () << "*)";
#else
  clang::Decl *aspect_cls = TI_Aspect::of (aspect ())->decl();
  if (ad_cls != aspect_cls)
    out << "((::" << ad_cls->getNameAsString () << "*)";
#endif
  // aspectof function
  out << "::" << signature (aspect ()) << "::aspectof";
  // generate <JoinPoint> if necessary
  if (aspectof_tjp.type_advice_needed () && !aspectof_tjp.pointer_advice_needed ())
    out << "<JoinPoint>";
  out << "(";
  if (aspectof_tjp.pointer_advice_needed ())
    out << "tjp";
  out << ")";
    
  if (ad_cls != aspect_cls)
    out << ")";
  out << "->";
  if (tjp.type_advice_needed () && !tjp.pointer_advice_needed())
    out << "template ";  
  out << name ();

  // generate <JoinPoint> if necessary
  if (tjp.type_advice_needed () && !tjp.pointer_advice_needed())
    out << "<JoinPoint>";
  out << " (";

  int nargs = 0;

  if (tjp.pointer_advice_needed()) {
    out << "tjp";
    nargs++;
  }

  // new implementation might have one argument (tjp)
#ifdef FRONTEND_PUMA
  for (int a = first_context_arg; a < (int)ad_func->Arguments (); a++) {
#else
  for (int a = first_context_arg; a < (int)ad_func->getNumParams (); a++) {
#endif
    if (nargs > 0)
       out << ", ";
#ifdef FRONTEND_PUMA
    out << "(" << *ad_func->Argument (a)->TypeInfo () << ")";
#else
      out << "(" << TI_Type::get_type_text(ad_func->getParamDecl(a)->getType(),
                                           &ad_func->getASTContext(),
                                           0,
                                           TSEF_ENABLE,
                                           false,
                                           TSEF_DONOTCHANGE,
                                           false,
                                           true,
                                           false);
      out << ")";
#endif
    out << "Arg" << (a - first_context_arg) << "::val (tjp)";
    nargs++;
  }

  out << ");" << endl;

  out << "  }" << endl;
}


void AdviceInfo::gen_invocation_func_call (ostream &stmt, const char* tjp_tp,
                                           const char *tjp_obj) {
  const ThisJoinPoint &tjp = TI_AdviceCode::of (_code)->this_join_point ();
  const ThisJoinPoint &aspectof_tjp = _aspect_info.aspectof_this_join_point ();
    
  // determine whether the invocation functions needs JoinPoint or tjp
  // new implementation has at least one argument
  int first_context_arg = ((_code.get_context() & ACC_OBJ) ? 1 : 0);
#ifdef FRONTEND_PUMA
  bool has_context = ((int) TI_AdviceCode::of (_code)->function ()->Arguments () > first_context_arg);
#else
  bool has_context = ((int) TI_AdviceCode::of (_code)->decl ()->getNumParams() > first_context_arg);
#endif
  bool type_needed = tjp.type_advice_needed () || aspectof_tjp.type_advice_needed () ||
                     has_context;
  bool pointer_needed = tjp.pointer_advice_needed () || has_context ||
                        aspectof_tjp.pointer_advice_needed ();
    
  stmt << "AC::invoke_" << signature (_aspect) << "_"
       << scope_name () << name ();
       
  if (type_needed) {
    stmt << "<" << tjp_tp;
//    if (has_context) {
//      string tp (tjp_tp);
//      tp.erase (tp.length () - 2, tp.length ());
//      stmt << ", Binding_" << tp << "_0_" << _aspect->name () << "_"
//           << Scope ()->Name () << "_" << (name () + 1);
//    }
    stmt << ">";
  }

  stmt << " (";
  if (pointer_needed)
    stmt << tjp_obj;
  stmt << ");";
}


void AdviceInfo::gen_binding_template (ostream &out, const char *jpname,
                                       const BackEndProblems &bep) {
  // later this will be passed as an argument
  Binding &bind = binding ();

  // no code generation if no context variables are expected
  if (!bind._used)
    return;
      
  // generate the mapping of context variables to advice function arguments
  out << "struct Binding_" /* << jpname << "_" */ << signature(_aspect) << "_"
      << scope_name () << "_" << (name ().c_str() + 1) << " {" << endl;
  out << "  typedef " /* << jpname*/ << "__TJP" << " TJP;" << endl;
  out << "  template <int I";
  if (bep._spec_scope)
    out << ", int DUMMY = 0";
  out << "> struct Arg {" << endl;
  out << "    void val (TJP *tjp) {} // for VC7" << endl;
  out << "  };" << endl;

  typedef ACM_Container<ACM_Arg, true> Container;
  Container &arguments = _code.get_pointcut()->get_args();
  int a = 0;
  for (Container::iterator i = arguments.begin (); i != arguments.end (); ++i, ++a) {
#ifdef FRONTEND_PUMA
    const Puma::CTypeInfo * curr_type = TI_Arg::of(*(*i))->type_info();
    const bool is_pointer = curr_type->isPointer();
    // Determine whether the current type is a reference type:
    const bool is_reference = curr_type->isAddress();
#else
    // Buffer the current clang::QualType:
    const clang::QualType curr_type = TI_Arg::of(*(*i))->type();
    const bool is_pointer = curr_type->isPointerType ();
    // Determine whether the current type is a reference type:
    const bool is_reference = curr_type->isReferenceType();
#endif
    int bind_index = bind.bound_to (*i);
    assert (bind_index != Binding::BIND_NOT_FOUND);
    out << "  template <";
    if (bep._spec_scope)
      out << "int DUMMY";
    out << "> struct Arg<" << a;
    if (bep._spec_scope)
      out << ", DUMMY";
    out << "> {" << endl
        << "    static typename TJP::";
    switch (bind_index) {
      case Binding::BIND_THAT:
        out << "That " << (is_pointer ? "*" : "&")
            << "val (TJP *tjp) { return " << (is_pointer ? "" : "*")
            << "tjp->that (); }" << endl;
        break;
      case Binding::BIND_TARGET:
        out << "Target " << (is_pointer ? "*" : "&")
            << "val (TJP *tjp) { return " << (is_pointer ? "" : "*")
            << "tjp->target (); }" << endl;
        break;
      case Binding::BIND_RESULT:
        out << "Res::ReferredType &val (TJP *tjp) { return " << (is_reference ? "*" : "") <<  "*tjp->result (); }" << endl;
        break;
      case Binding::BIND_NOT_FOUND: // already handled by assertion
        break;
      default: // argument
        out << "template Arg<" << bind_index << ">::ReferredType &val (TJP *tjp) {"
            << " return *tjp->template arg<" << bind_index << "> (); }" << endl;
    }
    out << "  };" << endl;
  }

  out << "};" << endl;  
}
std::string AdviceInfo::name () { return TI_AdviceCode::of (_code)->name (); }

void AdviceInfo::addTJPFlags(ThisJoinPoint &tjp) {
  tjp.merge (TI_AdviceCode::of (code ())->this_join_point ());
  tjp.merge (aspect_info().aspectof_this_join_point());

  ThisJoinPoint context_vars;
  context_vars.setup (binding ());
  tjp.merge (context_vars);
}

