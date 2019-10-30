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

#ifndef __CTemplateParamInfo_h__
#define __CTemplateParamInfo_h__

/** \file
 *  Semantic information about a template parameter. */
 
#include "Puma/CObjectInfo.h"

namespace Puma {


class CT_TemplateParamDecl;
class CTypeTemplateParam;
class CTemplateInstance;
class CTemplateInfo;
class CTree;


/** \class CTemplateParamInfo CTemplateParamInfo.h Puma/CTemplateParamInfo.h
 *  Semantic information about a template parameter. There are 
 *  three kinds of template parameter: type, non-type, and template
 *  template parameter. 
 *
 *  \code
 * // T is a type template parameter
 * // I is a non-type template parameter
 * // TT is a template template parameter
 * template<class T, int I, template<typename,int> class TT> 
 * class X {
 *   TT<T,I> x;
 * };
 *  \endcode */
class CTemplateParamInfo : public CObjectInfo {
  CTemplateInfo *_TemplateInfo;
  CTemplateInfo *_TemplateTemplate;
  CTemplateInstance *_InstanceInfo;
  CTypeInfo *_ValueType;
  bool _TypeParam;

public: 
  /** Constructor. */
  CTemplateParamInfo ();
  /** Destructor. If the object type is CObjectInfo::TEMPLATE_PARAM_INFO, 
   *  then CObjectInfo::CleanUp() is called and the template instance
   *  information object of a template template parameter is destroyed. */
  ~CTemplateParamInfo ();

  /** Check if this template parameter matches the given.
   *  Must be same kind of parameter, same type and value. */
  bool match(const CTemplateParamInfo& tp) const;

  /** Check if this is a template template parameter. */
  bool isTemplate () const;
  /** Check if this is a type template parameter. */
  bool isTypeParam () const;
  /** Get the template this parameter belongs to. */
  CTemplateInfo *TemplateInfo () const;
  /** Get the template information of a template template parameter. */
  CTemplateInfo *TemplateTemplate () const;
  /** Get the syntax tree node representing the template parameter. */
  CT_TemplateParamDecl *Tree () const;
  /** Get the default argument of the template parameter.
   *  \return The default argument expression or NULL if no default argument. */
  CTree *DefaultArgument () const;
  /** Get the template parameter type. */
  CTypeTemplateParam *TypeInfo () const; 
  /** Get the value type of a type template parameter. */
  CTypeInfo *ValueType () const;
  /** Get the template instance information for an instantiated 
   *  template template parameter. */
  CTemplateInstance *TemplateInstance () const;
  /** Get the position/index of this parameter in the template parameter list. */
  int getPosition () const;

  /** Set whether this is a type template parameter. 
   *  \param v True for yes, false for no. */
  void isTypeParam (bool v);
  /** Set the value type of a type template parameter. 
   *  \param type The value type. */
  void ValueType (CTypeInfo *type);
  /** Set the template parameter list information for 
   *  a template template parameter. 
   *  \param info The template parameter list information. */
  void TemplateInfo (CTemplateInfo *info);
  /** Set the template information for a template template parameter. 
   *  \param info The template information. */
  void TemplateTemplate (CTemplateInfo *info);
  /** Set the template instance information for an instantiated template 
   *  template parameter. 
   *  \param inst The template instance. */
  void TemplateInstance (CTemplateInstance *inst);
};

inline CTemplateParamInfo::CTemplateParamInfo () :
  CObjectInfo (CObjectInfo::TEMPLATE_PARAM_INFO),
  _TemplateInfo ((CTemplateInfo*)0),
  _TemplateTemplate ((CTemplateInfo*)0),
  _InstanceInfo ((CTemplateInstance*)0),
  _ValueType ((CTypeInfo*)0),
  _TypeParam (false)
 {}

inline void CTemplateParamInfo::isTypeParam (bool tp)
 { _TypeParam = tp; }
inline bool CTemplateParamInfo::isTypeParam () const 
 { return _TypeParam; }

inline bool CTemplateParamInfo::isTemplate () const 
 { return (_TemplateTemplate); }
 
inline void CTemplateParamInfo::ValueType (CTypeInfo *vt) 
 { _ValueType = vt; }
inline CTypeInfo *CTemplateParamInfo::ValueType () const 
 { return _ValueType; }

inline CTemplateInfo *CTemplateParamInfo::TemplateInfo () const
 { return _TemplateInfo; }
inline CTemplateInfo *CTemplateParamInfo::TemplateTemplate () const
 { return _TemplateTemplate; }
inline void CTemplateParamInfo::TemplateInfo (CTemplateInfo *info) 
 { _TemplateInfo = info; }
inline void CTemplateParamInfo::TemplateTemplate (CTemplateInfo *info) 
 { _TemplateTemplate = info; }
inline void CTemplateParamInfo::TemplateInstance (CTemplateInstance *info)
 { _InstanceInfo = info; }
 
inline CT_TemplateParamDecl *CTemplateParamInfo::Tree () const 
 { return (CT_TemplateParamDecl*)CObjectInfo::Tree (); }

inline CTypeTemplateParam *CTemplateParamInfo::TypeInfo () const
 { return (CTypeTemplateParam*)CObjectInfo::TypeInfo (); }

inline CTemplateInstance *CTemplateParamInfo::TemplateInstance () const
 { return _InstanceInfo; }


} // namespace Puma

#endif /* __CTemplateParamInfo_h__ */
