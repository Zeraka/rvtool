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

#ifndef __CTemplateInfo_h__
#define __CTemplateInfo_h__

/** \file
 *  Semantic information about a template declaration. */

#include "Puma/CStructure.h"

namespace Puma {


class CT_TemplateDecl;
class CTemplateParamInfo;
class CT_ExprList;
class CT_TemplateName;
class CTree;


/** \class CTemplateInfo CTemplateInfo.h Puma/CTemplateInfo.h
 *  Semantic information about a template declaration.
 *  Contains information about the parameters, specializations, 
 *  and instances of the template. */
class CTemplateInfo : public CStructure {
  CObjectInfo *_ObjectInfo;
  CTemplateInfo *_Base;
  bool _isSpecial;
  CT_TemplateName *_SpecializationName;
  unsigned _Depth;

  Array<CTemplateParamInfo*> _Params;
  Array<CObjectInfo*> _Instances;
  Array<CTemplateInfo*> _Specializations;
  Array<CTree*> _TreeFragments;

public: 
  /** Constructor. */
  CTemplateInfo ();
  /** Destructor. Unlinks itself from the specializations
   *  and instances of this template. If the object type 
   *  is CObjectInfo::TEMPLATE_INFO, CObjectInfo::CleanUp() 
   *  is called. */
  ~CTemplateInfo ();
  
  /** Remove all links from other semantic objects to
   *  this semantic object. */
  void removeLinks ();

  /** Check if this is a function template definition. */
  bool isFunction () const;
  /** Check if this is a class template definition. */
  bool isClass () const;
  /** Check if this is the definition of a static 
   *  template class date member. */
  bool isAttribute () const;
  /** Check if this is a base template and not a 
   *  specialization of another template. */
  bool isBaseTemplate () const;
  /** Check if this is a specialization of another template. */
  bool isSpecialization () const;
  
  /** Increase the instantiation depth (for nested template
   *  instantiations). */
  void increaseDepth ();
  /** Decrease the instantiation depth (for nested template
   *  instantiations). */
  void decreaseDepth ();
  /** Get the current instantiation depth (for nested template
   *  instantiations). */
  unsigned Depth () const;

  /** Get the semantic information for the entity. */
  CObjectInfo *ObjectInfo () const;
  /** Get the syntax tree node for the template declaration. */
  CT_TemplateDecl *Tree () const;
  /** Get the base template if this is template specialization. */
  CTemplateInfo *BaseTemplate () const;
  /** Create a new template parameter.
   *  \param add If \e true, add the created parameter to the template. */
  CTemplateParamInfo *newTemplateParam (bool add = true);
  
  /** Get the number of template parameters. */
  unsigned Parameters () const;
  /** Get the n-th template parameter.
   *  \param n The index of the template parameter.
   *  \return The template parameter or NULL if \e n is invalid. */
  CTemplateParamInfo *Parameter (unsigned n) const;
  /** Get the default argument of the n-th template parameter.
   *  \param n The index of the template parameter.
   *  \return The default argument expression or NULL if 
   *          the parameter has no default argument. */
  CTree *DefaultArgument (unsigned n) const;

  /** Get the number of instances of this template. */
  unsigned Instances () const;
  /** Get the n-th instance of this template.
   *  \param n The index of the instance.
   *  \return The template instance or NULL if \e n is invalid. */
  CObjectInfo *Instance (unsigned n);
  /** Add the given instance to the template.
   *  \param info The semantic object of the template instance. */
  void addInstance (CObjectInfo *info);
  /** Remove the given template instance. 
   *  \param info The template instance. */
  void removeInstance (const CObjectInfo *info);
  
  /** Get the number of specializations of this template. */
  unsigned Specializations () const;
  /** Get the n-th specialization of this template.
   *  \param n The index of the specialization.
   *  \return The specialization or NULL if \e n is invalid. */
  CTemplateInfo *Specialization (unsigned n);
  /** Add the given specialization of the template.
   *  \param info The semantic object of the specialization. */
  void addSpecialization (CTemplateInfo *info);
  /** Remove the given specialization of the template.
   *  \param info The template specialization. */
  void removeSpecialization (const CTemplateInfo *info);
  /** Set whether this is a specialization of a template.
   *  \param v \e true if this is a specialization. */
  void isSpecialization (bool v);
  /** Set the template specialization name (template id).
   *  \param name The template specialization name. */
  void SpecializationName (CT_TemplateName *name);
  /** Get the template specialization name. 
   *  \return The template specialization name or NULL if not a specialization. */
  CT_TemplateName *SpecializationName () const;

