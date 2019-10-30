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

#include "Puma/InstantiationCandidate.h"
#include "Puma/InstantiationParser.h"
#include "Puma/CStructure.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CTree.h"
#include "Puma/CCOverloading.h"
#include "Puma/CConstant.h"
#include "Puma/CCConversions.h"
#include "Puma/CCParser.h"
#include "Puma/TokenStream.h"
#include "Puma/UnitManager.h"
#include "Puma/CSemDatabase.h"
#include "Puma/PreMacroManager.h"
#include "Puma/PreprocessorParser.h"
#include "Puma/CFileInfo.h"
#include "Puma/CSourceInfo.h"
#include "Puma/CCConversions.h"
#include "Puma/CCSemExpr.h"
#include "Puma/CAttributeInfo.h"
#include "Puma/CTypedefInfo.h"
#include "Puma/CMemberAliasInfo.h"
#include "Puma/CArgumentInfo.h"
#include <sstream>
#include <string.h>

namespace Puma {


/*DEBUG*/int TRACE_INSTANCE_CODE = 0;
/*DEBUG*/int TRACE_PARSE_INSTANCE = 0;


#define SEM_ERROR(loc__,mesg__) \
  { \
    if (err) { \
      (*err) << sev_error << loc__->token ()->location () \
             << mesg__ << endMessage; \
    } \
  }


InstantiationCandidate::InstantiationCandidate () {
  obj_info   = 0;
  tpl_info   = 0;
  poi        = 0;
  err        = 0;
  inst_scope = 0;
  instance   = 0;
  parser     = 0;
  detected_delayed_parse_node = false;
}


InstantiationCandidate::~InstantiationCandidate () {
  for (long i = 0; i < darguments.length (); i++)
    if (darguments[i])
      delete darguments[i];
  if (parser)
    delete parser;
}


void InstantiationCandidate::reset () {
  obj_info   = 0;
  tpl_info   = 0;
  poi        = 0;
  err        = 0;
  inst_scope = 0;

  if (parser) {
    delete parser;
    parser = 0;
  }

  for (long i = 0; i < darguments.length (); i++)
    if (darguments[i])
      delete darguments[i];

  arguments.reset ();
  darguments.reset ();
  iarguments.reset ();
  detected_delayed_parse_node = false;
}


void InstantiationCandidate::initialize (ErrorSink *e) {
  err = e;
}


void InstantiationCandidate::initialize (CTree *p, CObjectInfo *o) {
  poi      = p;
  obj_info = o;
}


void InstantiationCandidate::initialize (CTree *p, CObjectInfo *o, CTemplateInfo *t, ErrorSink *e) {
  poi      = p;
  obj_info = o;
  tpl_info = t;
  err      = e ? e : err;
}


void InstantiationCandidate::TemplateInstance (CTemplateInstance *i) {
  instance = i;
  if (instance)
    inst_scope = instance->Object()->Scope()->Structure();
}


int InstantiationCandidate::getPosition (CTemplateParamInfo *param) const {
  for (unsigned i = 0; i < tpl_info->Parameters (); i++) 
    if (*param == *tpl_info->Parameter (i))
      return i;
  return -1;
}


CProject* InstantiationCandidate::getProject() {
  return obj_info->SemDB()->Project();
}


CTemplateParamInfo* InstantiationCandidate::getMatchingParameter (CTemplateParamInfo *param) {
  if (tpl_info && param && param->TemplateInfo () != tpl_info) {
    int pos = param->getPosition ();
    if (pos >= 0 && pos < (int)tpl_info->Parameters ())
      param = tpl_info->Parameter (pos);
    else
      param = 0;
  }
  return param;
}


bool InstantiationCandidate::canBeInstantiated () {
  // template instantiation disabled?
  if (getProject()->config().Option("--pseudo-instances"))
    return false;
  // point of instantiation depends on template parameter?
  CTree* name = poi;
  if (name->NodeName () == CT_QualName::NodeId () ||
      name->NodeName () == CT_RootQualName::NodeId ())
    name = ((CT_QualName*)name)->Name ();
  if (name->NodeName () == CT_TemplateName::NodeId ())
    if (CCSemExpr::isDependent(((CT_TemplateName*)name)->Arguments ()))
      return false;
  // everything OK
  return true;
}


// ��14.8.2 deduce template arguments (if necessary)
bool InstantiationCandidate::deduceArguments (bool real_inst) {
  // check if template really can be instantiated
  if (real_inst && ! canBeInstantiated())
    real_inst = false;

  CTree* name = poi;
  if (name->NodeName () == CT_QualName::NodeId () ||
      name->NodeName () == CT_RootQualName::NodeId ())
    name = ((CT_QualName*)name)->Name ();

  // template arguments
  CT_TemplateArgList *args;
  unsigned numargs;
  if (name->NodeName () == CT_TemplateName::NodeId ()) {
    args = ((CT_TemplateName*)name)->Arguments ();
    numargs = args->Entries ();
  } else {
    args = 0;
    numargs = 0;
  }

  // get template parameter list information
  if (obj_info->TemplateParamInfo ())
    tpl_info = obj_info->TemplateParamInfo ()->TemplateTemplate ();
  else
    tpl_info = obj_info->Template ();
  if (! tpl_info) {
    if (real_inst) {
      SEM_ERROR (poi, "fatal error: missing template parameter information");
      return false;
    }
    return true;
  }

  // check number of template arguments for function templates
  if (obj_info->FunctionInfo () && numargs > tpl_info->Parameters ()) {
    if (real_inst) {
      SEM_ERROR (poi, "wrong number of template arguments ("
        << numargs << ", should be " << tpl_info->Parameters () << ")");
      return false;
    }
    return true;
  }

  // calculate index of first parameter for which no direct template
  // argument or default template argument is given
  unsigned pos = 0;
  for (; pos < tpl_info->Parameters(); pos++) {
    CTemplateParamInfo *param = tpl_info->Parameter(pos);
    if (pos >= numargs) {
      // no direct template argument for the current parameter,
      // check default template arguments for non-function templates
      if (! obj_info->FunctionInfo()) {
        // not a function template, check default argument
        if (! param->DefaultArgument()) {
          // no default argument
          if (real_inst) {
            SEM_ERROR(poi, "missing default argument for parameter " << pos+1);
            return false;
          }
          return true;
        }
      } else {
        // function template parameters do not have default arguments,
        // ommited arguments must be deduced from the function call
        // argument list
        break;
      }
    }
  }

  // deduce the direct and default arguments
  if (! (real_inst ? parseDirectArguments(pos, numargs, args) :
                     matchDirectArguments(pos, numargs, args))) {
    return false;
  }

  if (pos < tpl_info->Parameters()) {
    // deduce remaining arguments from function call
    return deduceArgumentsFromFctCall(pos);
  }
  return true;
}


bool InstantiationCandidate::matchDirectArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args) {
  // match direct and default template arguments
  for (unsigned i = 0; i < pos; i++) {
    CTemplateParamInfo *param = tpl_info->Parameter (i);
    bool is_default_arg = (i >= numargs);
    CObjectInfo* oinfo = 0;
    CTree *arg;

    if (is_default_arg) {
      // default template argument
      arg = param->DefaultArgument ();
    } else {
      // direct template argument
      arg = args->Entry (i);
    }

    // template template parameter: expect name of class base template
    if (param->isTemplate ()) {
      if (arg->SemObject () && (oinfo = arg->SemObject ()->Object ()) && oinfo->TypeInfo ())
        darguments.append (new DeducedArgument (param,
          oinfo->TypeInfo ()->VirtualType (), arg, is_default_arg, !is_default_arg));
    }
    // template type parameter: expect type-id
    else if (param->isTypeParam ()) {
      if (arg->SemObject () && (oinfo = arg->SemObject ()->Object ()) && oinfo->TypeInfo ())
        darguments.append (new DeducedArgument (param,
          oinfo->TypeInfo ()->VirtualType (), arg, is_default_arg, !is_default_arg));
    }
    // template non-type parameter: expect constant expression
    else {
      if (arg->Type () && arg->Type ()->TypeTemplateParam () &&
          arg->Type ()->TypeTemplateParam ()->isNonType ()) {
        // template non-type parameter as argument
        darguments.append (new DeducedArgument (param,
          arg->Type (), arg, is_default_arg, !is_default_arg));
      }
      else if (arg->Value () && arg->Value ()->Constant ()) {
        darguments.append (new DeducedArgument (param,
          arg->Value ()->Constant (), arg, is_default_arg, !is_default_arg));
      }
    }
  }
  return true;
}


