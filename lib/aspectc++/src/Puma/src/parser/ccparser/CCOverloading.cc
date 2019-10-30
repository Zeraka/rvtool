// This file is part of PUMA.
// Copyright (C) 1999-2003  The PUMA developer team.
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

#include "Puma/CTemplateParamInfo.h"
#include "Puma/CCInstantiation.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/CCOverloading.h"
#include "Puma/CFunctionInfo.h"
#include "Puma/CArgumentInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CCNameLookup.h"
#include "Puma/CFctInstance.h"
#include "Puma/CUnionInfo.h"
#include "Puma/CClassInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CTree.h"

#include <string.h>

namespace Puma {


#define INTERNAL_ERROR(loc__,mesg__) \
  err << sev_fatal << loc__->token ()->location () \
      << "internal error: " << mesg__ << endMessage


////////////////////////////////////////////////////////////
// �13.3 overload resolution ///////////////////////////////
////////////////////////////////////////////////////////////


void CCOverloading::resolve (CTree *args, bool allow_user_defined) {
  CT_CallExpr *call;

  if (! Candidates () || ! args)
    return;

  user_def = allow_user_defined;
  _Poi = args;
  if (args->NodeName () == CT_CallExpr::NodeId ()) {
    call = (CT_CallExpr*)args;
    args = call->Arguments ();
    _Poi = call->Son (0);
  } else if (args->NodeName () != CT_ExprList::NodeId ())
    return;

  _Base = args;
  _Arguments = args;
  _NumArgs = ((CT_ExprList*)args)->Entries ();

  // start overload resolution algorithm
  chooseViableFcts ();
  chooseBestViableFct ();
}


void CCOverloading::resolve (CTree *base, CTree *arg0, bool allow_user_defined) {
  if (! Candidates () || ! base || ! arg0)
    return;

  user_def = allow_user_defined;
  _Base = base;
  _Arg0 = arg0;
  _NumArgs = 1;
  _Poi = arg0;

  // start overload resolution algorithm
  chooseViableFcts ();
  chooseBestViableFct ();
}


void CCOverloading::resolve (CTree *base, CTree *arg0, CTree *arg1, bool allow_user_defined) {
  if (! Candidates () || ! base || ! arg0 || ! arg1)
    return;

  user_def = allow_user_defined;
  _Base = base;
  _Arg0 = arg0;
  _Arg1 = arg1;
  _NumArgs = 2;
  _Poi = arg0;

  // start overload resolution algorithm
  chooseViableFcts ();
  chooseBestViableFct ();
}


void CCOverloading::resolveMember (CTree *args, CTree *member) {
  CT_CallExpr *call;

  if (! Candidates () || ! args)
    return;

  user_def = true;
  if (args->NodeName () == CT_CallExpr::NodeId ()) {
    call = (CT_CallExpr*)args;
    args = call->Arguments ();
  } else if (args->NodeName () != CT_ExprList::NodeId ())
    return;

  _Poi = member;
  _Base = args;
  _Arguments = args;
  _NumArgs = ((CT_ExprList*)args)->Entries ();

  // start overload resolution algorithm
  chooseViableFcts ();
  chooseBestViableFct ();
}


// �13.3.2 viable functions
void CCOverloading::chooseViableFcts () {
  bool has_ellipsis, is_method, implicit_this, real_inst, was_silent;
  CTree *arg_expr, *param_expr;
  unsigned num_params, offset;
  ErrorSeverity old_severity;
  CTypeInfo *ptype, *atype;
  CCConvSeq *seq, *seq2;
  CCandidateInfo *ci;
  CFunctionInfo *cf;
  CObjectInfo *info;
  CTypeList *params;
  CProject *project;

  // turn off error reporting
  project = 0;
  unsigned i = Candidates ();
  if (i) {
    project = Candidate (0)->Function ()->SemDB ()->Project ();
    was_silent = project->err ().silent ();
    old_severity = project->err ().severity ();
  }

  // check for each candidate if it is viable for the function call arguments
  for (; i > 0; i--) {
    ci = Candidate (i-1); // current candidate function
    cf = ci->Function ();
    real_inst = !cf->SemDB ()->Project ()->config ().Option ("--pseudo-instances");
    bool delayed_parse_problem = false;

    if (cf->isTemplate () && _Poi) {
      // instantiate function template
      implicit_this = cf->isMethod () && cf->isOperator ();
      CCInstantiation cci (err, false);
      for (unsigned j = (implicit_this ? 1 : 0); j < Arguments (); j++)
        cci.addArgument (Argument (j));
      info = cci.instantiate (_Poi, cf, true, 0, true);
      if (info && info->FunctionInfo ()) {
        cf = info->FunctionInfo ();
        ci->Function (cf);
        if (info->TemplateInstance()->hasDelayedParseProblem ()) {
          delayed_parse_problem = true;
        }
        if (! real_inst)
          dependent = true;
      // instantiation failed; not a viable function template
      } else {
        removeCandidate (i-1);
        continue;
      }
    }

    if (! real_inst && cf->TypeInfo ()->isDependent (false))
      dependent = true;

    params = cf->TypeInfo ()->ArgTypes ();
    is_method = false;
    offset = 0;

    // �13.3.1 contrive implicit object parameter (if necessary)
    if (cf->isMethod () && ! cf->isConstructor ()) {
      // skip all candidates that are less qualified than
      // the object they shall be called on
      // (exception: for static methods the qualifiers don't have to be checked)
      if (! cf->isStaticMethod () && obj_type &&
          ! equalOrMoreQualified (cf->TypeInfo (), obj_type)) {
        removeCandidate (i-1);
        continue;
      }

      atype = cf->Record ()->TypeInfo ();
      is_method = true;

      if (cf->isConversion ()) {
        ptype = cf->ConversionType ();
        if (! real_inst && ptype->isDependent (false))
          dependent = true;

        // �13.3.1.1.2.2 surrogate call function
        if ((ptype->isPointer () && 
             ptype->VirtualType ()->BaseType ()->isFunction ()) ||
            (ptype->isAddress () && ptype->isFunction ())) {
          // implicit object parameter conversion
          seq = new CCUserDefinedConvSeq;
          // 1. initial standard conversion sequence
          seq->addConversion (new CCIdentityConv (atype, atype));
          // 2. user-defined conversion
          seq->addConversion (new CCUserDefinedConv (ptype, atype, cf));
          // 3. second standard conversion sequence
          seq2 = implicitThisConv (ptype, cf);
          seq->addConversion (seq2);
          ci->addConvSequence (seq);
          ptype = ptype->isPointer () ? ptype->VirtualType ()->BaseType () : ptype;
          params = ptype->VirtualType ()->TypeFunction ()->ArgTypes ();
          cf = 0;
        // usual conversion function
        } else {
          // conversion functions do not have parameters 
          // (except the implicit object parameter)
          seq = implicitThisConv (atype, cf);
          ci->addConvSequence (seq);
          continue;
        }
      // non-conversion functions
      } else {
        seq = implicitThisConv (atype, cf);
        ci->addConvSequence (seq);
      }
    }

    // �13.3.2.2 the number of parameters shall agree in 
    // number of arguments of the function call;
    // in case of operator overloading the implicit object
    // parameter is considered to be a regular parameter
    // of the overloaded operator 
    if (operator_ovl && is_method) 
      offset = 1;
    num_params = params->Entries ();
    has_ellipsis = (num_params && params->Entry (num_params-1)->is_ellipsis ());
    num_params = has_ellipsis ? num_params-1 : num_params;
    if (! ((num_params+offset) == Arguments () ||
           // operators neither have variable number nor default arguments
           (! operator_ovl && 
            ((num_params < Arguments () && has_ellipsis) || 
             (hasDefaultArgument (cf, params, num_params)))))) {
      removeCandidate (i-1);
      continue;
    }

    // Remember that this candidate has a delayed parse problem with
    // an initializer of a default argument if a default argument had to
    // be used
    if (delayed_parse_problem &&
        hasDefaultArgument (cf, params, num_params))
      ci->markDelayedParseProblem ();

    // dependent call
    if (params->isDependent ())
      dependent = true;

    // �13.3.2.3 for each argument shall exist an implicit conversion 
    // sequence converting that argument to the corresponding parameter type
    for (unsigned j = 0; j < (Arguments ()-offset); j++) {
      // argument matches ellipsis parameter
      if (j >= num_params) {
        ci->addConvSequence (cvs.ellipsisConversions ());
        continue;
      }

      param_expr = cf->Arguments () > j ? cf->Argument (j)->Tree () : 0;
      ptype = params->Entry (j);
      arg_expr = Argument (j+offset);
      atype = arg_expr->Type ();
      if (! atype) { // for debugging purposes
        INTERNAL_ERROR (arg_expr, "missing type of argument expression");
        removeCandidate (i-1);
        break;
      }

      // try to convert argument expression implicitly to parameter type
      seq = implicitConversions (ptype, param_expr, atype, arg_expr);

      // no conversion applicable; not viable
      if (! seq) {
        removeCandidate (i-1);
        break;
      // implicit conversion sequence found
      } else 
        ci->addConvSequence (seq);
    }
  }

  // turn on error reporting if it was enabled before
  if (project) {
    if (! was_silent)
      project->err ().loud ();
    project->err ().severity (old_severity);
  }
}


// Check if tree is address of function expression and return the function name
CT_SimpleName* CCOverloading::isAddressOfFct (CTree* tree, bool address) const {
  if (! tree)
    return 0;
  else if (tree->NodeName () == CT_BracedExpr::NodeId ())
    return isAddressOfFct (((CT_BracedExpr*)tree)->Expr (), address);
  else if (tree->NodeName () == CT_AddrExpr::NodeId () && !address)
    return isAddressOfFct (((CT_AddrExpr*)tree)->Expr (), true);
  else if (tree->IsSimpleName () && address)
    return tree->Type () && tree->Type ()->TypeFunction () ? (CT_SimpleName*)tree : 0;
  return 0;
}


// Get the argument list of the function type parameter
CT_ArgDeclList* CCOverloading::getArgumentList (CTree* tree) const {
  if (tree && tree->NodeName () == CT_ArgDecl::NodeId ()) {
    tree = ((CT_ArgDecl*)tree)->Declarator ();
    if (tree && tree->NodeName () == CT_FctDeclarator::NodeId ()) {
      return ((CT_FctDeclarator*)tree)->Arguments ();
    }
  }
  return 0;
}


// Try to convert the function argument to the function parameter type
CCConvSeq *CCOverloading::implicitConversions (CTypeInfo *ptype, CTree *param_expr, CTypeInfo *atype, CTree *arg_expr) {
  CT_SimpleName* fctname = ptype->isPointer () && ptype->PtrToFct () ? isAddressOfFct (arg_expr) : 0;
  if (! fctname)
    return cvs.implicitConversions (ptype, atype, arg_expr, _Base, user_def);

  // argument is address of function; type of argument depends on parameter type
  // and a matching function in current or qualified scope

  // get current scope
  CStructure* scope = 0;
  CTree* tree = arg_expr;
  while (tree && (! tree->SemScope () || ! (scope = tree->SemScope ()->Scope ())))
    tree = tree->Parent ();
  if (! scope && fctname->Name ()->Object () && fctname->Name ()->Object ()->Scope ())
    scope = fctname->Name ()->Object ()->Scope ()->Structure ();
  if (! scope)
    return 0;

  // lookup function name in current scope and above
  CCNameLookup nl (err, scope);
  nl.lookup (fctname, scope, true, true);

  // check each function found
  CCConvSeq *last_seq = 0, *seq;
  CFunctionInfo *info, *last_info = 0;
  for (unsigned i = 0; i < nl.Objects (); i++) {
    info = nl.Object (i)->FunctionInfo ();

    if (last_info && *info == *last_info)
      continue; // same function
    last_info = info;

    if (info && info->isTemplate ()) {
      // instantiate function template
      CT_ArgDeclList* args = getArgumentList (param_expr);
      if (! args)
        continue;

      CCInstantiation cci (err, false);
      int entries = args->Entries ();
      for (int j = 0; j < entries; j++)
        cci.addArgument (args->Entry (j));

      CObjectInfo* oinfo = cci.instantiate (fctname, info, true, 0, true);
      info = oinfo ? oinfo->FunctionInfo () : 0;
    }

    if (! info)
      continue;

    if (info->isClassMember() && ! info->isStatic ()) {
      // method
      CTypeMemberPointer ptrToMembFct(info->TypeInfo (), info->Record ());
      seq = cvs.implicitConversions (ptype, &ptrToMembFct, arg_expr, _Base, user_def);
    } else {
      // function
      CTypePointer ptrToFct(info->TypeInfo ());
      seq = cvs.implicitConversions (ptype, &ptrToFct, arg_expr, _Base, user_def);
    }

    if (seq) {
      if (last_seq) {
        // ambiguous, can not resolve
        delete seq;
        last_seq = 0;
        break;
      } else {
        last_seq = seq;
      }
    }
  }
  return last_seq;
}


// �13.3.3 best viable function
void CCOverloading::chooseBestViableFct () {
  CCandidateInfo *challenger, *champion;
  unsigned candidates;

  candidates = Candidates ();
  if (candidates <= 1)
    return;

  // �13.3.3.1 run a simple tournament
  champion = Candidate (candidates-1);
  for (unsigned i = candidates-1; i > 0; i--) {
    challenger = Candidate (i-1); // current challenger candidate function

    // same candidate functions do not face each other
    if (*challenger->Function () == *champion->Function ()) {
      removeCandidate (i-1); 
      continue;
    }

    // let champion face the challenger 
    switch (compareCandidates (champion, challenger)) {
      // champion has won the round 
      case 1: 
        // elliminate challenger
        removeCandidate (i-1); 
        break;
      // challenger has won the round
      case -1:
        // elliminate old champion
        removeCandidate (i); 
        // challenger is the new champion
        champion = challenger; 
        break;
      // challenger and champion are equal
      default:
        // case 0: overload resolution ambiguous up till now
        champion = challenger; 
        break;
    }
  }
  
  // verify that the champion is better than all other remaining 
  // candidates (the champion did not yet faced)
  for (unsigned i = Candidates (); i > 0; i--) {
    challenger = Candidate (i-1); 
    if (challenger == champion)
      continue;
      
    if (*challenger->Function () == *champion->Function () ||
        compareCandidates (champion, challenger) == 1) {
      removeCandidate (i-1);     
    }
  }
}


// �13.3.3.1 compare two viable functions F1 and F2
// return 1 if F1 is a better candidate than F2
// return 0 if F1 is neither better nor worse than F2
// return -1 if F1 is a worse candidate than F2
int CCOverloading::compareCandidates (CCandidateInfo *c1, CCandidateInfo *c2) const {
  unsigned num_ics1, num_ics2, first_ics;
  CFunctionInfo *f1, *f2;
  bool c1_better, c2_better;

  c1_better = c2_better = false;
  f1 = c1->Function ();
  f2 = c2->Function ();
  num_ics1 = c1->ConvSequences ();
  num_ics2 = c2->ConvSequences ();

  // ??? compare member with non-member or constructor ???
  if (num_ics1 != num_ics2) { 
    INTERNAL_ERROR (Argument (0), "comparing member with non-member or constructor");
    return 0;
  }
    
  // comparison with implicit object parameter of static member 
  // function always yields 0 so skip it
  if (f1->isStaticMethod () || f2->isStaticMethod ())
    first_ics = 1;
  else
    first_ics = 0;

  // compare conversion sequences of the arguments
  for (unsigned i = first_ics; i < num_ics1; i++) {
    switch (cvs.compareConvSeq (c1->ConvSequence (i), c2->ConvSequence (i))) {
      case 1:
        c1_better = true;
        break;
      case -1:
        // worse
        c2_better = true;
        break;
      default: 
        // case 0: not worse
        break;
    }
  }
  if (c1_better && ! c2_better)
    return 1;
  if (! c1_better && c2_better)
    return -1;
  
  // non-template functions are better than template functions
  if (! f1->FctInstance () && f2->FctInstance ())
    return 1;
  if (f1->FctInstance () && ! f2->FctInstance ())
    return -1;
  
  // a more specialized template function is better than a less 
  // specialized template function
  if (f1->FctInstance () && f2->FctInstance ()) {
    switch (moreSpecialized (f1, f2)) {
      case 1:
        // f1 more specialized
        return 1;
      case -1:
        // f2 more specialized
        return -1;
      default: 
        // case 0: equal specialized
        break;
    }
  }
  
  // context is initialization by user-defined conversion (�13.3.1.5)
  // F1 is better than F2 if second standard conversion sequence of
  // F1 is better than second standard conversion sequence of F2
  if (c1->SecondConvSeq ()) 
    return cvs.compareConvSeq (c1->SecondConvSeq (), c2->SecondConvSeq ());

  return 0;
}


// create conversion for implicit function parameter
CCStandardConvSeq *CCOverloading::implicitThisConv (CTypeInfo *ptype, CFunctionInfo *cf) const {
  CCStandardConvSeq *seq;
  CTypeFunction *ft;
  bool c, v;

  c = v = false;
  ft = cf->TypeInfo ();

  if (! obj_type || (! obj_type->isConst () && ! obj_type->isVolatile ())) {
    // add const and volatile qualification
    if (ft->isConst () && ft->isVolatile ())
      v = c = true;
    // add const qualification
    else if (ft->isConst ())
      c = true;
    // add volatile qualification
    else if (ft->isVolatile ())
      v = true;
  } else if (obj_type) {
    if (obj_type->isConst () && obj_type->isVolatile ()) {
      // identity conversion
    } else if (obj_type->isConst ()) {
      // add volatile qualification
      if (ft->isVolatile ())
        v = true;
    } else if (obj_type->isVolatile ()) {
      // add const qualification
      if (ft->isConst ())
        c = true;
    }
  }

  ptype = ptype->Duplicate ();

  if (obj_type && (obj_type->isConst () || obj_type->isVolatile ())) {
    if (! ptype->TypeQualified ()) {
      ptype = new CTypeQualified (ptype, false, false, false);
    }
    ptype->TypeQualified ()->isConst (obj_type->isConst ());
    ptype->TypeQualified ()->isVolatile (obj_type->isVolatile ());
  }

  seq = new CCStandardConvSeq;
  seq->isReferenceBinding (true);

  if (c || v) {
    if (! ptype->TypeQualified ()) {
      ptype = new CTypeQualified (ptype, false, false, false);
    }
    if (c)
      ptype->TypeQualified ()->isConst (true);
    if (v)
      ptype->TypeQualified ()->isVolatile (true);
    seq->addConversion (new CCQualificationConv (ptype, obj_type));
  } else {
    seq->addConversion (new CCIdentityConv (ptype, obj_type));
  }

  CTypeInfo::Destroy (ptype);
  return seq;
}


bool CCOverloading::equalOrMoreQualified (CTypeFunction *t1, CTypeInfo *t2) const {
  unsigned t1_qual, t2_qual;
  t1_qual = (t1->isConst ()?2:0) + (t1->isVolatile ()?1:0);
  t2_qual = (t2->isConst ()?2:0) + (t2->isVolatile ()?1:0);
  if (t1_qual == t2_qual || t1_qual == 3 || t2_qual == 0)
    return true;
  return false;
}


CTree *CCOverloading::Argument (unsigned a) const {
  if (_Arguments)
    return ((CT_ExprList*)_Arguments)->Entry (a);
  else switch (a) {
    case 0: return _Arg0;
    case 1: return _Arg1;
    case 2: return _Arg2;
    default: return (CTree*)0;
  }
}


// collect conversion functions of cu (class or union)
// and its base classes that are visible in cu
void CCOverloading::collectConvFunctions (CRecord *cu) {
  collectConvFunctions (cu, Candidates ());
}


void CCOverloading::collectConvFunctions (CRecord *cu, unsigned first) {
  CFunctionInfo *fct;
  CClassInfo *cinfo;
  bool hidden;

  // ensure this class is fully instantiated
  if (cu->ClassInfo())
    cu->TypeInfo()->TypeClass()->instantiate(cu->Parent()->Structure());

  for (unsigned i = 0; i < cu->Functions (); i++) {
    fct = cu->Function (i);

    if (fct->isConversion ()) {
      // apply name hiding rules
      hidden = false;
      for (unsigned j = Candidates (); j > first; j--) {
        if (*Candidate (j-1)->Function ()->ConversionType () ==
            *fct->ConversionType ()) {
          hidden = true;
          break;
        }
      }
      if (! hidden)
        addCandidate (fct);
    }
  }

  // also consider base classes
  cinfo = cu->ClassInfo ();
  if (cinfo)
    for (unsigned i = 0; i < cinfo->BaseClasses (); i++)
      collectConvFunctions (cinfo->BaseClass (i)->Class (), first);
}


// collect converting constructors of cu (class or union)
void CCOverloading::collectConstructors (CRecord *cu, bool default_constr) {
  CFunctionInfo *fct;

  // ensure this class is fully instantiated
  if (cu->ClassInfo())
    cu->TypeInfo()->TypeClass()->instantiate(cu->Parent()->Structure());

  cu = cu->DefObject ()->Record ();
  for (unsigned i = 0; i < cu->Functions (); i++) {
    fct = cu->Function (i);
    // non-default non-explicit constructors are converting
    // constructors (i.e. constructors with parameters)
    if (fct->isConstructor () && ! fct->isExplicit () && 
        (default_constr || fct->Arguments () > 0))
      addCandidate (fct);
  }
}


void CCOverloading::addCandidates (CCNameLookup &nl) {
  CObjectInfo *info;
  for (unsigned i = 0; i < nl.Objects (); i++) {
    info = nl.Object (i);
    if (! info->FunctionInfo ())
      continue;
    addCandidate (info->FunctionInfo ());
  }
}


void CCOverloading::addCandidate (CFunctionInfo *fi) {
  bool is_template = fi->isTemplate ();
  for (unsigned i = Candidates (); i > 0; i--) {
    CFunctionInfo* f = Candidate (i-1)->Function ();
    if (f == fi)
      return;
    if (! is_template && *f == *fi) {
      if (f->isFctDef () || ! fi->isFctDef ())
        return;
      removeCandidate (i-1);
      break;
    }
  }
  _Candidates.append (new CCandidateInfo (fi)); 
}


bool CCOverloading::hasDefaultArgument (CFunctionInfo *finfo, 
 CTypeList *params, unsigned num_params) const {
  if (num_params <= Arguments ())
    return false;
  
  if (finfo) {
    if (finfo->FctInstance ())
      // In case of a function template's instance, we have to check
      // whether the function template has a default argument (and not
      // the instance), because the default argument might have been
      // hidden from the instance parser, because the initializer was
      // a delayed parse tree.
      return finfo->FctInstance()->TemplateInstance()->Template ()->ObjectInfo()->FunctionInfo ()->hasDefaultArgument (Arguments ());
    else
      return (finfo->hasDefaultArgument (Arguments ()));
  }
  
  // in case of surrogate call function
  return params->ArgumentList () &&
         ((CT_ArgDecl*)params->ArgumentList ()->
         Entry (Arguments ()))->Initializer ();
}


void CCOverloading::ObjectType (CTypeInfo *t) { 
  obj_type = t; 
  while (obj_type && obj_type->TypeAddress ())
    obj_type = obj_type->BaseType ();
}


int CCOverloading::moreSpecialized (CFunctionInfo *f1, CFunctionInfo *f2) const {
  CTemplateInstance *inst1 = f1->TemplateInstance ();
  CTemplateInstance *inst2 = f2->TemplateInstance ();
  f1 = inst1->Template ()->ObjectInfo ()->FunctionInfo ();
  f2 = inst2->Template ()->ObjectInfo ()->FunctionInfo ();

  InstantiationCandidate cand1;
  cand1.initialize (inst1->PointOfInstantiation (), f1, inst1->Template ());
  unsigned numargs = Arguments () > f2->Arguments () ? f2->Arguments () : Arguments ();
  unsigned offset = f2->isMethod () && ! f1->isMethod () ? 1 : 0;
  for (unsigned i = offset; i < numargs; i++)
    cand1.addArgument (f2->Argument (i)->Tree ());

  InstantiationCandidate cand2;
  cand2.initialize (inst2->PointOfInstantiation (), f2, inst2->Template ());
  numargs = Arguments () > f1->Arguments () ? f1->Arguments () : Arguments ();
  offset = f1->isMethod () && ! f2->isMethod () ? 1 : 0;
  for (unsigned i = offset; i < numargs; i++)
    cand2.addArgument (f1->Argument (i)->Tree ());

  // perform argument deduction against the other function
  bool f1_at_least_as_specialized = cand2.deduceArgumentsFromFctCall (0, false, true);
  bool f2_at_least_as_specialized = cand1.deduceArgumentsFromFctCall (0, false, true);

  // equal specialized
  if (f1_at_least_as_specialized && f2_at_least_as_specialized)
    return 0;
  // f2 more specialized
  if (f2_at_least_as_specialized)
    return -1;
  // f1 more specialized
  if (f1_at_least_as_specialized)
    return 1;
  // equal specialized
  return 0;
}


////////////////////////////////////////////////////////////
// �13.6 built-in operators ////////////////////////////////
////////////////////////////////////////////////////////////


// create built-in operators used as candidates for operator overloading
void CCOverloading::createBuiltinOperators (CSemDatabase *db,
  const char *opname, int oper, CTree *arg0, CTree *arg1) {
  unsigned len0, len1;
  CTypeInfo *t0, *t1;
  CRecord *record;
   
  // do not create operators if not in operator overloading mode
  if (! operator_ovl)
    return;
    
  // �13.3.1.2.3 for the operators ',', '->', and unary '&' the set is empty
  if (oper == TOK_COMMA || oper == TOK_PTS || 
      (oper == TOK_AND && ! arg1))
    return;
    
  // for all other operators, the built-in candidates include all of 
  // the candidate operator functions defined in �13.6 that, compared 
  // to the given operator,
  //   - have the same operator name, and
  //   - accept the same number of operands, and
  //   - accept operand types to which the given operand or operands
  //     can be converted according to �13.3.3.1 (implicit conversion
  //     sequences), and
  //   - do not have the same parameter type list as any non-template 
  //     non-member candidate
  
  t0 = arg0 ? arg0->Type () : 0;
  t1 = arg1 ? arg1->Type () : 0;
  t0 = t0 ? t0->UnqualType () : t0;
  t1 = t1 ? t1->UnqualType () : t1;
  if (! t0)
    return;

  // �13.6 set of built-in operator candidate funtions

  // operator ||, operator &&, operator !
  // (these operators are always the same)
  if (oper == TOK_OR_OR || oper == TOK_AND_AND || oper == TOK_NOT)
    createLogOp (db, oper, opname, t0, t1);
  else {
    // collect the types for the operands of the operators to be created
    Array<CTypeInfo*> types0; types0.append (t0);
    Array<CTypeInfo*> types1; types1.append (t1);
  
    // collect the types the first operand type can be converted to
    if (t0->isRecord ()) {
      record = t0->VirtualType ()->TypeRecord ()->Record ();
      collectConvTypes (record, types0);
    }
    // collect the types the second operand type can be converted to
    if (t1 && t1->isRecord ()) {
      record = t1->VirtualType ()->TypeRecord ()->Record ();
      collectConvTypes (record, types1);
    }

    len0 = types0.length ();
    len1 = types1.length ();
    for (unsigned i = 0; i < len0; i++) {
      t0 = types0[i];
      if (t0->TypeAddress ()) {
        t0 = t0->BaseType ();
      }
    
      for (unsigned j = 0; j < len1; j++) {
        t1 = types1[j];
        if (t1 && t1->TypeAddress ()) {
          t1 = t1->BaseType ();
        }
        
        // operator ++
        if (oper == TOK_INCR)
          createIncrOp (db, oper, opname, t0, t1);
        // operator --
        else if (oper == TOK_DECR)
          createDecrOp (db, oper, opname, t0, t1);
        // operator *
        else if (oper == TOK_MUL)
          createMulOp (db, oper, opname, t0, t1);
        // operator +
        else if (oper == TOK_PLUS)
          createPlusOp (db, oper, opname, t0, t1);
        // operator -
        else if (oper == TOK_MINUS)
          createMinusOp (db, oper, opname, t0, t1);
        // operator ~
        else if (oper == TOK_TILDE)
          createTildeOp (db, oper, opname, t0, t1);
        // operator ->*
        else if (oper == TOK_PTS_STAR) 
          createMembPtrOp (db, oper, opname, t0, t1);
        // operator []
        else if (oper == TOK_OPEN_SQUARE) // first token of []
          createIndexOp (db, oper, opname, t0, t1);
        // operator ?
        else if (oper == TOK_QUESTION) // cannot be overloaded! see �13.6.24
          createIfThenOp (db, oper, opname, t0, t1);
        // operator <, operator >, operator <=, operator >=
        else if (oper == TOK_LESS || oper == TOK_GREATER || 
                 oper == TOK_LEQ || oper == TOK_GEQ) 
          createRelOp (db, oper, opname, t0, t1);
        // operator %, operator &, operator ^, operator |, operator <<, operator >>
        else if (oper == TOK_MODULO || oper == TOK_AND || oper == TOK_ROOF || 
                 oper == TOK_OR || oper == TOK_LSH || oper == TOK_RSH) 
          createBinOp (db, oper, opname, t0, t1);
        // operator %=, operator &=, operator ^=, operator |=, operator <<=, operator >>=
        else if (oper == TOK_MOD_EQ || oper == TOK_AND_EQ || oper == TOK_XOR_EQ || 
                 oper == TOK_IOR_EQ || oper == TOK_LSH_EQ || oper == TOK_RSH_EQ) 
          createEqAssOp (db, oper, opname, t0, t1);
        // operator =
        else if (oper == TOK_ASSIGN)
          createAssOp (db, oper, opname, t0, t1);
        // operator /
        else if (oper == TOK_DIV) 
          createDivOp (db, oper, opname, t0, t1);
        // operator ==, operator !=
        else if (oper == TOK_EQL || oper == TOK_NEQ) 
          createEqOp (db, oper, opname, t0, t1);
        // operator *=, operator /=
        else if (oper == TOK_MUL_EQ || oper == TOK_DIV_EQ) 
          createMulAssOp (db, oper, opname, t0, t1);
        // operator +=, operator -=
        else if (oper == TOK_ADD_EQ || oper == TOK_SUB_EQ)
          createAddAssOp (db, oper, opname, t0, t1);
      }
    }
  }
}


// operator ++
void CCOverloading::createIncrOp (CSemDatabase *db, int oper, 
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) { 
  CTypeInfo *t;
  
  if (t1) // unary operators do not have two operands
    return;
  
  if (t0->isConst ()) 
    return;
  
  // 3. For every pair (T, VQ), where T is an arithmetic type, and VQ is either 
  // volatile or empty, there exist candidate operator functions of the form
  //   VQ T& operator++(VQ T&);
  //   T     operator++(VQ T&, int);
  if (t0->isArithmetic ()) {
    t = new CTypeAddress (t0->Duplicate ());
    createOperator (db, oper, opname, t->Duplicate (), t->Duplicate ());
    createOperator (db, oper, opname, t0->UnqualType ()->Duplicate (), 
                    t, &CTYPE_INT);
  }

  // 5. For every pair (T, VQ), where T is a cv-qualified or cv-unqualified 
  // object type, and VQ is either volatile or empty, there exist candidate 
  // operator functions of the form
  //   T*VQ& operator++(T*VQ&);
  //   T*    operator++(T*VQ&, int);
  if (t0->isObject () && t0->isPointer ()) {
    t = t0->UnqualType ()->Duplicate ();
    if (t0->isVolatile ())
      t = new CTypeQualified (t, false, true, false);
    t = new CTypeAddress (t);
    t0 = t0->UnqualType ()->Duplicate ();
    createOperator (db, oper, opname, t->Duplicate (), t->Duplicate ());
    createOperator (db, oper, opname, t0, t, &CTYPE_INT);
  }
}


// operator --
void CCOverloading::createDecrOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  if (t1) // unary operators do not have two operands
    return;

