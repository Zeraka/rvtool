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

#ifndef __CFunctionInfo_h__
#define __CFunctionInfo_h__

/** \file 
 *  Semantic information about a function. */

#include "Puma/CStructure.h"
#include "Puma/Array.h"

namespace Puma {


class CArgumentInfo;
class CLabelInfo;
class CRecord;
class CNamespaceInfo;
class CTemplateInfo;
class CTypeFunction;


/** \class CFunctionInfo CFunctionInfo.h Puma/CFunctionInfo.h
 *  Semantic information about a function, method, overloaded
 *  operator, or user conversion function. */
class CFunctionInfo : public CStructure {
  Array<CArgumentInfo*> _Arguments;
  Array<CLabelInfo*> _Labels;

  CTypeInfo     *_ConversionType;
  CTemplateInfo *_TemplateInfo;
  bool           _isTemplate;

  enum MethodType {
    NONE,
    CONSTRUCTOR,
    DESTRUCTOR,
    OPERATOR,
    CONVERSION
  } _MethodType;

protected:
  /** Constructor.
   *  \param id The object type. */
  CFunctionInfo (ObjectId id);

public: 
  /** Constructor. */
  CFunctionInfo ();
  /** Destructor. If the object type is CObjectInfo::FUNCTION_INFO, 
   *  then CObjectInfo::CleanUp() is called. */
  ~CFunctionInfo ();
  
  /** Get the semantic object of the function definition. 
   *  \see CObjectInfo::DefObject() */
  CFunctionInfo *DefObject () const;
  /** Get the type information for the function. */
  CTypeFunction *TypeInfo () const;
  /** Get the conversion type if this is a conversion function.
   *  \return The conversion type or NULL if not a conversion function. */
  CTypeInfo *ConversionType () const;
  /** Get the return type of the function. */
  CTypeInfo *ReturnType () const;
  /** Get the template information if this is a function template.
   *  \return The template information or NULL if not a function template. */
  CTemplateInfo *TemplateInfo () const;
  /** Get the class or union if this is a method of a class or union.
   *  \return The class/union or NULL if not a method. */
  CRecord *Record () const;
  /** Get the namespace if this function is declared in a namespace.
   *  \return The namespace or NULL if not declared in a namespace. */
  CNamespaceInfo *Namespace () const;

  /** Get the number of jump labels defined in the function body. */
  unsigned Labels () const;
  /** Get the number of function parameters. */
  unsigned Arguments () const;
  /** Get the n-th jump label.
   *  \param n The index of the jump label.
   *  \return The label or NULL if \e n is invalid. */
  CLabelInfo *Label (unsigned n) const;
  /** Get the jump label with the given name.
   *  \param name The name of the jump label.
   *  \return The label or NULL if no such label. */
  CLabelInfo *Label (const char *name) const;
  /** Get the n-th function parameter.
   *  \param n The index of the function parameter. 
   *  \return The function parameter or NULL if \e n is invalid. */
  CArgumentInfo *Argument (unsigned n) const;
  /** Get the function parameter with the given name. 
   *  \param name The name of the function parameter.
   *  \return The function parameter or NULL if no such parameter. */
  CArgumentInfo *Argument (const char *name) const;
  /** Get the default argument of the n-th function parameter.
   *  \param n The index of the function parameter.
   *  \return The syntax tree node of the default argument or 
   *          NULL if the n-th parameter has no default argument. */
  CTree *DefaultArgument (unsigned n) const;

  /** Set the conversion type of a conversion function. 
   *  \param type The conversion type. */
  void ConversionType (CTypeInfo *type);
  /** Set the qualification scope of a class/union method or 
   *  function declared in a namespace.
   *  \param scope The qualification scope. */
  void QualifiedScope (CStructure *scope);
  /** Add a function parameter. 
   *  \param info The function parameter. */
  void addArgument (CArgumentInfo *info);
  /** Add a jump label.
   *  \param label The jump label. */
  void addLabel (CLabelInfo *label);
  /** Remove the given function parameter.
   *  \param info The function parameter. */
  void removeArgument (const CArgumentInfo *info);
  /** Remove the given jump label.
   *  \param label The jump label. */
  void removeLabel (const CLabelInfo *label);
  /** Set the template information of a function template.
   *  \param info The template information. */
  void TemplateInfo (CTemplateInfo *info);
  /** Set whether the function is a function template.
   *  \param v \c true if the function is a template. */
  void isTemplate (bool v);

  /** Create a new function parameter. The new function parameter
   *  is added to the function. */
  CArgumentInfo *newArgument ();
  /** Create a new jump label. The new jump label is added
   *  to the function. */
  CLabelInfo *newLabel ();
  /** Remove and destroy the given function parameter. 
   *  \param info The function parameter. */
  void deleteArgument (const CArgumentInfo *info);
  /** Remove and destroy the given jump label. 
   *  \param info The jump label. */
  void deleteLabel (const CLabelInfo *info);

  /** Check if the function accepts a variable argument list. */
  bool hasEllipsis () const;
  /** Check if the n-th function parameter has a default argument.
   *  \param n The index of the function parameter. */
  bool hasDefaultArgument (unsigned n) const;