// ��14.8.2 deduce template arguments from function call
bool InstantiationCandidate::deduceArgumentsFromFctCall (unsigned skip, bool exact_match, bool partial_ordering) {
  DeducedArgumentList dargs (tpl_info->Parameters ());
  CTypeInfo *atype, *ptype;
  CFunctionInfo *finfo;
  CTypeList *fargs;
  unsigned numargs;
  bool succeeded;
  CTree *arg;
  int ret;

  succeeded = true;
  finfo = obj_info->FunctionInfo ();
  fargs = finfo->TypeInfo ()->ArgTypes ();
  numargs = Arguments ();

  // compare number of function arguments and parameters
  if (fargs->Entries () > numargs) {
    if (! partial_ordering)
      numargs = fargs->Entries ();
  }
  else if (fargs->Entries () < numargs) {
    if (finfo->hasEllipsis ()) {
      numargs = fargs->Entries () - 1;
    }
    else {
      if (! partial_ordering) {
        SEM_ERROR (poi, "too many arguments in call to function `"
          << finfo->Name () << "'");
        return false;
      }
      numargs = fargs->Entries ();
    }
  }

  // prepare temporary deduced argument container
  for (unsigned i = 0; i < tpl_info->Parameters (); i++)
    dargs.append (0);

  // iterate and analyse arguments
  for (unsigned i = 0; i < numargs && succeeded; i++) {
    // consider default arguments for function parameters
    if (i >= Arguments ()) {
      if (! finfo->hasDefaultArgument (i)) {
        if (! partial_ordering) {
          SEM_ERROR (poi, "too few arguments in call to function `"
            << finfo->Name () << "'");
          return false;
        }
        break; // ignore default arguments when doing partial ordering
      }
      else {
        arg = finfo->Argument(i)->Tree ();
        // arg = finfo->DefaultArgument (i);
      }
    } else
      arg = Argument (i);

    atype = arg->Type () ? arg->Type () : arg->SemObject ()->Object ()->TypeInfo ();
    ptype = fargs->Entry (i);

    if (atype->TypeAddress ()) {
      atype = atype->NonReferenceType ();
    }
    if (ptype->TypeAddress ()) {
      ptype = ptype->NonReferenceType ();
    } else {
      // ��14.3.2.5 implicit argument type conversions
      atype = atype->VirtualType ();
      ptype = ptype->VirtualType ();
      // array-to-pointer and function-to-pointer conversions
      if (atype->isArray () || atype->isFunction ()) {
        atype = atype->isArray () ? atype->VirtualType ()->BaseType () : atype;
        // TODO: Delete this type if done and not used!
        atype = new CTypePointer (atype->Duplicate ());
      }
    }

    ret = deduceArguments (ptype, atype, dargs, exact_match, partial_ordering);
    if (ret == 1 || ret == 3) {
      succeeded = false;
    }
  }

  for (unsigned i = skip; i < (unsigned)dargs.length () && succeeded; i++) {
    if (! dargs[i]) {
      SEM_ERROR (poi, "could not deduce argument for parameter " << i+1);
      succeeded = false;
    } else
      darguments.append (dargs[i]);
  }

  return succeeded;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceArguments (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  int ret;

  if (atype->TypeTemplateParam ()) {
    for (unsigned i = 0; i < (unsigned)dargs.length (); i++) {
      if (dargs[i] && *dargs[i]->TemplateParam ()->TypeInfo () == *atype) {
        atype = dargs[i]->Type ();
      }
    }
  }

  // template parameter
  if (ftype->TypeTemplateParam ()) {
    return deduceTemplateParam (ftype, atype, dargs, exact_match, partial_ordering);
  }

  // deduce from qualified type
  if (ftype->TypeQualified ()) {
    return deduceFromQualifiedType (ftype, atype, dargs, exact_match, partial_ordering);
  }

  // types must be equal to deduce template arguments
  if (ftype->Id () != atype->Id ()) {
    return exact_match ? 2 : (partial_ordering && atype->TypeTemplateParam ()) ? 3 : ftype->isDependent () ? 1 : 0;
  }

  // nothing to deduce
  if (ftype->TypeEmpty () || ftype->TypePrimitive ()) {
    return 0;
  }

  // deduce from pointer to class member
  if (ftype->TypeMemberPointer ()) {
    return deduceFromMemberPointer (ftype, atype, dargs, exact_match, partial_ordering);

  // deduce from class or union type
  } else if (ftype->TypeRecord ()) {
    return deduceFromRecord (ftype, atype, dargs, exact_match, partial_ordering);

  // deduce from array type
  } else if (ftype->TypeArray ()) {
    if ((ret = deduceFromArray (ftype, atype, dargs, exact_match, partial_ordering)) != 0) {
      return ret;
    }

  // deduce from function type
  } else if (ftype->TypeFunction ()) {
    if ((ret = deduceFromFunction (ftype, atype, dargs, exact_match, partial_ordering)) != 0) {
      return ret;
    }
  }

  // deduce from the base types, types must match exactly
  return deduceArguments (ftype->BaseType (), atype->BaseType (), dargs, exact_match, partial_ordering);
}


CTypeQualified* InstantiationCandidate::getQualifiers (CTypeInfo* type) {
  if (type->UnqualType ()->TypeArray ()) {
    do {
      type = type->UnqualType ()->BaseType ();
    } while (type->UnqualType ()->TypeArray ());
  }
  return type->TypeQualified ();
}


CTypeInfo* InstantiationCandidate::setQualification (CTypeInfo* type, bool is_const, bool is_volatile, bool is_restrict) {
  CTypeQualified* qual = type->TypeQualified ();
  CTypeInfo* parent = 0;
  if (type->UnqualType ()->TypeArray ()) {
    parent = type->UnqualType ();
    while (parent->BaseType ()->UnqualType ()->TypeArray ())
      parent = parent->BaseType ()->UnqualType ();
    qual = parent->BaseType ()->TypeQualified ();
  }
  if (!is_const && !is_volatile && !is_restrict) {
    // remove all qualifiers
    type = type->UnqualType ();
    if (parent && qual)
      parent->BaseType (qual->BaseType ());
    if (qual)
      delete qual;
  }
  else if (! qual) {
    // create qualifiers
    qual = new CTypeQualified (parent ? parent->BaseType () : type, is_const, is_volatile, is_restrict);
    if (parent)
      parent->BaseType (qual);
    else
      type = qual;
  }
  else {
    // set qualifiers
    qual->isConst (is_const);
    qual->isVolatile (is_volatile);
    qual->isRestrict (is_restrict);
  }
  return type;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceFromQualifiedType (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CTypeQualified* fqt = ftype->TypeQualified ();
  CTypeQualified* aqt = atype->TypeQualified ();

  bool ftype_is_tpl_param = ftype->UnqualType ()->TypeTemplateParam ();

  // get qualifiers of array element type, if an array
  if (ftype_is_tpl_param)
    aqt = getQualifiers (atype);

  // exact match required: qualifiers must be the same
  if (exact_match) {
    if (! aqt || fqt->isConst () != aqt->isConst () || fqt->isVolatile () != aqt->isVolatile ()) {
      return 2;
    }

    atype = atype->UnqualType ();
    if (atype->TypeArray ()) {
      atype = setQualification (atype->Duplicate (), false, false, false);
      int ret = deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
      CTypeInfo::Destroy (atype);
      return ret;
    } else
      return deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
  }

  // ordinary parameter type
  if (! ftype_is_tpl_param) {
    // only matches if equal or more qualified than the argument type
    if (! CCConversions::equalOrMoreQualified (ftype, atype)) {
      return 0;
    }

    // deduce from the unqualified versions of the types
    return deduceArguments (ftype->UnqualType (), atype->UnqualType (), dargs, exact_match, partial_ordering);
  }
  // more specialized
  else if (partial_ordering && atype->UnqualType ()->TypeTemplateParam () &&
           ! CCConversions::equalOrMoreQualified (atype, ftype)) {
    return 3;
  }

  // qualified template parameter type, remove all equal qualifiers
  // from the argument type

  bool only_arg_has_const = aqt && aqt->isConst () && ! fqt->isConst ();
  bool only_arg_has_volatile = aqt && aqt->isVolatile () && ! fqt->isVolatile ();

  // argument type is less or equal unqualified than parameter type
  // remove all qualifiers from argument type
  if (! only_arg_has_const && ! only_arg_has_volatile) {
    atype = atype->UnqualType ();
    if (atype->TypeArray ()) {
      atype = setQualification (atype->Duplicate (), false, false, false);
      int ret = deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
      CTypeInfo::Destroy (atype);
      return ret;
    } else
      return deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
  }

  // argument type is not full qualified but more qualified than parameter type
  // remove no qualifier from argument type
  else if (! (aqt->isConst () && aqt->isVolatile ())) {
    return deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
  }

  // argument type is full qualified and more qualified than parameter type
  // remove the qualifier from argument type that also the parameter type has
  else {
    atype = setQualification (atype->Duplicate (), only_arg_has_const, only_arg_has_volatile, aqt->isRestrict ());
    int ret = deduceArguments (ftype->UnqualType (), atype, dargs, exact_match, partial_ordering);
    CTypeInfo::Destroy (atype);
    return ret;
  }
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceFromFunction (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CTypeFunction *fft = ftype->TypeFunction ();
  CTypeFunction *aft = atype->TypeFunction ();

  // number of function parameters must be equal
  if (fft && aft && fft->ArgTypes ()->Entries () == aft->ArgTypes ()->Entries ()) {
    CTypeList *ftl = fft->ArgTypes ();
    CTypeList *atl = aft->ArgTypes ();

    bool matching_args = true;
    bool ambiguous_type = false;
    bool more_specialized = false;
    DeducedArgumentList curr_dargs;

    // prepare temporary deduced argument container
    for (unsigned i = 0; i < tpl_info->Parameters (); i++) {
      curr_dargs.append (0);
    }

    // deduce from the function parameters
    for (unsigned i = 0; i < ftl->Entries () && matching_args && ! ambiguous_type; i++) {
      switch (deduceArguments (ftl->Entry (i), atl->Entry (i), dargs, true, partial_ordering)) {
        case 3: more_specialized = true; break;
        case 2: matching_args = false; break;
        case 1: ambiguous_type = true; break;
        default: break;
      }
    }

    // join the deduced template arguments if possible
    if (! joinDeducedArguments (dargs, curr_dargs, matching_args, ambiguous_type)) {
      return matching_args && ambiguous_type ? 1 :
             exact_match && ! matching_args ? 2 :
             more_specialized ? 3 : 0;
    }

  // different number of function parameters
  } else if (exact_match) {
    return 2;
  }

  return partial_ordering && atype->TypeTemplateParam () ? 3 : 0;
}


bool InstantiationCandidate::joinDeducedArguments (DeducedArgumentList &dargs, DeducedArgumentList &curr_dargs, bool &matching_args, bool &ambiguous_type) {
  // check if the current deduced template arguments are valid
  if (! matching_args || ambiguous_type) {
    // discard the deduced template arguments
    for (long i = 0; i < curr_dargs.length (); i++) {
      DeducedArgument *darg = curr_dargs.fetch (i);
      if (darg) delete darg;
    }
  }
  // if not ambiguous, add the deduced arguments to dargs
  if (matching_args) {
    // join the deduced template arguments
    if (! ambiguous_type) {
      for (long i = 0; i < curr_dargs.length (); i++) {
        DeducedArgument *darg = curr_dargs.fetch (i);
        if (darg && ! setDeducedArgument (dargs, i, darg)) {
          ambiguous_type = true;
        }
      }
    }
  }
  return matching_args && ! ambiguous_type;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceFromArray (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CTypeArray *array = ftype->TypeArray ();

  // array size depends on a template parameter
  if (array->DepDim ()) {
    CTemplateParamInfo *param = array->DepDim ()->TemplateParamInfo ();

    if (param) {
      int pos = getPosition (param);

      // non-type template parameter
      if (pos != -1 && ! param->isTypeParam ()) {
        array = atype->TypeArray ();

        // array has dimension
        if (array->Dimension ()) {
          if (! setDeducedArgument (dargs, pos, new DeducedArgument (param, array->Dimension ()))) {
            return 1;
          }

        // match non-type template parameter for partial ordering of function templates
        } else if (array->DepDim ()) {
          CTemplateParamInfo *aparam = array->DepDim ()->TemplateParamInfo ();
          if (aparam &&
              ! aparam->isTypeParam () &&
              ! setDeducedArgument (dargs, pos, new DeducedArgument (param, atype))) {
            return 1;
          }
        }
      }
    }

  // ordinary types must be an exact match
  } else if (exact_match && (*ftype != *atype)) {
    return 2;
  }

  return partial_ordering && atype->TypeTemplateParam () ? 3 : 0;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceTemplateParam (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CTemplateParamInfo *param = ftype->TypeTemplateParam ()->TemplateParamInfo ();
  int ret, pos = getPosition (param);

  if (pos != -1) {
    // template template parameter
    if (param->isTemplate ()) {
      CObjectInfo *ainfo = atype->TypeRecord () ? (CObjectInfo*)atype->TypeRecord ()->Record () :
                           atype->TypeTemplateParam () ? (CObjectInfo*)atype->TypeTemplateParam ()->TemplateParamInfo () : 0;

      // template parameter list must match
      if (! matchingTemplateParameters (param, ainfo)) {
        return exact_match ? 2 : 0;
      }

      // get the template instance info of the argument, if it is an template instance
      CTemplateInstance *ti = ainfo->Record () ? ainfo->TemplateInstance () : ainfo->TemplateParamInfo ()->TemplateInstance ();  
      CTemplateInfo *tpl = getTemplateInfo (ainfo);

      // match template instance
      if (param->TemplateInstance ()) {
        if (! ti) {
          // not a template instance
          return exact_match ? 2 : 0;
        }

        // deduce from the argument list
        ret = deduceArgumentsFromTemplateArgList (param->TemplateInstance (), ti, dargs, partial_ordering);
        if (ret != 0) {
          return ret;
        }

        // matches the template itself
        if (! setDeducedArgument (dargs, pos, new DeducedArgument (param, tpl->ObjectInfo ()->TypeInfo ()))) {
          return 1;
        }

      // template template parameter does not match template instances
      } else if (ti) {
        return exact_match ? 2 : 0;

      // match template itself (template name)
      } else {
        if (! setDeducedArgument (dargs, pos, new DeducedArgument (param, tpl->ObjectInfo ()->TypeInfo ()))) {
          return 1;
        }
      }

    // type template parameter, matches any type
    } else if (param->isTypeParam ()) {
      if (! setDeducedArgument (dargs, pos, new DeducedArgument (param, atype))) {
        return 1;
      }

    // non-type template parameter, matches any value or non-type template parameter,
    // for partial ordering of function templates
    } else if (! param->isTypeParam ()) {
      if (atype->TypeTemplateParam ()) {
        CTemplateParamInfo *aparam = atype->TypeTemplateParam ()->TemplateParamInfo ();
        if (aparam &&
            ! aparam->isTypeParam () &&
            ! setDeducedArgument (dargs, pos, new DeducedArgument (param, atype))) {
          return 1;
        }
      }
    }
  }

  return 0;
}


bool InstantiationCandidate::matchingTemplateParameters (CTemplateParamInfo *param, CObjectInfo *ainfo) {
  if (! ainfo)
    return false;

  CTemplateInfo *atpl = getTemplateInfo (ainfo);
  CTemplateInfo *ftpl = param->TemplateTemplate ();

  if (! ftpl && ! atpl)
    return true;
  if (! ftpl || ! atpl || ftpl->Parameters () != atpl->Parameters ())
    return false;

  for (unsigned i = 0; i < ftpl->Parameters (); i++) {
    CTemplateParamInfo *fparam = ftpl->Parameter (i);
    CTemplateParamInfo *aparam = atpl->Parameter (i);

    if (! fparam->match (*aparam))
      return false;
  }

  return true;
}


CTemplateInfo *InstantiationCandidate::getTemplateInfo (CObjectInfo *info) {
  CTemplateInfo *tpl = 0;
  if (info->TemplateInstance ())
    tpl = info->TemplateInstance ()->Template ();
  else if (info->Record ())
    tpl = info->Record ()->TemplateInfo ();
  else if (info->TemplateParamInfo ())
    tpl = info->TemplateParamInfo ()->TemplateTemplate ();
  return tpl;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceFromMemberPointer (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CTemplateParamInfo *param = ftype->TypeMemberPointer ()->TemplateParam ();

  // member pointer of form T::member where T is a type template parameter
  if (param) {
    int pos = getPosition (param);
    if (pos != -1 && param->isTypeParam ()) {

      CRecord *record = atype->TypeMemberPointer ()->Record ();

      // argument type is class or union, can be deduced
      if (record) {
        if (! setDeducedArgument (dargs, pos, new DeducedArgument (param, record->TypeInfo ()))) {
          return 1;
        }

      // argument member pointer type contains template parameter, handled as unique type for
      // partial ordering of function templates
      } else {
        CTemplateParamInfo *tp = atype->TypeMemberPointer ()->TemplateParam ();
        if (tp && tp->isTypeParam () && ! setDeducedArgument (dargs, pos, new DeducedArgument (param, atype))) {
          return 1;
        }
      }
    }

  // ordinary types must match, if not the top level types
  } else if (exact_match && (*ftype != *atype)) {
    return 2;
  }

  return partial_ordering && atype->TypeTemplateParam () ? 3 : 0;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceFromRecord (CTypeInfo *ftype, CTypeInfo *atype, DeducedArgumentList &dargs, bool exact_match, bool partial_ordering) {
  CRecord *finfo = ftype->TypeRecord ()->Record ();
  CRecord *ainfo = atype->TypeRecord ()->Record ();

  if (finfo && ainfo) {
    // types must have same record type, i.e. either class or union
    if ((finfo->ClassInfo () && ! ainfo->ClassInfo ()) ||
        (finfo->UnionInfo () && ! ainfo->UnionInfo ())) {
      return exact_match ? 2 : partial_ordering && atype->TypeTemplateParam () ? 3 : 0;
    }

    // parameter type is template instance
    if (finfo->isTemplateInstance ()) {
      // deduce template arguments from the type arguments of the template instances,
      // the template used to instantiate finfo must be equal to the template used
      // to instantiate ainfo or one of its base classes

      CTemplateInstance *fi = finfo->TemplateInstance ();
      Array<CTemplateInstance*> instances;
      getTemplateInstances (ainfo, instances); // search the class hierarchy of ainfo

      bool at_least_one_match = false;
      for (long j = 0; j < instances.length (); j++) {
        CTemplateInstance *ai = instances.fetch (j);

        // same template and same number of template arguments
        CTemplateInfo *ft = fi->Template ()->BaseTemplate () ? fi->Template ()->BaseTemplate () : fi->Template ();
        CTemplateInfo *at = ai->Template ()->BaseTemplate () ? ai->Template ()->BaseTemplate () : ai->Template ();
        if (*ft->ObjectInfo () != *at->ObjectInfo () || fi->InstantiationArgs () != ai->InstantiationArgs ()) {
          continue;
        }

        // deduce from the template arguments
        int ret = deduceArgumentsFromTemplateArgList (fi, ai, dargs, partial_ordering);
        if (ret == 1) {
          return 1;
        }
        at_least_one_match = at_least_one_match || (ret == 0);
      }

      // require exact match
      if (! at_least_one_match) {
        return 2;
      }

    // ordinary types must match, if not the top level types
    } else if (exact_match && (*finfo != *ainfo)) {
      return 2;
    }
  }

  return partial_ordering && atype->TypeTemplateParam () ? 3 : 0;
}


// return 0 if all ok, 1 if ambiguous type for parameter, 2 if types don't match, 3 if more specialized
int InstantiationCandidate::deduceArgumentsFromTemplateArgList (CTemplateInstance *fi, CTemplateInstance *ai, DeducedArgumentList &dargs, bool partial_ordering) {
  bool matching_args = true;
  bool ambiguous_type = false;
  bool more_specialized = false;
  DeducedArgumentList curr_dargs;

  // number of template arguments must match
  if (fi->InstantiationArgs () != ai->InstantiationArgs ()) {
    return 2;
  }

  // prepare temporary deduced argument container
  for (unsigned i = 0; i < tpl_info->Parameters (); i++) {
    curr_dargs.append (0);
  }

  // now try to deduce from the template arguments, must match exactly
  // for a successful deduction
  for (unsigned i = 0; i < fi->InstantiationArgs () && matching_args && ! ambiguous_type; i++) {
    CTypeInfo *fdtype = fi->InstantiationArg (i)->Type ();
    CTypeInfo *adtype = ai->InstantiationArg (i)->Type ();
    CConstant *fvalue = fi->InstantiationArg (i)->Value ();
    CConstant *avalue = ai->InstantiationArg (i)->Value ();

    if (fdtype) {
      // type template arguments
      if (adtype) {
        switch (deduceArguments (fdtype, adtype, curr_dargs, true, partial_ordering)) {
          case 3: more_specialized = true; break;
          case 2: matching_args = false; break;
          case 1: ambiguous_type = true; break;
          default: break;
        }

      // non-type template arguments
      } else if (avalue &&
                 fdtype->TypeTemplateParam () &&
                 ! fdtype->TypeTemplateParam ()->isTypeParam ()) {
        CTemplateParamInfo *param = fdtype->TypeTemplateParam ()->TemplateParamInfo ();
        if (param) {
          int pos = getPosition (param);
          if (pos != -1 && ! setDeducedArgument (curr_dargs, pos, new DeducedArgument (param, avalue))) {
            ambiguous_type = true;
          }
        }

      } else {
        matching_args = false;
      }

    // non-type template arguments
    } else if (fvalue && (! avalue || (*fvalue != *avalue))) {
      matching_args = false;
    }
  }

  // check if deduced template arguments are valid
  if (! matching_args || ambiguous_type) {
    // discard the deduced template arguments
    for (long i = 0; i < curr_dargs.length (); i++) {
      DeducedArgument *darg = curr_dargs.fetch (i);
      if (darg) delete darg;
    }
  }
  // if not ambiguous, add the deduced arguments to dargs
  if (matching_args) {
    // add the deduced template arguments
    if (! ambiguous_type) {
      for (long i = 0; i < curr_dargs.length (); i++) {
        DeducedArgument *darg = curr_dargs.fetch (i);
        if (darg && ! setDeducedArgument (dargs, i, darg)) {
          ambiguous_type = true;
        }
      }
    }
    return ambiguous_type ? 1 : more_specialized ? 3 : 0;
  }
  return 2;
}


void InstantiationCandidate::getTemplateInstances(CRecord* c, Array<CTemplateInstance*>& instances) {
  if (c->isTemplateInstance ()) {
    instances.append (c->TemplateInstance ());
  }
  CClassInfo *cl = c->ClassInfo ();
  if (cl) {
    for (unsigned i = 0; i < cl->BaseClasses (); i++) {
      getTemplateInstances (cl->BaseClass (i)->Class (), instances);
    }
  }
}


// ��14.5.4.1.2 try to match a partial specialization
// against a given template argument list
bool InstantiationCandidate::match (InstantiationCandidate& other) {
  CT_TemplateArgList *args;
  CTemplateInfo *base;
  CTree *arg, *parg, *name;
  unsigned numargs;

  // template argument list
  name = poi;
  if (name->NodeName () == CT_QualName::NodeId () ||
      name->NodeName () == CT_RootQualName::NodeId ())
    name = ((CT_QualName*)name)->Name ();
  if (name->NodeName () == CT_TemplateName::NodeId ()) {
    args = ((CT_TemplateName*)name)->Arguments ();
    numargs = args->Entries ();
  } else {
    args = 0;
    numargs = 0;
  }

  // prepare deduced argument container
  darguments.reset ();
  for (unsigned i = 0; i < tpl_info->Parameters (); i++)
    darguments.append (0);

  // compare argument lists
  base = tpl_info->BaseTemplate ();
  for (unsigned i = 0; i < Arguments (); i++) {
    // current partial specialization argument
    parg = Argument (i);

    // more partial specialization arguments than arguments
    // given at point of instantiation => consider default arguments
    if (i >= numargs) {
      if (base && base->Parameters () > i && base->Parameter (i)->DefaultArgument ()) {
        if (other.DeducedArgs () > i && other.DeducedArg (i)->TemplateArg ())
          arg = other.DeducedArg (i)->TemplateArg ();
        else
          arg = base->Parameter (i)->DefaultArgument ();
      } else {
        // oops, no default argument? this is an error
        return false;
      }
    } else
      arg = args->Entry (i);

    // try to match the current argument, must be an exact match!
    if (! matchArgument (parg, arg))
      return false;
  }

  // check if all template parameters could be deduced while matching
  for (unsigned i = 0; i < DeducedArgs (); i++)
    if (! DeducedArg (i))
      return false;

  return true;
}


bool InstantiationCandidate::sameType (int pos, CTypeInfo *type) {
  DeducedArgument *darg = darguments[pos];
  if (darg && darg->Value ())
    return false; // not a type
  CTypeInfo *otype = darg ? darg->Type () : 0;
  if (! type)
    return false;
  if (otype) {
    if (otype->TypeTemplateParam () && type->TypeTemplateParam ()) {
      CTemplateParamInfo *p1 = otype->TypeTemplateParam ()->TemplateParamInfo ();
      CTemplateParamInfo *p2 = type->TypeTemplateParam ()->TemplateParamInfo ();
      if (! p1 || ! p2 || *p1 != *p2)
        return false;
    } else if (*otype != *type)
      return false;
  }
  return true;
}


bool InstantiationCandidate::sameValue (int pos, CConstant *value) {
  DeducedArgument *darg = darguments[pos];
  if (darg && darg->Type ())
    return false; // not a value
  CConstant *ovalue = darg ? darg->Value () : 0;
  if (! value || (ovalue && (*ovalue != *value)))
    return false;
  return true;
}


bool InstantiationCandidate::matchArgument (CTree *pexpr, CTree *aexpr) {
  CConstant *value;

  // match against value, must be equal
  if (pexpr->Value () && pexpr->Value ()->Constant ()) {
    if (aexpr->Value () && (value = aexpr->Value ()->Constant ()) &&
        (*value == *pexpr->Value ()->Constant ()))
      return true;
    return false;
  }

  CTypeInfo *ptype = 0, *atype = 0;
  if (pexpr->NodeName () == CT_NamedType::NodeId () &&
      pexpr->SemObject () && pexpr->SemObject ()->Object ())
    ptype = pexpr->SemObject ()->Object ()->TypeInfo ();
  if (aexpr->NodeName () == CT_NamedType::NodeId () &&
      aexpr->SemObject () && aexpr->SemObject ()->Object ())
    atype = aexpr->SemObject ()->Object ()->TypeInfo ();
  if (! ptype)
    ptype = pexpr->Type ();
  if (! atype)
    atype = aexpr->Type ();
  if (! ptype || ! atype)
    return false;

  return matchArgument (ptype, atype, aexpr);
}


bool InstantiationCandidate::matchArgument (CTypeInfo *ptype, CTypeInfo *atype, CTree *aexpr) {
  CTemplateInstance *pti, *ati;
  CTemplateInfo *ptinfo, *atinfo;
  CObjectInfo *pinfo, *ainfo;
  CTemplateParamInfo *param, *param2;
  CTypeList *ptl, *atl;
  CConstant *value, *value2;
  CTypeInfo *type, *type2;
  CRecord *prec, *arec;
  long pdim, adim;
  int pos;

  // match against template parameter
  if (ptype->TypeTemplateParam ()) {
    param = ptype->TypeTemplateParam ()->TemplateParamInfo ();
    pos = getPosition (param);
    if (pos != -1) {
      // template template parameter
      if (param->isTemplate ()) {
        if (atype->TypeTemplateParam ()) {
          if (sameType (pos, atype)) {
            darguments[pos] = new DeducedArgument (param, atype);
            return true;
          }
        } else if (atype->TypeRecord ()) {
          ainfo = atype->TypeRecord ()->Record ();
          type = ainfo->TypeInfo ();
          if (ainfo->isTemplateInstance ()) {
            type = ainfo->TemplateInstance ()->Template ()->ObjectInfo ()->TypeInfo ();
          } 
          // template parameter must always deduce to the same type
          if (sameType (pos, type)) {
            darguments[pos] = new DeducedArgument (param, type);
            return true;
          }
        }
      // type template parameter
      } else if (param->isTypeParam ()) {
        if (sameType (pos, atype)) {
          darguments[pos] = new DeducedArgument (param, atype);
          return true;
        }
      // non-type template parameter
      } else if (atype->TypeTemplateParam ()) {
        if (atype->TypeTemplateParam ()->isNonTypeParam ()) {
          if (sameType (pos, atype)) {
            darguments[pos] = new DeducedArgument (param, atype);
            return true;
          }
        }
      // non-type template parameter
      } else if (aexpr) {
        if (aexpr->Value () && (value = aexpr->Value ()->Constant ()) &&
            sameValue (pos, value)) {
          darguments[pos] = new DeducedArgument (param, value);
          return true;
        }
      }
    } 
    return false;
  } 
  
  // non-template parameter arguments and non-value arguments 
  // must be equal in type
  if (ptype->Id () != atype->Id ())
    return false;

  // match against primitive type, no further analyses needed
  if (ptype->TypeEmpty () || ptype->TypePrimitive ())
    return true;
  
  // match against class or union
  if (ptype->TypeRecord ()) {
    pinfo = ptype->TypeRecord ()->Record ();
    ainfo = atype->TypeRecord ()->Record ();
    if (pinfo && ainfo) {
      if (pinfo->isTemplateInstance () && ainfo->isTemplateInstance ()) {
        pti = pinfo->TemplateInstance (); 
        ati = ainfo->TemplateInstance (); 
        if (pti && ati && pti->DeducedArgs () == ati->DeducedArgs ()) {
          ptinfo = pti->Template ();
          atinfo = ati->Template ();
          if (! ptinfo || ! atinfo)
            return false;
          ptinfo = ptinfo->BaseTemplate () ? ptinfo->BaseTemplate () : ptinfo;
          atinfo = atinfo->BaseTemplate () ? atinfo->BaseTemplate () : atinfo;
          if (*ptinfo != *atinfo)
            return false;
          for (unsigned i = 0; i < pti->DeducedArgs (); i++) {
            type = pti->DeducedArg (i)->Type ();
            type2 = ati->DeducedArg (i)->Type ();
            value = pti->DeducedArg (i)->Value ();
            value2 = ati->DeducedArg (i)->Value ();
            if (type) {
              if (type->TypeTemplateParam () &&
                  type->TypeTemplateParam ()->isNonTypeParam ()) {
                param = type->TypeTemplateParam ()->TemplateParamInfo ();
                if (! param || (pos = getPosition (param)) == -1)
                  return false;
                if (type2 && type2->TypeTemplateParam ()) {
                  if (! type2->TypeTemplateParam ()->isNonTypeParam () ||
                      ! sameType (pos, type2))
                    return false;
                  darguments[pos] = new DeducedArgument (param, type2);
                  continue;
                } else if (! value2 || ! sameValue (pos, value2)) 
                  return false;
                darguments[pos] = new DeducedArgument (param, value2);
              } else if (! type2 || ! matchArgument (type, type2, 0))
                return false;
            } else if (! value || ! value2 || *value != *value2)
              return false;
          }
          return true;
        }
      } else if (*pinfo == *ainfo)
        return true;
    } 
    return false;
  // match against pointer to class member
  } else if (ptype->TypeMemberPointer ()) {
    prec = ptype->TypeMemberPointer ()->Record ();
    arec = atype->TypeMemberPointer ()->Record ();
    param = ptype->TypeMemberPointer ()->TemplateParam ();
    if (param) {
      if (! arec)
        return false;
      type = arec->TypeInfo ();
      pos = getPosition (param);
      if (pos == -1 || ! param->isTypeParam () || ! sameType (pos, type)) 
        return false;
      darguments[pos] = new DeducedArgument (param, type);
      return true;
    }
    if ((bool)prec != (bool)arec || *prec != *arec)
      return false;
  // match against array
  } else if (ptype->TypeArray ()) {
    pdim = ptype->TypeArray ()->Dimension ();
    adim = atype->TypeArray ()->Dimension ();
    if (ptype->TypeArray ()->DepDim ()) {
      CConstant dim ((LONG_LONG)adim, &CTYPE_LONG);
      param = ptype->TypeArray ()->DepDim ()->TemplateParamInfo ();
      if (! param || param->isTypeParam () || 
          (pos = getPosition (param)) == -1)
        return false;
      if (atype->TypeArray ()->DepDim ()) {
        param2 = atype->TypeArray ()->DepDim ()->TemplateParamInfo ();
        if (! param2 || param2->isTypeParam () || 
            ! sameType (pos, param2->TypeInfo ()))
          return false;
        darguments[pos] = new DeducedArgument (param, param2->TypeInfo ());
      } else if (sameValue (pos, &dim)) {
        darguments[pos] = new DeducedArgument (param, adim);
      } else 
        return false;
    } else if (pdim != adim)
      return false;
  // match against function
  } else if (ptype->TypeFunction ()) {
    ptl = ptype->TypeFunction ()->ArgTypes ();
    atl = atype->TypeFunction ()->ArgTypes ();
    if (ptl->Entries () != atl->Entries ())
      return false;
    for (unsigned i = 0; i < ptl->Entries (); i++)
      if (! matchArgument (ptl->Entry (i), atl->Entry (i), 0)) 
        return false;
  }
  
  // match base types
  return matchArgument (ptype->BaseType (), atype->BaseType (), aexpr);
}


// ��14.5.5.2 partial ordering rules
// return 0 if equal, 1 if more, and -1 if less specialized than other
int InstantiationCandidate::compare (InstantiationCandidate &other) {
  InstantiationCandidate cand1;
  cand1.initialize (other.TemplateInfo ()->SpecializationName (), ObjectInfo (), TemplateInfo ());
  for (unsigned i = 0; i < Arguments (); i++) 
    cand1.addArgument (Argument (i));
  
  InstantiationCandidate cand2;
  cand2.initialize (TemplateInfo ()->SpecializationName (), other.ObjectInfo (), other.TemplateInfo ());
  for (unsigned i = 0; i < other.Arguments (); i++) 
    cand2.addArgument (other.Argument (i));

  // match against the other
  bool matches1 = cand1.match (cand2); 
  bool matches2 = cand2.match (cand1); 
  
  // equal specialized
  if (matches1 && matches2) 
    return 0;
  // more specialized
  if (matches1) 
    return -1;
  // less specialized
  if (matches2) 
    return 1;
  // equal specialized
  return 0;
}


bool InstantiationCandidate::setDeducedArgument (DeducedArgumentList &args, int pos, DeducedArgument *arg) {
  bool res = true;
  DeducedArgument *old = args[pos];
  if (! old) {
    args[pos] = arg;
  } else {
    res = (*arg == *old);
    delete arg;
  }
  return res;
}


void InstantiationCandidate::printArgumentList (std::ostream &out, bool print_default_args) const {
  out << "<";
  unsigned i = 0;
  for (; i < DeducedArgs (); i++) {
    DeducedArgument *arg = DeducedArg (i);

    // do not list default arguments
    if (! print_default_args && arg->isDefaultArg ())
      break;

    if (i) out << ",";
    out << *arg;
  }
  for (; i < InstantiationArgs (); i++) {
    DeducedArgument *arg = InstantiationArg (i);

    // do not list default arguments
    if (! print_default_args && arg->isDefaultArg ())
      break;

    if (i) out << ",";
    out << *arg;
  }
  out << ">";
}


Token* InstantiationCandidate::getPointOfInstantiationToken(CTree* p) const {
  Token* token = 0;
  CTree* point = p ? p : poi;
  if (point) {
    do {
      token = point->token();
    } while (! token && (point = point->Parent()));
  }
  return token;
}


bool InstantiationCandidate::maxInstDepthReached () {
  // ISO says not more than 17
  unsigned max_depth = 17;
  // maximum set by user?
  const ConfOption *opt = getProject ()->config ().Option ("--template-depth");
  if (opt && opt->Arguments () > 0) {
    max_depth = strtol (opt->Argument (0), NULL, 10);
  }
  if (tpl_info->Depth () > max_depth) {
    *err << sev_error << getPointOfInstantiationToken()->location ()
         << "maximum instantiation depth ("
         << max_depth << ") reached" << endMessage;
    return true;
  }
  return false;
}


void InstantiationCandidate::printCodeLine (Token* token) const {
  int line = token->location().line();
  Unit* u = token->unit();
  for (token = u->first(); token; token = u->next(token)) {
    if (token->location().line() == line) {
      if (token->is_whitespace() && strchr(token->text(), '\n'))
        break;
      *err << token->text();
    }
  }
  *err << endMessage;
}


void InstantiationCandidate::printInstantiationErrorHeader (Token* token) const {
  if (token) {
    CObjectInfo* obj = obj_info;
    if (! obj && instance)
      obj = instance->Object();

    if (obj) {
      bool verbose = obj->SemDB()->Project()->config().Option("--verbose-errors");
      *err << token->location()
           << "In instantiation of `" << *obj->TypeInfo();
      if (!obj->TemplateInstance()) {
        std::ostringstream args;
        printArgumentList(args);
        *err << args.str().c_str();
      }
      *err << "'" << endMessage;
      if (verbose)
        printCodeLine(token);

      /* TODO: provide a way to get this information
      Unit* unit = token->unit();
      if (unit->isTemplateInstance()) {
        unit = ((TemplateInstanceUnit*)unit)->ContainingUnit();

        while (unit->isTemplateInstance()) {
          TemplateInstanceUnit* tiunit = (TemplateInstanceUnit*)unit;
          CTemplateInstance* inst = tiunit->TemplateInstance();
          CTree* poi = inst ? inst->PointOfInstantiation() : 0;
          token = poi ? poi->token() : 0;

          if (token && inst->Object()) {
            *err << token->location()
                 << "  instantiated from `" << *inst->Object()->TypeInfo()
                 << "'" << endMessage;
            if (verbose)
              printCodeLine(token);
          }

          unit = tiunit->ContainingUnit();
        }
      }*/
    }
  }
}


CStructure *InstantiationCandidate::createInstanceScope() {
  CStructure *scope = tpl_info->Parent()->newNamespace();
  scope->NamespaceInfo()->aroundInstantiation(true);
  scope->Name("%<anonymous>");
  scope->TypeInfo(&CTYPE_UNDEFINED);
  scope->SemDB(obj_info->SemDB());
  scope->SourceInfo()->FileInfo(obj_info->SourceInfo()->FileInfo());
  scope->SourceInfo()->StartToken(PointOfInstantiation()->token_node());
  return scope;
}


bool InstantiationCandidate::parseDirectArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args) {
  // check maximal instantiation depth
  if (maxInstDepthReached())
    return false;

  // create new template instance scope
  inst_scope = createInstanceScope();

  // parse the arguments
  return parseArguments(pos, numargs, args);
}


InstantiationParser& InstantiationCandidate::getParser() {
  if (! parser) {
    parser = new InstantiationParser();
    parser->init(obj_info, tpl_info, inst_scope);
  }
  return *parser;
}


bool InstantiationCandidate::parseArguments (unsigned pos, unsigned numargs, CT_TemplateArgList *args) {
  unsigned i = 0;

  // parse each direct or default argument
  for (; i < pos; i++) {
    CTemplateParamInfo *param = tpl_info->Parameter(i);
    bool is_default_arg = (i >= numargs);
    CTree *arg;

    if (is_default_arg) {
      // default template argument
      arg = param->DefaultArgument();
    } else {
      // direct template argument
      arg = args->Entry(i);
    }

    if (param->isTemplate()) {
      // template-template parameter
      CTypeInfo* type = 0;
      if (!is_default_arg) {
        // get the template type
        CObjectInfo* oinfo;
        CTree* sn = arg;
        if (sn->NodeName() == CT_NamedType::NodeId())
          sn = sn->Son(0)->Son(0);
        if (!(sn->NodeName() == CT_SimpleName::NodeId() ||
              sn->NodeName() == CT_RootQualName::NodeId() ||
              sn->NodeName() == CT_QualName::NodeId()) ||
            !sn->SemObject() ||
            !(oinfo = sn->SemObject()->Object()) ||
            !(oinfo->Record() && oinfo->Record()->isTemplate())) {
          SEM_ERROR(arg, "expected a class template as argument " << i+1);
          break;
        }
        type = oinfo->TypeInfo()->VirtualType();
      }
      if (!parseTemplateTemplateArgument(param, arg, type)) {
        SEM_ERROR(arg, "invalid template argument " << i+1);
        break;
      }
    }
    else if (param->isTypeParam()) {
      // template type parameter
      CTypeInfo* type = 0;
      if (!is_default_arg) {
        // get the type
        CObjectInfo* oinfo;
        if (!(arg->NodeName() == CT_NamedType::NodeId()) ||
            !arg->SemObject() ||
            !(oinfo = arg->SemObject()->Object())) {
          SEM_ERROR(arg, "expected a type as argument " << i+1);
          break;
        }
        // if the type in the argument list is a bitfield type,
        // take the base type. Otherwise take it as it is.
        type = oinfo->TypeInfo();
        if (type->isBitField ())
          type = type->BaseType();
      }
      if (!parseTypeTemplateArgument(param, arg, type)) {
        SEM_ERROR(arg, "invalid template argument " << i+1);
        break;
      }
    }
    else {
      // template non-type parameter
      CConstant* value = 0;
      if (!is_default_arg) {
        // not a non-type argument
        if (arg->NodeName() == CT_NamedType::NodeId()) {
          SEM_ERROR(arg, "expected a constant as argument " << i+1);
          break;
        }
        // need value of constant
        if (!arg->Value() || !arg->Value()->Constant()) {
          SEM_ERROR(arg, "expected a constant as argument " << i+1);
          break;
        }
        value = arg->Value()->Constant();
      }
      if (!parseNonTypeTemplateArgument(param, arg, value)) {
        SEM_ERROR(arg, "invalid template argument " << i+1);
        break;
      }
    }
  }

  // return true if all arguments could be successfully parsed
  return i == pos;
}


bool InstantiationCandidate::parseNonTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CConstant* value) {
  bool is_default_arg = !value;

  if (is_default_arg && !CCSemExpr::isDependent(tree)) {
    // default argument does not depend on template parameters, get its value
    if (tree->Value())
      value = tree->Value()->Constant();
  }

  if (!value && tree) {
    // dependent default argument, parse it again to determine the real value
    bool delayed = false;
    tree = getParser().parse(tpl_info, tree, &InstantiationSyntax::ass_expr, delayed);

    if (tree && !parser->failed()) {
      // resolve the expression and calculate the value
      CCSemExpr cse(parser->builder().err(), inst_scope);
      cse.resolveExpr(tree);

      if (tree->Value())
        value = tree->Value()->Constant();
    }
  }

  if (!value)
    return false;

  // introduce constant with template argument as value
  CObjectInfo* info = introduceObjectForNonTypeTemplateArgument(param, tree, value);
  if (!info)
    return false;

  // successfully deduced
  darguments.append(new DeducedArgument(param, value, tree, is_default_arg, !is_default_arg));

  return true;
}


CObjectInfo* InstantiationCandidate::introduceObjectForNonTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CConstant* value) {
  // no value type given
  CTypeInfo* type = param->ValueType();
  if (!type || type->isDependent()) {
    // dependent parameter type, parse it again to determine real type
    bool delayed = false;
    CT_NonTypeParamDecl* ntpd = (CT_NonTypeParamDecl*) getParser().parse(tpl_info, param->Tree(), &InstantiationSyntax::non_type_param_without_init, delayed);

    if (ntpd && !parser->failed()) {
      CT_SimpleName* name = findName(ntpd->Declarator());
      if (name)
        type = name->Type();
    }
  }
  if (!type)
    return 0;

  type = type->Duplicate();
  if (!type->isConst()) {
    // need to make type const to be accepted as constant
    if (type->TypeQualified())
      type->TypeQualified()->isConst(true);
    else
      type = new CTypeQualified(type, true, false, false);
  }

  // TODO: Double check memory management for syntax trees of template instances
  // The Init-Expression of a non-type template parameter had referenced a deleted
  // node. Therefore, I (Olaf) removed the following condition:
//  if (!tree) {
    // no syntax tree given
    tree = new CT_Expression();
    tree->SemValue()->setValueRef(value);
    tpl_info->addTreeFragment(tree);
//  }

  // introduce constant with template argument as value
  CAttributeInfo* info = inst_scope->newAttribute();
  info->Init(tree);
  info->Name(param->Name());
  info->TypeInfo(type);
  info->SourceInfo()->FileInfo(param->SourceInfo()->FileInfo());
  CT_Token* token_node = tree->token_node();
  info->SourceInfo()->StartToken(token_node ? token_node : param->SourceInfo()->StartToken());

  return info;
}


bool InstantiationCandidate::parseTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type) {
  bool is_default_arg = !type;

  if (is_default_arg && !CCSemExpr::isDependent(tree)) {
    // default argument does not depend on template parameters, get its type
    CT_SimpleName* name = findPrivateName(tree);
    if (name)
      type = name->Type();
  }

  if (!type && tree) {
    // dependent default argument, parse it again to determine final type
    bool delayed = false;
    tree = getParser().parse(tpl_info, tree, &InstantiationSyntax::type_id, delayed);

    if (tree && !parser->failed()) {
      CT_SimpleName* name = findPrivateName(tree);
      if (name)
        type = name->Type();
    }
  }

  if (!type)
    return false;

  // introduce typedef for argument type with template parameter name
  CObjectInfo* info = introduceObjectForTypeTemplateArgument(param, tree, type->Duplicate());
  if (!info)
    return false;

  // successfully deduced
  darguments.append(new DeducedArgument(param, info->TypeInfo()->VirtualType(), tree, is_default_arg, !is_default_arg));

  return true;
}


CObjectInfo* InstantiationCandidate::introduceObjectForTypeTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type) {
 // introduce typedef for argument type with template parameter name
  CObjectInfo* info = inst_scope->newTypedef();
  info->Name(param->Name());
  info->TypeInfo(type);
  info->TypeInfo(info->TypeInfo()->TypedefInfo(info));
  info->SourceInfo()->FileInfo(param->SourceInfo()->FileInfo());
  info->SourceInfo()->StartToken(tree ? tree->token_node() : param->SourceInfo()->StartToken());
  return info;
}


bool InstantiationCandidate::parseTemplateTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type) {
  bool is_default_arg = !type;

  if (is_default_arg && !CCSemExpr::isDependent(tree)) {
    // default argument does not depend on template parameters, get its type
    CT_SimpleName* name = findName(tree);
    if (name)
      type = name->Type();
  }

  if (!type && tree) {
    // default argument depends on template parameters, parse it again
    getParser().semantic().enter_expr();
    bool delayed = false;
    tree = parser->parse(tpl_info, tree, &InstantiationSyntax::id_expr, delayed);
    parser->semantic().leave_expr();

    if (tree && !parser->failed()) {
      // resolve the expression and calculate the value
      CCSemExpr cse(parser->builder().err(), inst_scope);
      cse.resolveExpr(tree);

      CT_SimpleName* name = findName(tree);
      if (name)
        type = name->Type();
    }
  }

  if (!type)
    return false;

  // introduce member alias for template with template parameter name
  CObjectInfo* info = introduceObjectForTemplateTemplateArgument(param, tree, type->Duplicate());
  if (!info)
    return false;

  // successfully deduced
  darguments.append(new DeducedArgument(param, info->TypeInfo(), tree, is_default_arg, !is_default_arg));

  return true;
}


bool InstantiationCandidate::isDependent(CTemplateInfo* tpl) const {
  for (unsigned i = 0; i < tpl->Parameters(); i++) {
    CTemplateParamInfo* param = tpl->Parameter(i);
    if (param && isDependent((CT_TemplateParamDecl*) param->Tree()))
      return true;
  }
  return false;
}


bool InstantiationCandidate::isDependent(CT_TemplateParamDecl* pd) const {
  if (!pd)
    return false;

  if (pd->DefaultArgument() && CCSemExpr::isDependent(pd->DefaultArgument()))
    return true;

  if (pd->NodeName() == CT_NonTypeParamDecl::NodeId()) {
    CObjectInfo* obj = pd->SemObject()->Object();
    if (obj && obj->TemplateParamInfo()) {
      CTypeInfo* type = obj->TemplateParamInfo()->ValueType();
      return type && type->isDependent();
    }
  } else if (pd->NodeName() == CT_TypeParamDecl::NodeId()) {
    CT_TypeParamDecl* tpd = (CT_TypeParamDecl*)pd;
    if (tpd->Parameters() && tpd->Parameters()->Scope())
      return isDependent(tpd->Parameters()->Scope()->TemplateInfo());
  }

  return false;
}

CObjectInfo* InstantiationCandidate::introduceObjectForTemplateTemplateArgument(CTemplateParamInfo* param, CTree* tree, CTypeInfo* type) {
  if (!type->TypeRecord())
    return 0;

  CTemplateInfo* ttpl = param->TemplateTemplate();
  if (isDependent(ttpl)) {
    // template template depends on other template parameters, parse again
    bool delayed = false;
    CT_TypeParamDecl* tpd = (CT_TypeParamDecl*) getParser().parse(tpl_info, param->Tree(), &InstantiationSyntax::type_param_without_init, delayed);

    if (tpd && !parser->failed()) {
      if (tpd->Parameters())
        ttpl = tpd->Parameters()->Scope()->TemplateInfo();
    }
  }

  // check if template parameter lists match
  CRecord* record = type->TypeRecord()->Record();
  if (!CTemplateInfo::equalParameters(ttpl, record->TemplateInfo()))
    return 0;

  // introduce member alias for template with template parameter name
  CObjectInfo* info = inst_scope->newMemberAlias(type->TypeRecord()->Record());
  info->Name(param->Name());
  info->MemberAliasInfo()->Member(type->TypeRecord()->Record());
  info->TypeInfo(type);
  info->SourceInfo()->FileInfo(param->SourceInfo()->FileInfo());
  info->SourceInfo()->StartToken(tree ? tree->token_node() : param->SourceInfo()->StartToken());

  return info;
}


CTree* InstantiationCandidate::instantiate(CCInstantiation* inst) {
  // add all non-direct non-default arguments
  for (unsigned i = 0; i < DeducedArgs(); i++) {
    DeducedArgument *arg = DeducedArg(i);
    CTemplateParamInfo *param = getMatchingParameter(arg->TemplateParam());
    if (!param) {
      continue;
    }

    // direct or default argument
    if ((arg->isDefaultArg() || arg->isDirectArg()) && arg->TemplateParam()->Template() == tpl_info) {
      // nothing to do
      continue;
    }

    if (param->isTemplate()) {
      // deduced template-template argument
      if (!introduceObjectForTemplateTemplateArgument(param, 0, arg->Type()->Duplicate()))
        return (CTree*)0;
    } else if (param->isTypeParam()) {
      // deduced type argument
      if (!introduceObjectForTypeTemplateArgument(param, 0, arg->Type()->Duplicate()))
        return (CTree*)0;
    } else {
      // deduced non-type argument
      if (!introduceObjectForNonTypeTemplateArgument(param, 0, arg->Value()))
        return (CTree*)0;
    }
  }

  // parse the template without the template header
  CTree* tree = 0;
  if (TemplateInfo()->Tree()) {
    // create new parser for parsing the instance
    if (parser)
      delete parser;
    parser = new InstantiationParser();
    parser->init(obj_info, tpl_info, inst_scope, inst);

    // parse the template instance
    CTree* decl = TemplateInfo()->Tree()->Declaration(), *son;
    bool delayed = false;
    if (decl->NodeName() == CT_FctDef::NodeId())
      tree = parser->parse(tpl_info, decl, &InstantiationSyntax::fct_def, delayed);
    else if (decl->NodeName() == CT_ObjDecl::NodeId() &&
             decl->Sons() && (son = decl->Son(0)) &&
             son->Sons() && (son = son->Son(0)) &&
             son->NodeName() == CT_ClassDef::NodeId())
      tree = parser->parse(tpl_info, decl, &InstantiationSyntax::class_spec, delayed);
    else {
      tree = parser->parse(tpl_info, decl, &InstantiationSyntax::decl, delayed);
    }
    if (delayed)
      detected_delayed_parse_node = true;

    // semantic expression analysis, only if instantiating function bodies,
    // not needed otherwise
    if (tree && getProject()->config().Option("--inst-fct-bodies"))
      parser->resolve(tpl_info, inst_scope, tree);
  }
  return tree;
}


void InstantiationCandidate::printInstantiationHeader() const {
  std::cout << std::endl;
  if (PointOfInstantiation()) {
    std::cout << getPointOfInstantiationToken()->location() << ": ";
  }
  std::cout << obj_info->Name();
  printArgumentList(std::cout);
  std::cout << ": instantiated here" << std::endl;
  if (tpl_info->Tree() && tpl_info->Tree()->token() && tpl_info->ObjectInfo()) {
    std::cout << tpl_info->Tree()->token()->location()
              << ": instantiation of template "
              << tpl_info->ObjectInfo()->QualName(true) << std::endl;
  }
}


CT_SimpleName *InstantiationCandidate::findPrivateName (CTree *node) {
  const char *id = node->NodeName ();
  if (id == CT_PrivateName::NodeId ())
    return (CT_SimpleName*)node;
  else if (id == CT_NamedType::NodeId ())
    return findPrivateName (((CT_NamedType*)node)->Declarator ());
  else if (id == CT_FctDeclarator::NodeId ())
    return findPrivateName (((CT_FctDeclarator*)node)->Declarator ());
  else if (id == CT_ArrayDeclarator::NodeId ())
    return findPrivateName (((CT_ArrayDeclarator*)node)->Declarator ());
  else if (id == CT_PtrDeclarator::NodeId ())
    return findPrivateName (((CT_PtrDeclarator*)node)->Declarator ());
  else if (id == CT_MembPtrDeclarator::NodeId ())
    return findPrivateName (((CT_MembPtrDeclarator*)node)->Declarator ());
  else if (id == CT_BracedDeclarator::NodeId ())
    return findPrivateName (((CT_BracedDeclarator*)node)->Declarator ());
  else if (id == CT_BitFieldDeclarator::NodeId ())
    return findPrivateName (((CT_BitFieldDeclarator*)node)->Declarator ());
  else if (id == CT_RefDeclarator::NodeId ())
    return findPrivateName (((CT_RefDeclarator*)node)->Declarator ());
  else if (id == CT_InitDeclarator::NodeId ())
    return findPrivateName (((CT_InitDeclarator*)node)->Declarator ());
  return (CT_SimpleName*)0;
}


CT_SimpleName *InstantiationCandidate::findName (CTree *node) {
  const char *id = node->NodeName ();
  if (node->IsSimpleName ())
    return (CT_SimpleName*)node;
  else if (id == CT_NamedType::NodeId ())
    return findName (((CT_NamedType*)node)->Declarator ());
  else if (id == CT_FctDeclarator::NodeId ())
    return findName (((CT_FctDeclarator*)node)->Declarator ());
  else if (id == CT_ArrayDeclarator::NodeId ())
    return findName (((CT_ArrayDeclarator*)node)->Declarator ());
  else if (id == CT_PtrDeclarator::NodeId ())
    return findName (((CT_PtrDeclarator*)node)->Declarator ());
  else if (id == CT_MembPtrDeclarator::NodeId ())
    return findName (((CT_MembPtrDeclarator*)node)->Declarator ());
  else if (id == CT_BracedDeclarator::NodeId ())
    return findName (((CT_BracedDeclarator*)node)->Declarator ());
  else if (id == CT_BitFieldDeclarator::NodeId ())
    return findName (((CT_BitFieldDeclarator*)node)->Declarator ());
  else if (id == CT_RefDeclarator::NodeId ())
    return findName (((CT_RefDeclarator*)node)->Declarator ());
  else if (id == CT_InitDeclarator::NodeId ())
    return findName (((CT_InitDeclarator*)node)->Declarator ());
  return (CT_SimpleName*)0;
}


} // namespace Puma