  if (t0->isConst ()) 
    return;

  // 4. For every pair (T, VQ), where T is an arithmetic type other than bool, 
  // and VQ is either volatile or empty, there exist candidate operator 
  // functions of the form
  //   VQ T& operator--(VQ T&);
  //   T     operator--(VQ T&, int);
  if (t0->isArithmetic () && ! t0->UnqualType ()->is_bool ()) {
    t = new CTypeAddress (t0->Duplicate ());
    createOperator (db, oper, opname, t->Duplicate (), t->Duplicate ());
    createOperator (db, oper, opname, t0->UnqualType ()->Duplicate (), 
                    t, &CTYPE_INT);
  }

  // 5. For every pair (T, VQ), where T is a cv-qualified or cv-unqualified 
  // object type, and VQ is either volatile or empty, there exist candidate 
  // operator functions of the form
  //   T*VQ& operator--(T*VQ&);
  //   T*    operator--(T*VQ&, int);
  if (t0->isObject () && t0->isPointer ()) {
    t = t0->UnqualType ()->Duplicate ();
    if (t0->isVolatile ())
      t = new CTypeQualified (t, false, true, false);
    t = new CTypeAddress (t);
    t0 = t0->UnqualType ()->Duplicate ();
    createOperator (db, oper, opname, t->Duplicate (), t->Duplicate ());
    createOperator (db, oper, opname, t0, t, &CTYPE_INT);
  }
}


