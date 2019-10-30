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

#ifndef __CTypeTemplateParam_h__
#define __CTypeTemplateParam_h__

/** \file
 *  Type of a template parameter. */
 
#ifndef __CTypeInfo_h__
#warning !!!      DO NOT INCLUDE THIS FILE      !!!  
#warning !!! INCLUDE FILE "CTypeInfo.h" INSTEAD !!!  
#endif

namespace Puma {


class CTemplateParamInfo;


/** \class CTypeTemplateParam CTypeTemplateParam.inc Puma/CTypeInfo.h
 * Type of a template parameter.
 * \ingroup types */
class CTypeTemplateParam : public CTypeInfo {
  CTemplateParamInfo *_TemplateParam;

public:
  /** Constructor. The type ID is CTypeInfo::TYPE_TEMPLATE_PARAM.
   *  \param info The template parameter. */
  CTypeTemplateParam (CTemplateParamInfo *info);
  /** Destructor. */
  ~CTypeTemplateParam ();

  /** Get the template parameter information. */
  CTemplateParamInfo *TemplateParamInfo () const;
  /** Set the template parameter information. 
   *  \param info The template parameter. */
  void TemplateParamInfo (CTemplateParamInfo *info);
  /** Check if this is the type of a type template parameter. */
  bool isType () const;
  /** Check if this is the type of a non-type template parameter. */
  bool isNonType () const;
};

inline CTypeTemplateParam::CTypeTemplateParam (CTemplateParamInfo *t) :
  CTypeInfo (&CTYPE_EMPTY, CTypeInfo::TYPE_TEMPLATE_PARAM),
  _TemplateParam (t)
 {}
inline CTypeTemplateParam::~CTypeTemplateParam ()
 {}
 
inline CTemplateParamInfo *CTypeTemplateParam::TemplateParamInfo () const 
 { return _TemplateParam; }
 
inline bool CTypeTemplateParam::isNonType () const
 { return (! isType ()); }

inline void CTypeTemplateParam::TemplateParamInfo (CTemplateParamInfo *info)
 { _TemplateParam = info; }

} // namespace Puma

#endif /* __CTypeTemplateParam_h__ */