  /** Check if this is a function definition. */
  bool isFctDef () const;
  /** Check if this is a method of a class or union. */
  bool isMethod () const;
  /** Check if this is a static method of a class or union. */
  bool isStaticMethod () const;
  /** Check if this is a function template. */
  bool isTemplate () const;
  /** Check if the function is defined. */
  bool isDefined () const;
  /** Check if this is a destructor. */
  bool isDestructor () const;
  /** Check if this is a constructor. */
  bool isConstructor () const;
  /** Check if this is an overloaded operator. */
  bool isOperator () const;
  /** Check if this is a conversion function. */
  bool isConversion () const;
  /** Check if the function is pure virtual. */
  bool isPureVirtual () const;
  /** Check if this is a default constructor. */
  bool isDefaultConstructor () const;
  /** Check if this is a copy constructor. */
  bool isCopyConstructor () const;
  /** Check if this is a copy assignment operator. */
  bool isCopyAssignOperator () const;

  /** Set whether the function is a destructor. 
   *  \param v \e true if the function is a destructor. */
  void isDestructor (bool v);
  /** Set whether the function is a constructor. 
   *  \param v \e true if the function is a constructor. */
  void isConstructor (bool v);
  /** Set whether the function is an overloaded operator. 
   *  \param v \e true if the function is an overloaded operator. */
  void isOperator (bool v);
  /** Set whether the function is a conversion function. 
   *  \param v \e true if the function is a conversion function. */
  void isConversion (bool v);

  /** Check if the given function has the same name and
   *  parameter types as this function.
   *  \param fi The function to compare with. */
  bool hasSameNameAndArgs (const CFunctionInfo *fi) const;

  /** Check if the function is a non-static member function and if
   *  in any of the base classes there is a function definition of a
   *  virtual function with the same name and argument types. */
  bool overridesVirtual () const;
};

inline CFunctionInfo::CFunctionInfo (ObjectId id) :
  CStructure (id),
  _Arguments (1, 2),
  _Labels (1, 2),
  _ConversionType ((CTypeInfo*)0),
  _TemplateInfo ((CTemplateInfo*)0),
  _isTemplate (false),
  _MethodType (NONE)
 {}
inline CFunctionInfo::CFunctionInfo () :
  CStructure (CObjectInfo::FUNCTION_INFO),
  _Arguments (1, 2),
  _Labels (1, 2),
  _ConversionType ((CTypeInfo*)0),
  _TemplateInfo ((CTemplateInfo*)0),
  _isTemplate (false),
  _MethodType (NONE)
 {}

inline CTypeInfo *CFunctionInfo::ConversionType () const
 { return _ConversionType; }
inline void CFunctionInfo::ConversionType (CTypeInfo *t)
 { _ConversionType = t; }
 
inline CTypeFunction *CFunctionInfo::TypeInfo () const
 { return (CTypeFunction*)CObjectInfo::TypeInfo (); }

inline unsigned CFunctionInfo::Arguments () const
 { return _Arguments.length (); }
inline CArgumentInfo *CFunctionInfo::Argument (unsigned n) const
 { return _Arguments.lookup (n); }
inline unsigned CFunctionInfo::Labels () const
 { return _Labels.length (); }
inline CLabelInfo *CFunctionInfo::Label (unsigned n) const
 { return _Labels.lookup (n); }

inline void CFunctionInfo::QualifiedScope (CStructure *s)
 { _QualScope = s; }
 
inline bool CFunctionInfo::isDestructor () const
 { return _MethodType == DESTRUCTOR; }
inline bool CFunctionInfo::isConstructor () const
 { return _MethodType == CONSTRUCTOR; }
inline bool CFunctionInfo::isOperator () const
 { return _MethodType == OPERATOR; }
inline bool CFunctionInfo::isConversion () const
 { return _MethodType == CONVERSION; }
inline bool CFunctionInfo::isMethod () const 
 { return (Record ()); }
inline bool CFunctionInfo::isStaticMethod () const 
 { return (isMethod () && isStatic ()); }

inline void CFunctionInfo::isDestructor (bool v)
 { if (v) _MethodType = DESTRUCTOR; }
inline void CFunctionInfo::isConstructor (bool v)
 { if (v) _MethodType = CONSTRUCTOR; }
inline void CFunctionInfo::isOperator (bool v)
 { if (v) _MethodType = OPERATOR; }
inline void CFunctionInfo::isConversion (bool v)
 { if (v) _MethodType = CONVERSION; }

inline bool CFunctionInfo::isTemplate () const 
 { return _isTemplate; }
inline void CFunctionInfo::isTemplate (bool v)
 { _isTemplate = v; }

inline CTemplateInfo *CFunctionInfo::TemplateInfo () const
 { return _TemplateInfo; }
inline void CFunctionInfo::TemplateInfo (CTemplateInfo *info) 
 { _TemplateInfo = info; }

//inline bool CFunctionInfo::isRedefined () const 
// { return BaseObject () && ! isInherited (); }


} // namespace Puma

#endif /* __CFunctionInfo_h__ */