// operator *
void CCOverloading::createMulOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  // 6. For every cv-qualified or cv-unqualified object type T, there exist 
  // candidate operator functions of the form
  //   T& operator*(T*);
  // 7. For every function type T, there exist candidate operator functions 
  // of the form
  //   T& operator*(T*);
  if (! t1 && t0->isPointer ()) {
    t = new CTypeAddress (t0->PtrBaseType ()->Duplicate ());
    createOperator (db, oper, opname, t, t0->Duplicate ());
  }

  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   LR   operator*(L, R);
  if (t1 && t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    t = cvs.usualArithmeticConv (t0, t1);
    createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ());
  }
}


// operator +
void CCOverloading::createPlusOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t = 0;
  
  // 8. For every type T, there exist candidate operator functions of the form
  //   T* operator+(T*);
  if (! t1 && t0->isPointer ()) {
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate ());
  }

  // 13. For every cv-qualified or cv-unqualified object type T there exist 
  // candidate operator functions of the form
  //   T* operator+(T*, ptrdiff_t);
  //   T* operator+(ptrdiff_t, T*);
  if (t1 && t0->isObject () && t0->isPointer ()) {
    t = CTypeInfo::CTYPE_PTRDIFF_T; // ptrdiff_t
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate (), t);
    createOperator (db, oper, opname, t0->Duplicate (), t, t0->Duplicate ());
  }

  // 9. For every promoted arithmetic type T, there exist candidate operator 
  // functions of the form
  //   T operator+(T);
  if (! t1 && t0->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate ()); 
  }

  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   LR   operator+(L, R);
  if (t1 && t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    t = cvs.usualArithmeticConv (t0, t1);
    createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator -
void CCOverloading::createMinusOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t = 0;

  // 13. For every cv-qualified or cv-unqualified object type T there exist 
  // candidate operator functions of the form
  //   T* operator-(T*, ptrdiff_t);
  if (t1 && t0->isObject () && t0->isPointer ()) {
    t = CTypeInfo::CTYPE_PTRDIFF_T; // ptrdiff_t
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate (), t);
  }

  // 14. For every T, where T is a pointer to object type, there exist candidate
  // operator functions of the form
  //   ptrdiff_t operator-(T, T);
  if (t1 && t0->isPointer () && t0->PtrBaseType ()->isObject ()) {
    t = CTypeInfo::CTYPE_PTRDIFF_T; // ptrdiff_t
    createOperator (db, oper, opname, t, t0->Duplicate (), t0->Duplicate ());
  }
  if (t1 && t1->isPointer () && t1->PtrBaseType ()->isObject ()) {
    t = CTypeInfo::CTYPE_PTRDIFF_T; // ptrdiff_t
    createOperator (db, oper, opname, t, t1->Duplicate (), t1->Duplicate ());
  }

  // 9. For every promoted arithmetic type T, there exist candidate operator 
  // functions of the form
  //   T operator-(T);
  if (! t1 && t0->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate ()); 
  }

  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   LR   operator-(L, R);
  if (t1 && t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    t = cvs.usualArithmeticConv (t0, t1);
    createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator ~
void CCOverloading::createTildeOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  if (t1) // unary operators do not have two operands
    return;

  // 10. For every promoted integral type T, there exist candidate operator 
  // functions of the form
  //   T operator~(T);
  if (t0->isInteger ()) {
    t0 = cvs.integralPromotion (t0);
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate ());
  }
}


