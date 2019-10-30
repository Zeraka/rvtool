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

#include "Puma/CCInstantiation.h"
#include "Puma/CCSemVisitor.h"
#include "Puma/CStructure.h"
#include "Puma/CClassInstance.h"
#include "Puma/CUnionInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/CTemplateInfo.h"
#include "Puma/CTemplateParamInfo.h"
#include "Puma/CTree.h"
#include "Puma/PreTreeNodes.h"
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
#include "Puma/InstantiationParser.h"

#include <stdio.h>      /* sprintf() */
#include <stdlib.h>     /* strtol() */
#include <sstream>      /* ostringstream */

namespace Puma {


#define SEM_ERROR(loc__,mesg__) \
  {if (report) {\
    err << sev_error << loc__->token ()->location () \
        << mesg__ << endMessage;}}


CCInstantiation::CCInstantiation (ErrorSink &e, bool rep) : err (e) {
  instance      = 0;
  last_instance = 0;
  report        = rep;
  current_scope = 0;

  base_candidate = new InstantiationCandidate;
  candidates[0] = base_candidate;
  base_candidate->initialize (rep ? &e : (ErrorSink*)0);
}


CCInstantiation::~CCInstantiation () {
  for (long i = 0; i < candidates.length (); i++) {
    InstantiationCandidate *candidate = candidates.lookup (i);
    if (candidate == base_candidate)
      base_candidate = 0;
    delete candidate;
  }
  if (base_candidate)
    delete base_candidate;
}


// start instantiation process
CObjectInfo *CCInstantiation::instantiate (CTree *node,
 CObjectInfo *binfo, bool real_inst, CStructure *cs, bool inst_immediately) {
  // don't call twice
  if (instance || ! binfo || ! node)
    return instance;

  First ().initialize (node, binfo);
  current_scope = cs;

  if (real_inst && ! First ().canBeInstantiated ())
    real_inst = false;

  // §14.8.2 deduce template arguments
  if (! First ().deduceArguments (real_inst)) {
    return instance;
  }

  // choose the appropriate template specialization (if any)
  if (! chooseSpecialization ()) {
    return instance;
  }

  // instantiate template if not already done before
  if (! alreadyInstantiated (real_inst)) {
    // create an instance of the template
    createPseudoInstance ();

    if (instance) {
      instance->TemplateInstance ()->canInstantiate (real_inst);
      if (real_inst && inst_immediately) {
        instance = instance->TemplateInstance ()->instantiate (cs) ? instance : 0;
      }
    }
  }

  return instance;
}


// §14.5.4.1 matching of template partial specializations
bool CCInstantiation::chooseSpecialization () {
  bool no_default;
  unsigned entries;
  CObjectInfo *info;
  CTemplateInfo *tinfo;
  CT_TemplateName *name;
  CT_TemplateArgList *args;

  // only class templates can be partially specialized
  if (ObjectInfo ()->FunctionInfo ())
    return true;

  // there are no known specializations if we are parsing a template
  // and access a template template parameter
  if (ObjectInfo ()->TemplateParamInfo ())
    return true;

  // find all specializations
  std::list<CTemplateInfo*> specializations;
  CTemplateInfo *curr_template = TemplateInfo ();
  do {
    for (unsigned i = 0; i < curr_template->Specializations (); i++) {
      tinfo = curr_template->Specialization (i);
      specializations.push_back(tinfo);
    }
    // find next connected template declaration/definition
    if (!curr_template->ObjectInfo ()) {
      break;
    }
    CObjectInfo *next = curr_template->ObjectInfo ()->NextObject ();
    if (!next || next->TemplateInstance () || !next->ClassInfo () || !next->Template ()) {
      break;
    }
    curr_template = next->Template ();
  } while (curr_template != TemplateInfo ());

  // handle each specialization
  for (std::list<CTemplateInfo*>::iterator i = specializations.begin();
      i != specializations.end (); ++i) {
    tinfo = *i;
    if (! tinfo->ObjectInfo ()) // internal error
      continue;
    info = tinfo->ObjectInfo ()->DefObject ();
    name = tinfo->SpecializationName ();
    if (! name) // internal error
      continue;
    args = name->Arguments ();
    if (! args) // internal error
      continue;

    // create new instantiation candidate
    InstantiationCandidate *cand = new InstantiationCandidate;
    cand->initialize (PointOfInstantiation (), info, tinfo, report ? &err : (ErrorSink*)0);

    // add the template arguments of the partial specialization
    entries = args->Entries ();
    for (unsigned j = 0; j < entries; j++)
      cand->addArgument (args->Entry (j));

    // add default arguments of the base template if not overwritten
    no_default = false;
    for (unsigned j = entries; j < TemplateInfo ()->Parameters (); j++) {
      // no default argument? this is an error
      if (! TemplateInfo ()->Parameter (j)->DefaultArgument ()) {
        no_default = true;
        cand->reset ();
        delete cand;
        break;
      }
      CTree* arg = (j < DeducedArgs () ? DeducedArg (j)->TemplateArg () : 0);
      cand->addArgument (arg ? arg : TemplateInfo ()->Parameter (j)->DefaultArgument ());
    }
    if (no_default)
      continue;

    // §14.5.4.1.2 try to match the partial specialization 
    // against the actual template argument list
    if (! cand->match (First())) {
      // does not match, not a candidate for instantiation
      cand->reset ();
      delete cand;
    } else {
      candidates[candidates.length ()] = cand;
    }
  }

  // if exactly one matching specialization is found, the 
  // instantiation is generated from that specialization
  if (candidates.length () == 2) {
    // remove the base template as candidate
    candidates.remove (0); 
  // if more than one specialization is matching, the
  // best specialization is chosen using the partial 
  // ordering rules (§14.5.4.2 and §14.5.5.2)
  } else if (candidates.length () > 2) {
    return chooseBestSpecialization ();
  }
  
  return true;
}


bool CCInstantiation::chooseBestSpecialization () {
  // run a tournament to choose the best specialization 
  // comparing two specializations using the partial 
  // ordering rules (§14.5.4.2 and §14.5.5.2)
  InstantiationCandidate *challenger, *champion;
  unsigned num;

  num = candidates.length ();
  champion = candidates.lookup (num-1);
  for (unsigned i = num-1; i > 1; i--) {
    // current challenger
    challenger = candidates.lookup (i-1); 
    
    // let the champion face the challenger 
    switch (champion->compare (*challenger)) {
      // champion has won the round 
      case 1: 
        // elliminate challenger
        delete candidates.lookup (i-1); 
        candidates.remove (i-1); 
        break;
      // challenger has won the round
      case -1:
        // elliminate old champion
        delete candidates.lookup (i); 
        candidates.remove (i); 
        // challenger is the new champion
        champion = challenger; 
        break;
      // challenger and champion are equal
      default:
        // case 0: ambiguous up till now
        champion = challenger; 
        break;
    }
  }
  
  // verify that the champion is better than all other remaining 
  // candidates (the champion did not yet faced)
  for (unsigned i = candidates.length (); i > 1; i--) {
    challenger = candidates.lookup (i-1); 
    if (challenger == champion)
      continue;
      
    if (champion->compare (*challenger) == 1) {
      delete candidates.lookup (i-1); 
      candidates.remove (i-1);
    }
  }
  
  // if exactly one specialization left over, the 
  // instantiation is generated from that specialization
  if (candidates.length () == 2) {
    // remove the base template as candidate
    candidates.remove (0); 
    return true;
  } 

  // more than one specialization left over,
  // so the instantiation is ambiguous 
  if (report) {
    std::ostringstream name;
    name << ObjectInfo ()->Name ();
    base_candidate->printArgumentList (name);
    err << sev_error << getPoiToken()->location ()
        << "Instantiation of `" << name.str ().c_str ()
        << "' is ambiguous" << endMessage;
    for (int i = 1; i < candidates.length (); i++) {
      InstantiationCandidate *cand = candidates[i];
      if (cand->ObjectInfo ()->Tree () && cand->ObjectInfo ()->Tree ()->token ())
        err << cand->ObjectInfo ()->Tree ()->token ()->location ();
      if (i == 1)
        err << "candidates are: ";
      else
        err << "                ";
      err << name.str ().c_str () << endMessage;
    }
  }

  return false;
}


bool CCInstantiation::alreadyInstantiated (bool real_inst) {
  last_instance = 0;
  for (unsigned i = 0; i < TemplateInfo ()->Instances (); i++) {
    CObjectInfo *ti = TemplateInfo ()->Instance (i);
    bool cont = false;

    if ((TemplateInfo ()->isBaseTemplate () ==
         ti->TemplateInstance ()->Template ()->isBaseTemplate ()) &&
        (DeducedArgs () == ti->TemplateInstance ()->DeducedArgs ())) {
      for (unsigned j = 0; j < ti->TemplateInstance ()->DeducedArgs (); j++)
        if (*DeducedArg (j) != *ti->TemplateInstance ()->DeducedArg (j))
          cont = true;
    } else
      cont = true;

    if (! cont) {
      instance = ti;
      if ((ti->FunctionInfo () && ti->FunctionInfo ()->isFctDef ()) ||
          (ti->Record () && ti->Record ()->isDefined ())) {
        last_instance = 0;
        return true;
      } else
        last_instance = ti;
    }
  }

  instance = 0;
  if (last_instance) {
    if (real_inst &&
        ((ObjectInfo ()->FunctionInfo () && ObjectInfo ()->FunctionInfo ()->isFctDef ()) ||
         (ObjectInfo ()->Record () && ObjectInfo ()->Record ()->isDefined ())))
      return false;
    instance = last_instance;
    last_instance = 0;
    return true;
  }
  return false;
}


void CCInstantiation::createPseudoInstance () {
  CTemplateInstance *ti = 0;
  CStructure* scope = InstanceScope () ? InstanceScope () : First ().createInstanceScope ();

  if (ObjectInfo ()->FunctionInfo ()) {
    instance = scope->newFunction (true);
    instance->FunctionInfo ()->isConstructor (ObjectInfo ()->FunctionInfo ()->isConstructor ());
    instance->FunctionInfo ()->isDestructor (ObjectInfo ()->FunctionInfo ()->isDestructor ());
    instance->FunctionInfo ()->isOperator (ObjectInfo ()->FunctionInfo ()->isOperator ());
    instance->FunctionInfo ()->isConversion (ObjectInfo ()->FunctionInfo ()->isConversion ());
    if (instance->FunctionInfo ()->isConversion ()) {
      instance->FunctionInfo ()->ConversionType (ObjectInfo ()->FunctionInfo ()->ConversionType ());
    }
    ti = instance->TemplateInstance ();
  } else if (ObjectInfo ()->UnionInfo ()) {
    instance = scope->newUnion (true);
    ti = instance->TemplateInstance ();
  } else if (ObjectInfo ()->ClassInfo ()) {
    instance = scope->newClass (true);
    ti = instance->TemplateInstance ();
  } else if (ObjectInfo ()->TemplateParamInfo ()) {
    instance = ObjectInfo ()->TemplateParamInfo ()->TemplateTemplate ()->newTemplateParam (false);
    instance->TemplateParamInfo ()->TemplateInfo (ObjectInfo ()->TemplateParamInfo ()->TemplateInfo ());
    instance->TemplateParamInfo ()->TemplateInstance (new CTemplateInstance);
    instance->TemplateParamInfo ()->isTypeParam (ObjectInfo ()->TemplateParamInfo ()->isTypeParam ());
    instance->TemplateParamInfo ()->ValueType (ObjectInfo ()->TemplateParamInfo ()->ValueType ()->Duplicate ());
    instance->TemplateParamInfo ()->TemplateTemplate (ObjectInfo ()->TemplateParamInfo ()->TemplateTemplate ());
    ObjectInfo ()->NextObject (instance);
    ti = instance->TemplateParamInfo ()->TemplateInstance ();
  } else {
    return; // unsupported object type
  }

  instance->Name (ObjectInfo ()->Name ());
  instance->SourceInfo ()->FileInfo (ObjectInfo ()->SourceInfo ()->FileInfo ());
  instance->SourceInfo ()->StartToken (PointOfInstantiation ()->token_node ());
  instance->TypeInfo (ObjectInfo ()->TypeInfo ()->Duplicate ());
  instance->Specifiers (ObjectInfo ()->Specifiers ());
  instance->Protection (ObjectInfo ()->Protection ());
  instance->Linkage (ObjectInfo ()->Linkage ());
  instance->Storage (ObjectInfo ()->Storage ());
  instance->Language ().Type (ObjectInfo ()->Language ().Type ());
  instance->isVirtual (ObjectInfo ()->isVirtual ());
  instance->isStatic (ObjectInfo ()->isStatic ());
  instance->isThreadLocal (ObjectInfo ()->isThreadLocal ());
  instance->isExtern (ObjectInfo ()->isExtern ());
  instance->isMutable (ObjectInfo ()->isMutable ());
  instance->isRegister (ObjectInfo ()->isRegister ());
  instance->isExplicit (ObjectInfo ()->isExplicit ());
  instance->isInline (ObjectInfo ()->isInline ());
  instance->isAuto (ObjectInfo ()->isAuto ());
  instance->AssignedScope (ObjectInfo ()->AssignedScope ());

  if (ObjectInfo ()->QualifiedScope ()) {
    if (instance->FunctionInfo ())
      instance->FunctionInfo ()->QualifiedScope (ObjectInfo ()->QualifiedScope ());
    else if (instance->Record ())
      instance->Record ()->QualifiedScope (ObjectInfo ()->QualifiedScope ());
  }

  ti->PointOfInstantiation (PointOfInstantiation (), current_scope);
  ti->Template (TemplateInfo ());
  ti->Object (instance);
  ti->isPseudoInstance (true);

  if (instance->FunctionInfo ())
    instance->TypeInfo ()->TypeFunction ()->FunctionInfo (instance->FunctionInfo ());
  else if (instance->TemplateParamInfo ())
    instance->TypeInfo ()->TypeTemplateParam ()->TemplateParamInfo (instance->TemplateParamInfo ());
  else
    instance->TypeInfo ()->TypeRecord ()->Record (instance->Record ());

  for (unsigned i = 0; i < DeducedArgs (); i++)
    ti->addDeducedArg (DeducedArg (i));
  for (unsigned i = 0; i < base_candidate->DeducedArgs (); i++)
    ti->addInstantiationArg (base_candidate->DeducedArg (i));

  First ().TemplateInstance (ti);
  First ().forgetDeducedArgs ();

  if (base_candidate != &First ()) {
    base_candidate->forgetDeducedArgs ();
  }

  TemplateInfo ()->addInstance (instance);
}


void CCInstantiation::setupFromPseudoInstance (CTemplateInstance* instinfo, CStructure* scope) {
  instance = instinfo->Object ();
  current_scope = (scope && scope == instance) ? scope->Parent ()->Structure () : scope;
  CObjectInfo* obj = instinfo->Template ()->ObjectInfo ();
  if (! obj->FunctionInfo ())
    obj = obj->DefObject ();
  base_candidate->initialize (instinfo->PointOfInstantiation (), obj, obj->Template ());
  for (unsigned i = 0; i < instinfo->DeducedArgs (); i++)
    base_candidate->addDeducedArg (instinfo->DeducedArg (i));
  for (unsigned i = 0; i < instinfo->InstantiationArgs (); i++)
    base_candidate->addInstantiationArg (instinfo->InstantiationArg (i));
  base_candidate->TemplateInstance (instinfo);
}


bool CCInstantiation::instantiate (CTemplateInstance* instinfo, CStructure* instscope) {
  // setup from the pseudo instance information to
  // reset the correct instantiation information
  setupFromPseudoInstance(instinfo, instscope);

  // check maximal instantiation depth
  if (First().maxInstDepthReached()) {
    First().forgetDeducedArgs();
    removeInstance();
    return false;
  }

  // instantiate now
  CTree* tree = First().instantiate(this);

  // report errors and clean up
  if (!tree || First().getParser().failed()) {
    if (report && First().getParser().failed()) {
      First().printInstantiationErrorHeader(getPoiToken());
      First().getParser().builder().errors(err);
    }
    removeInstance();
  }

  First().forgetDeducedArgs();
  return instance;
}


Token* CCInstantiation::getPoiToken(CTemplateInstance* ti) {
  Token* tok = 0;
  if (ti) {
    CTree* poi = ti->PointOfInstantiation();
    do {
      tok = poi->token();
    } while (! tok && (poi = poi->Parent()));
  }
  else {
    tok = First().getPointOfInstantiationToken();
  }
  return tok;
}


void CCInstantiation::insertInstance (CObjectInfo *info) {
  if (info && info->TemplateInstance ()) {
    instance = info;
    instance->TemplateInstance ()->isInstantiated (true);
    if (last_instance && last_instance != instance)
      instance->NextObject (last_instance);
  }
}


void CCInstantiation::removeInstance () {
  if (instance && TemplateInfo()) {
    instance->TemplateInstance ()->isInstantiated (false);
    TemplateInfo()->removeInstance(instance);
  }
  instance = 0;
}


} // namespace Puma