  /** Remove and destroy the given template parameter.
   *  \param p The template parameter. */
  void deleteTemplateParam (const CTemplateParamInfo *p); 
  /** Set the semantic object of the entity (class, function, etc).
   *  \param info The semantic object. */
  void ObjectInfo (CObjectInfo *info);
  /** Set the base template if this is a template specialization.
   *  \param info The base template. */
  void BaseTemplate (CTemplateInfo *info);

  /** Check if the given template instances have the 
   *  same instantiation arguments. 
   *  \param i1 A template instance.
   *  \param i2 Another template instance.
   *  \param matchTemplateParams Template parameter compare mode. */
  static bool equalArguments (CTemplateInstance *i1, CTemplateInstance *i2, bool matchTemplateParams = false);

  /** Check if the given templates have equals parameters.
   *  \param t1 A template.
   *  \param t2 Another template. */
  static bool equalParameters (CTemplateInfo *t1, CTemplateInfo *t2);

  /** Add a tree fragment created during template instantiation. */
  void addTreeFragment (CTree* fragment);

private:
  void linkInstance (CObjectInfo *);
  CTemplateInfo *nextTemplate (CTemplateInfo *tinfo) const; 
};

inline CTemplateInfo::CTemplateInfo () :
  CStructure (CObjectInfo::TEMPLATE_INFO),
  _ObjectInfo ((CObjectInfo*)0),
  _Base ((CTemplateInfo*)0),
  _isSpecial (false),
  _SpecializationName ((CT_TemplateName*)0),
  _Depth (0),
  _Params (1, 2),
  _Instances (1, 5),
  _Specializations (1, 5),
  _TreeFragments (2, 5)
 {}

inline void CTemplateInfo::increaseDepth () 
 { _Depth++; }
inline void CTemplateInfo::decreaseDepth ()
 { if (_Depth) _Depth--; }
inline unsigned CTemplateInfo::Depth () const
 { return _Depth; }

inline unsigned CTemplateInfo::Instances () const
 { return _Instances.length (); }
inline CObjectInfo *CTemplateInfo::Instance (unsigned i)
 { return _Instances.lookup (i); }
inline void CTemplateInfo::addInstance (CObjectInfo *inst)
 { _Instances.append (inst); linkInstance (inst); }

inline unsigned CTemplateInfo::Specializations () const
 { return _Specializations.length (); }
inline CTemplateInfo *CTemplateInfo::Specialization (unsigned i)
 { return _Specializations.lookup (i); }
inline void CTemplateInfo::isSpecialization (bool v)
 { _isSpecial = v; }
inline CT_TemplateName *CTemplateInfo::SpecializationName () const 
 { return _SpecializationName; }
inline void CTemplateInfo::SpecializationName (CT_TemplateName *name)
 { _SpecializationName = name; }

inline unsigned CTemplateInfo::Parameters () const
 { return _Params.length (); }
inline CTemplateParamInfo *CTemplateInfo::Parameter (unsigned i) const
 { return _Params.lookup (i); }

inline CObjectInfo *CTemplateInfo::ObjectInfo () const
 { return _ObjectInfo; }
inline void CTemplateInfo::ObjectInfo (CObjectInfo *info)
 { if (info && (void*)info < (void*)0x10) *(int*)info = 0;
   _ObjectInfo = info; }
inline CTemplateInfo *CTemplateInfo::BaseTemplate () const
 { return _Base; }
 
inline bool CTemplateInfo::isFunction () const
 { return ObjectInfo () && ObjectInfo ()->FunctionInfo (); }
inline bool CTemplateInfo::isClass () const
 { return ObjectInfo () && ObjectInfo ()->ClassInfo (); }
inline bool CTemplateInfo::isAttribute () const
 { return ObjectInfo () && ObjectInfo ()->AttributeInfo (); }
inline bool CTemplateInfo::isBaseTemplate () const
 { return ! _isSpecial && ! (_Base); }
inline bool CTemplateInfo::isSpecialization () const
 { return _Base || _isSpecial; }

inline CT_TemplateDecl *CTemplateInfo::Tree () const 
 { return (CT_TemplateDecl*)CObjectInfo::Tree (); }


} // namespace Puma

#endif /* __CTemplateInfo_h__ */