// operator ->*
void CCOverloading::createMembPtrOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  CRecord *c0, *c1;
  
  if (! t1) // binary operators must have two operands
    return;

  // 11. For every quintuple (C1, C2, T, CV1, CV2), where C2 is a class type, 
  // C1 is the same type as C2 or is a derived class of C2, T is an object 
  // type or a function type, and CV1 and CV2 are cv-qualifier-seqs, there
  // exist candidate operator functions of the form (where CV12 is the union 
  // of CV1 and CV2)
  //   CV12 T& operator->*(CV1 C1*, CV2 T C2::*);
  if (t1->TypeMemberPointer () && t0->isPointer () && t0->PtrBaseType ()->isRecord ()) {
    c1 = t1->TypeMemberPointer ()->Record ();
    c0 = t0->PtrBaseType ()->VirtualType ()->TypeRecord ()->Record ();
    if (CCSemExpr::sameOrBaseClass (c1, c0)) {
      t = t1->VirtualType ()->BaseType ()->Duplicate ();
      if (t0->PtrBaseType ()->isQualified ()) {
        if (t->isQualified ()) {
          if (t0->PtrBaseType ()->isConst ()) 
            t->TypeQualified ()->isConst (true);
          if (t0->PtrBaseType ()->isVolatile ()) 
            t->TypeQualified ()->isVolatile (true);
        } else {
          t = new CTypeQualified (t, t0->PtrBaseType ()->isConst (), 
                                  t0->PtrBaseType ()->isVolatile (), false);
        }
      }
      t = new CTypeAddress (t);
      createOperator (db, oper, opname, t, t0->Duplicate (), t1->Duplicate ());
    }
  } 
}


