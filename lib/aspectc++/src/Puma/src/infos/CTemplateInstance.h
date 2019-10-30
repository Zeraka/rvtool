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

#ifndef __CTemplateInstance_h__
#define __CTemplateInstance_h__

/** \file
 *  Semantic information about a template instance. */
 
#include "Puma/DeducedArgument.h"
#include "Puma/CTemplateInfo.h"

namespace Puma {


class CT_SimpleName;


/** \class CTemplateInstance CTemplateInstance.h Puma/CTemplateInstance.h
 *  Semantic information about a template instance.
 *  Contains the point of instantiation, the instantiated template,
 *  the instantiation arguments, and the deduced template arguments. 
 *
 *  The point of instantiation (POI) is the corresponding template-id. 
 *  \code X<int> x;  // X<int> is the POI \endcode 
 *
 *  The instantiation arguments are the arguments of the template-id
 *  at the POI.
 *  \code Y<int,1> y;  // 'int' and '1' are the instantiation arguments \endcode 
 *
 *  The deduced template arguments are calculated from the instantiation
 *  arguments and the template default arguments.
 *
 *  \code 
 *  template<class T1, class T2 = float>
 *  struct Foo { 
 *    template<class T3, class T4>
 *    T1 foo(T2,T3,T4*);
 *  };
 *
 *  void bar(bool b, char* s) {
 *    Foo<int> f;     // deduced arguments: T1=int,  T2=float
 *    f.foo(1,b,s);   // deduced arguments: T3=bool, T4=char
 *  }
 *  \endcode
 *
 *  If a template instance is not yet created (maybe because real template
 *  instantiation is disabled or due to late template instantiation), then
 *  this template instance is called a pseudo instance. */
class CTemplateInstance {
protected:
  /** The semantic information about the template. */
  CTemplateInfo           *_TemplateInfo;
  /** The semantic information about the instance. */
  CObjectInfo             *_ObjectInfo;
  /** The point of instantiation. */
  CTree                   *_PointOfInstantiation;
  /** The scope in which the template was instantiated. */
  CScopeInfo              *_PointOfInstantiationScope;
  /** The list of deduced template arguments. */
  Array<DeducedArgument*>  _DeducedArgs;
  /** The list of instantiation arguments. */
  Array<DeducedArgument*>  _InstantiationArgs;
  /** True if this is not a real template instance. */
  bool                     _Pseudo;
  /** True if this pseudo instance can be instantiated. */
  bool                     _CanInstantiate;
  /** True if instantiated. */
  bool                     _IsInstantiated;
  /** True if this is the instance of a template specialization. */
  bool                     _IsSpecialization;
  /** True if this node had a delayed parse problem. */
  bool                     _HasDelayedParseProblem;

public:
  /** Constructor. */
  CTemplateInstance ();
  /** Destructor. Destroys the instantiation and deduced arguments.
   *  Optionally destroys the translation unit of the instance code. */
  ~CTemplateInstance ();

  /** If this is a pseudo instance, instantiate the template. 
   *  \param scope The scope in which to instantiate the template. 
   *  \return true if instantiation succeeded. */
  bool instantiate (CStructure *scope);

  /** Get the semantic information about the instantiated template. */
  CTemplateInfo *Template () const;
  /** Get the semantic information about the class or function. */
  CObjectInfo *Object () const;
  /** Set the semantic information about the instantiated template. 
   *  \param info The template that was instantiated. */
  void Template (CTemplateInfo *info);
  /** Set the semantic information about the instance. 
   *  \param info The instance object. */
  void Object (CObjectInfo *info);

  /** Get the point of instantiation (e.g. the template-id). */
  CTree *PointOfInstantiation () const;
  /** Get the scope in which the template was instantiated. */
  CScopeInfo *PointOfInstantiationScope () const;
  /** Set the point of instantiation (e.g. the template-id).
   *  \param poi The template-id used to instantiate the template.
   *  \param scope The scope in which the template was instantiated. */
  void PointOfInstantiation (CTree *poi, CScopeInfo *scope);

  /** Set that the pseudo instance can be instantiated. */
  void canInstantiate (bool can);

  /** Return true if the pseudo instance can be instantiated. */
  bool canInstantiate ();

  /** Set whether this is a pseudo template instance.
   *  \param v True for yes, false for no. */
  void isPseudoInstance (bool v);
  /** Check if this is a pseudo template instance. */
  bool isPseudoInstance () const;
  
