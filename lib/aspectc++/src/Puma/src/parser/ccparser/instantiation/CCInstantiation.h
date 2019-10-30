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

#ifndef __CCInstantiation__
#define __CCInstantiation__

#include "Puma/Array.h"
#include "Puma/ErrorSink.h"
#include "Puma/DeducedArgument.h"
#include "Puma/InstantiationCandidate.h"
#include <sstream>

namespace Puma {


class Token;
class CTree;
class CTypeInfo;
class CConstant;
class CStructure;
class CT_Program;
class CT_ExprList;
class CObjectInfo;
class CT_SimpleName;
class CT_TemplateName;
class CTemplateInfo;
class CTemplateParamInfo;
class CCOverloading;

class CCInstantiation {
  typedef Array<InstantiationCandidate*> CandArray;

  ErrorSink               &err;
  CObjectInfo             *instance;
  CObjectInfo             *last_instance;
  bool                     report;
  CStructure              *current_scope;
  CandArray                candidates;
  InstantiationCandidate  *base_candidate;

public:
  CCInstantiation (ErrorSink &, bool = true); 
  ~CCInstantiation ();

  CObjectInfo *instantiate (CTree *, CObjectInfo *, bool = true, CStructure * = 0, bool inst_immediately = false);

  bool instantiate (CTemplateInstance *pseudo_instance, CStructure* scope);
  bool detectedDelayedParseProblem ();

  CObjectInfo *ObjectInfo ();
  CStructure *InstanceScope ();
  CObjectInfo *Instance () const;
  CStructure *Scope () const;
  void addArgument (CTree *);

  void insertInstance (CObjectInfo *);
  void removeInstance ();

  void printInstantiationHeader();

private:
  InstantiationCandidate &First ();
  CTemplateInfo *TemplateInfo ();
  CTree *PointOfInstantiation ();
  Token *getPoiToken(CTemplateInstance* ti = 0);

  unsigned Arguments ();
  unsigned DeducedArgs ();
  CTree *Argument (unsigned);
  DeducedArgument *DeducedArg (unsigned);

  bool deduceArguments (bool);
  bool deduceArguments (unsigned);
  void deduceArguments (CTypeInfo *, CTypeInfo *, Array<DeducedArgument*> &);

  bool chooseSpecialization ();
  bool chooseBestSpecialization ();

  void setupFromPseudoInstance (CTemplateInstance* pseudo_instance, CStructure* scope);
  void createPseudoInstance ();
  bool alreadyInstantiated (bool);

  int getPosition (CTemplateParamInfo *);

  bool maxInstDepthReached ();
};

inline CObjectInfo *CCInstantiation::Instance () const
 { return instance; }
inline CStructure *CCInstantiation::Scope () const
 { return current_scope; }

inline InstantiationCandidate &CCInstantiation::First () 
 { return *candidates.lookup (0); }
inline CObjectInfo *CCInstantiation::ObjectInfo ()
 { return First ().ObjectInfo (); }
inline CTemplateInfo *CCInstantiation::TemplateInfo ()
 { return First ().TemplateInfo (); }
inline CStructure *CCInstantiation::InstanceScope ()
 { return First ().InstanceScope (); }
inline CTree *CCInstantiation::PointOfInstantiation ()
 { return First ().PointOfInstantiation (); }
inline bool CCInstantiation::detectedDelayedParseProblem ()
 { return First ().detectedDelayedParseNode (); }

inline unsigned CCInstantiation::Arguments () 
 { return First ().Arguments (); }
inline unsigned CCInstantiation::DeducedArgs ()
 { return First ().DeducedArgs (); }
inline CTree *CCInstantiation::Argument (unsigned i)
 { return First ().Argument (i); }
inline DeducedArgument *CCInstantiation::DeducedArg (unsigned i) 
 { return First ().DeducedArg (i); }

inline void CCInstantiation::addArgument (CTree *a)
 { First ().addArgument (a); }

inline void CCInstantiation::printInstantiationHeader()
 { First().printInstantiationHeader(); }

} // namespace Puma

#endif /* __CCInstantiation__ */