// operator []
void CCOverloading::createIndexOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *rt, *pt, *t;
  
  if (! t1) // binary operators must have two operands
    return;
  
  // 13. For every cv-qualified or cv-unqualified object type T there exist 
  // candidate operator functions of the form
  //   T& operator[](T*, ptrdiff_t);
  //   T& operator[](ptrdiff_t, T*);
  if (t0->isObject () && t0->isPointer ()) {
    rt = new CTypeAddress (t0->PtrBaseType ()->Duplicate ());
    pt = t0->Duplicate ();
    t = CTypeInfo::CTYPE_PTRDIFF_T; // ptrdiff_t
    createOperator (db, oper, opname, rt->Duplicate (), pt->Duplicate (), t); 
    createOperator (db, oper, opname, rt, t, pt);
  }
}


// operator ?
void CCOverloading::createIfThenOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;

  if (! t1) // binary operators must have two operands
    return;
  
  // 25. For every type T, where T is a pointer or pointer-to-member type, there exist 
  // candidate operator functions of the form
  //   T operator?(bool, T, T);   // first operand skipped!!!
  if (t0->isMemberPointer () || t0->isPointer ()) {
    createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate (), t0->Duplicate ());
  }
  if (t1->isMemberPointer () || t1->isPointer ()) {
    createOperator (db, oper, opname, t1->Duplicate (), t1->Duplicate (), t1->Duplicate ());
  }

  // 24. For every pair of promoted arithmetic types L and R, there exist candidate 
  // operator functions of the form (where LR is the result of the usual arithmetic 
  // conversions between types L and R)
  //   LR operator?(bool, L, R);   // first operand skipped!!!
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    t = cvs.usualArithmeticConv (t0, t1);
    createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator ||, operator &&, operator !