  /** Set whether this template instance really is instantiated.
   *  \param is True for yes, false for no. */
  void isInstantiated (bool is);
  /** Check if this template instance really is instantiated. */
  bool isInstantiated () const;

  /** Indicate a delayed parse problem during instantiation.
   *  \param has True for yes, false for no. */
  void hasDelayedParseProblem (bool has);

  /** Check whether this instance had a delayed parse problem. */
  bool hasDelayedParseProblem () const;
  
  /** Get the number of instantiation arguments. */
  unsigned InstantiationArgs () const;
  /** Get the n-th instantiation argument.
   *  \param n The index of the instantiation argument. */
  DeducedArgument *InstantiationArg (unsigned n) const;
  /** Add an instantiation argument. 
   *  \param arg The instantiation argument. */
  void addInstantiationArg (DeducedArgument *arg);

  /** Get the number of deduced arguments. */
  unsigned DeducedArgs () const;
  /** Get the n-th deduced template argument.
   *  \param n The index of the template argument. */
  DeducedArgument *DeducedArg (unsigned n) const;
  /** Add a deduced template argument.
   *  \param arg The deduced template argument. */
  void addDeducedArg (DeducedArgument *arg);
  /** Discard the deduced template arguments. */
  void clearDeducedArgs ();
};

inline CTemplateInstance::CTemplateInstance () :
  _TemplateInfo ((CTemplateInfo*)0),
  _ObjectInfo ((CObjectInfo*)0),
  _PointOfInstantiation ((CTree*)0),
  _DeducedArgs (1, 2),
  _InstantiationArgs (1, 2),
  _Pseudo (false),
  _CanInstantiate (true),
  _IsInstantiated (false),
  _IsSpecialization (false)
 {}

inline void CTemplateInstance::canInstantiate (bool can)
 { _CanInstantiate = can; }
inline bool CTemplateInstance::canInstantiate () 
 { return _CanInstantiate; }

inline void CTemplateInstance::isPseudoInstance (bool v)
 { _Pseudo = v; }
inline bool CTemplateInstance::isPseudoInstance () const
 { return _Pseudo; }

inline void CTemplateInstance::isInstantiated (bool is)
 { _IsInstantiated = is; }
inline bool CTemplateInstance::isInstantiated () const
 { return _IsInstantiated; }

inline void CTemplateInstance::hasDelayedParseProblem (bool has)
 { _HasDelayedParseProblem = has; }
inline bool CTemplateInstance::hasDelayedParseProblem () const
 { return _HasDelayedParseProblem; }

inline void CTemplateInstance::addDeducedArg (DeducedArgument *a)
 { _DeducedArgs.append (a); }
inline DeducedArgument *CTemplateInstance::DeducedArg (unsigned i) const
 { return _DeducedArgs.lookup (i); }
inline unsigned CTemplateInstance::DeducedArgs () const
 { return _DeducedArgs.length (); }
inline void CTemplateInstance::addInstantiationArg (DeducedArgument *a)
 { _InstantiationArgs.append (a); }
inline DeducedArgument *CTemplateInstance::InstantiationArg (unsigned i) const
 { return _InstantiationArgs.lookup (i); }
inline unsigned CTemplateInstance::InstantiationArgs () const
 { return _InstantiationArgs.length (); }
inline void CTemplateInstance::clearDeducedArgs () 
 { _DeducedArgs.reset (); _InstantiationArgs.reset (); }

inline CTemplateInfo *CTemplateInstance::Template () const
 { return _TemplateInfo; }
inline void CTemplateInstance::Template (CTemplateInfo *info) 
 { _TemplateInfo = info; _IsSpecialization = info && info->isSpecialization(); }
inline CObjectInfo *CTemplateInstance::Object () const
 { return _ObjectInfo; }
inline void CTemplateInstance::Object (CObjectInfo *info) 
 { _ObjectInfo = info; }

inline CTree *CTemplateInstance::PointOfInstantiation () const
 { return _PointOfInstantiation; }
inline CScopeInfo *CTemplateInstance::PointOfInstantiationScope () const
 { return _PointOfInstantiationScope; }
inline void CTemplateInstance::PointOfInstantiation (CTree *poi, CScopeInfo *pois)
 { _PointOfInstantiation = poi;
   _PointOfInstantiationScope = pois;
 }


} // namespace Puma

#endif /* __CTemplateInstance_h__ */