void CCOverloading::createLogOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  // 23. There also exist candidate operator functions of the form
  //   bool operator!(bool);
  //   bool operator&&(bool, bool);
  //   bool operator||(bool, bool);
  if (! t1) {
    createOperator (db, oper, opname, &CTYPE_BOOL, &CTYPE_BOOL);
  } else {
    createOperator (db, oper, opname, &CTYPE_BOOL, &CTYPE_BOOL, &CTYPE_BOOL);
  }
}


// operator <, operator >, operator <=, operator >=
void CCOverloading::createRelOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  if (! t1) // binary operators must have two operands
    return;

  // 15. For every pointer or enumeration type T, there exist candidate operator
  // functions of the form
  //   bool operator<(T, T);
  //   bool operator>(T, T);
  //   bool operator<=(T, T);
  //   bool operator>=(T, T);  
  if (t0->isPointer () || t0->isEnum ()) {
    createOperator (db, oper, opname, &CTYPE_BOOL, t0->Duplicate (), t0->Duplicate ());
  } 
  if (t1->isPointer () || t1->isEnum ()) {
    createOperator (db, oper, opname, &CTYPE_BOOL, t1->Duplicate (), t1->Duplicate ());
  } 
  
  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   bool operator<(L, R);
  //   bool operator>(L, R);
  //   bool operator<=(L, R);
  //   bool operator>=(L, R);
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    createOperator (db, oper, opname, &CTYPE_BOOL, t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator %, operator &, operator ^, operator |, operator <<, operator >>
void CCOverloading::createBinOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  if (! t1) // binary operators must have two operands
    return;

  // 17. For every pair of promoted integral types L and R, there exist candidate
  // operator functions of the form (where LR is the result of the usual arithmetic 
  // conversions between types L and R)
  //   LR operator%(L, R);
  //   LR operator&(L, R);
  //   LR operator^(L, R);
  //   LR operator|(L, R);
  //   L  operator<<(L, R);
  //   L  operator>>(L, R);
  if (t0->isInteger () && t1->isInteger ()) {
    t0 = cvs.integralPromotion (t0);
    t1 = cvs.integralPromotion (t1);
    if (*(opname+9) == '<' || *(opname+9) == '>') {
      createOperator (db, oper, opname, t0->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
    } else {
      t = cvs.usualArithmeticConv (t0, t1);
      createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
    }
  }
}


// operator %=, operator &=, operator ^=, operator |=, operator <<=, operator >>=
void CCOverloading::createEqAssOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  if (! t1) // binary operators must have two operands
    return;

  // 22. For every triple (L, VQ, R), where L is an integral type, VQ is either 
  // volatile or empty, and R is a promoted integral type, there exist candidate 
  // operator functions of the form
  //   VQ L& operator%=(VQ L&, R);
  //   VQ L& operator<<=(VQ L&, R);
  //   VQ L& operator>>=(VQ L&, R);
  //   VQ L& operator&=(VQ L&, R);
  //   VQ L& operator^=(VQ L&, R);
  //   VQ L& operator|=(VQ L&, R);
  if (t0->isInteger () && t1->isInteger () && ! t0->isConst ()) {
    t1 = cvs.integralPromotion (t1);
    t0 = new CTypeAddress (t0->Duplicate ());
    createOperator (db, oper, opname, t0, t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator =
void CCOverloading::createAssOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  if (! t1) // binary operators must have two operands
    return;

  if (t0->isConst ()) 
    return;

  // 19. For every pair (T, VQ), where T is any type and VQ is either volatile or
  // empty, there exist candidate operator functions of the form
  //   T*VQ& operator=(T*VQ&, T*);
  if (t0->isPointer ()) {
    t = t0->UnqualType ()->Duplicate ();
    if (t0->isVolatile ())
      t = new CTypeQualified (t, false, true, false);
    t = new CTypeAddress (t);
    createOperator (db, oper, opname, t, t->Duplicate (), t0->UnqualType ()->Duplicate ());
  }

  // 20. For every pair (T, VQ), where T is an enumeration or pointer to member type 
  // and VQ is either volatile or empty, there exist candidate operator functions 
  // of the form
  //   VQ T& operator=(VQ T&, T);
  if (t0->isMemberPointer () || t0->isEnum ()) {
    t = new CTypeAddress (t0->Duplicate ());
    createOperator (db, oper, opname, t, t->Duplicate (), t0->UnqualType ()->Duplicate ());
  }

  // 18. For every triple (L, VQ, R), where L is an arithmetic type, VQ is either 
  // volatile or empty, and R is a promoted arithmetic type, there exist candidate 
  // operator functions of the form
  //   VQ L& operator=(VQ L&, R);
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t = new CTypeAddress (t0->Duplicate ());
    t1 = cvs.arithmeticPromotion (t1);
    createOperator (db, oper, opname, t, t->Duplicate (), t1->Duplicate ()); 
  }
}


// operator /
void CCOverloading::createDivOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  if (! t1) // binary operators must have two operands
    return;

  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   LR   operator/(L, R);
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    t = cvs.usualArithmeticConv (t0, t1);
    createOperator (db, oper, opname, t->Duplicate (), t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator ==, operator !=
void CCOverloading::createEqOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  if (! t1) // binary operators must have two operands
    return;

  // 15. For every pointer or enumeration type T, there exist candidate operator
  // functions of the form
  //   bool operator==(T, T);
  //   bool operator!=(T, T);
  // 16. For every pointer to member type T, there exist candidate operator 
  // functions of the form
  //   bool operator==(T, T);
  //   bool operator!=(T, T);
  if (t0->isMemberPointer () || t0->isPointer () || t0->isEnum ()) {
    createOperator (db, oper, opname, &CTYPE_BOOL, t0->Duplicate (), t0->Duplicate ());
  } 
  if (t1->isMemberPointer () || t1->isPointer () || t1->isEnum ()) {
    createOperator (db, oper, opname, &CTYPE_BOOL, t1->Duplicate (), t1->Duplicate ());
  } 

  // 12. For every pair of promoted arithmetic types L and R, there exist 
  // candidate operator functions of the form (where LR is the result of the 
  // usual arithmetic conversions between types L and R)
  //   bool operator==(L, R);
  //   bool operator!=(L, R);     
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t0 = cvs.arithmeticPromotion (t0);
    t1 = cvs.arithmeticPromotion (t1);
    createOperator (db, oper, opname, &CTYPE_BOOL, t0->Duplicate (), t1->Duplicate ()); 
  }
}


// operator *=, operator /=
void CCOverloading::createMulAssOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;

  if (! t1) // binary operators must have two operands
    return;

  // 18. For every triple (L, VQ, R), where L is an arithmetic type, VQ is either 
  // volatile or empty, and R is a promoted arithmetic type, there exist candidate 
  // operator functions of the form
  //   VQ L& operator*=(VQ L&, R);
  //   VQ L& operator/=(VQ L&, R);
  if (! t0->isConst () && t0->isArithmetic () && t1->isArithmetic ()) {
    t = new CTypeAddress (t0->Duplicate ());
    t1 = cvs.arithmeticPromotion (t1);
    createOperator (db, oper, opname, t, t->Duplicate (), t1->Duplicate ()); 
  }
}


// operator +=, operator -=
void CCOverloading::createAddAssOp (CSemDatabase *db, int oper,
  const char *opname, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeInfo *t;
  
  if (! t1) // binary operators must have two operands
    return;

  if (t0->isConst ())
    return;

  // 21. For every pair (T, VQ), where T is a cv-qualified or cv-unqualified object 
  // type and VQ is either volatile or empty, there exist candidate operator functions 
  // of the form
  //   T*VQ& operator+=(T*VQ&, ptrdiff_t);
  //   T*VQ& operator-=(T*VQ&, ptrdiff_t);
  if (t0->isObject () && t0->isPointer ()) {
    t = t0->UnqualType ()->Duplicate ();
    if (t0->isVolatile ())
      t = new CTypeQualified (t, false, true, false);
    t = new CTypeAddress (t);                  /* ptrdiff_t */
    createOperator (db, oper, opname, t, t->Duplicate (), CTypeInfo::CTYPE_PTRDIFF_T);
  }

  // 18. For every triple (L, VQ, R), where L is an arithmetic type, VQ is either 
  // volatile or empty, and R is a promoted arithmetic type, there exist candidate 
  // operator functions of the form
  //   VQ L& operator+=(VQ L&, R);
  //   VQ L& operator-=(VQ L&, R);
  if (t0->isArithmetic () && t1->isArithmetic ()) {
    t = new CTypeAddress (t0->Duplicate ());
    t1 = cvs.arithmeticPromotion (t1);
    createOperator (db, oper, opname, t, t->Duplicate (), t1->Duplicate ()); 
  }
}


void CCOverloading::createOperator (CSemDatabase *db, int oper, 
  const char *name, CTypeInfo *rtype, CTypeInfo *t0, CTypeInfo *t1) {
  CTypeList *args;
  CFunctionInfo *info;
  
  // add operator function to candidate set only if there is no 
  // other non-template non-member candidate with the same parameter 
  // type list 

  for (unsigned i = Candidates (); i > 0; i--) {
    info = Candidate (i-1)->Function ();
    args = info->TypeInfo ()->ArgTypes ();
    
    // same number of arguments
    if (args->Entries () == (unsigned)((t0?1:0) + (t1?1:0))) 
      // non-template non-member function      
      if (! info->isTemplateInstance () && ! info->isMethod ())
        // same name/operator
        if (info->Name () && strcmp (info->Name (), name) == 0)
          // same parameter type list
          if ((t0 ? *t0 == *args->Entry (0) : true) &&
              (t1 ? *t1 == *args->Entry (1) : true)) {
            if (t0) CTypeInfo::Destroy (t0);
            if (t1) CTypeInfo::Destroy (t1);
            if (rtype) CTypeInfo::Destroy (rtype);
            return; // operator already exists
          }
  }

  // get the built-in operator object from the class the and add it as a
  // candidate
  addCandidate (db->BuiltinOperator (name, oper, rtype, t0, t1));
}


void CCOverloading::collectConvTypes (CRecord *cu, Array<CTypeInfo*> &types) {
  CFunctionInfo *fct;
  CClassInfo *cinfo;
  bool skip;

  // ensure this class is fully instantiated
  if (cu->ClassInfo())
    cu->TypeInfo()->TypeClass()->instantiate(cu->Parent()->Structure());

  for (unsigned i = 0; i < cu->Functions (); i++) {
    fct = cu->Function (i);

    if (fct->isConversion ()) {
      // don't add the same type twice
      skip = false;
      for (unsigned j = types.length (); j > 0; j--) {
        if (*types[j-1] == *fct->ConversionType ()) {
          skip = true;
          break;
        }
      }
      if (! skip)
        types.append (fct->ConversionType ());
    }
  }

  // also consider base classes
  cinfo = cu->ClassInfo ();
  if (cinfo)
    for (unsigned i = 0; i < cinfo->BaseClasses (); i++)
      collectConvTypes (cinfo->BaseClass (i)->Class (), types);
}


} // namespace Puma
