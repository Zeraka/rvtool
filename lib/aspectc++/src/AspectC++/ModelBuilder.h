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

#ifndef __ModelBuilder_h__
#define __ModelBuilder_h__

// class alias for frontend-specific model builder implementation

#include "ACModel/Elements.h"
#include "TransformInfo.h"

/* helper classes to merge the TI_ classes into the ACM_ classes, i.e. to
 * extend the join point model elements by transformation infos
 */

class TU_CodeAdvice : public ACM_CodeAdvice, public TI_CodeAdvice {
public:
  TU_CodeAdvice () : ACM_CodeAdvice () { transform_info (this); }
  ~TU_CodeAdvice () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Type : public ACM_Type, public TI_Type {
public:
  TU_Type () : ACM_Type () { transform_info (this); }
  ~TU_Type () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Arg : public ACM_Arg, public TI_Arg {
public:
  TU_Arg () : ACM_Arg () { transform_info (this); }
  ~TU_Arg () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Namespace : public ACM_Namespace, public TI_Namespace {
public:
  TU_Namespace () : ACM_Namespace () { transform_info (this); }
  ~TU_Namespace () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Class : public ACM_Class, public TI_Class {
public:
  TU_Class () : ACM_Class () { transform_info (this); }
  ~TU_Class () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Aspect : public ACM_Aspect, public TI_Aspect {
public:
  TU_Aspect () : ACM_Aspect () { transform_info (this); }
  ~TU_Aspect () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Function : public ACM_Function, public TI_Function {
public:
  TU_Function () : ACM_Function () { transform_info (this); }
  ~TU_Function () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Variable : public ACM_Variable, public TI_Variable {
public:
  TU_Variable () : ACM_Variable () { transform_info (this); }
  ~TU_Variable () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_AdviceCode : public ACM_AdviceCode, public TI_AdviceCode {
public:
  TU_AdviceCode () : ACM_AdviceCode () { transform_info (this); }
  ~TU_AdviceCode () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_ClassSlice : public ACM_ClassSlice, public TI_ClassSlice {
public:
  TU_ClassSlice () : ACM_ClassSlice () { transform_info (this); }
  ~TU_ClassSlice () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_MethodCall : public ACM_Call, public TI_MethodCall {
public:
  TU_MethodCall () { transform_info (this); }
  ~TU_MethodCall () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

// only define the classes for new feature for clang frontend, as there is no implementation for puma
#ifdef FRONTEND_CLANG

class TU_Get : public ACM_Get, public TI_Get {
public:
  TU_Get() { transform_info( this ); }
  ~TU_Get() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

class TU_Set : public ACM_Set, public TI_Set {
public:
  TU_Set() { transform_info( this ); }
  ~TU_Set() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

class TU_Ref : public ACM_Ref, public TI_Ref {
public:
  TU_Ref() { transform_info( this ); }
  ~TU_Ref() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

class TU_GetRef : public ACM_GetRef, public TI_GetRef {
public:
  TU_GetRef() { transform_info( this ); }
  ~TU_GetRef() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

class TU_SetRef : public ACM_SetRef, public TI_SetRef {
public:
  TU_SetRef() { transform_info( this ); }
  ~TU_SetRef() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

class TU_Builtin : public ACM_Builtin, public TI_Builtin {
public:
  TU_Builtin() { transform_info( this ); }
  ~TU_Builtin() { transform_info( 0 ); }
  virtual ModelNode &jpl() { return *this; }
};

#endif

class TU_Method : public ACM_Execution, public TI_Method {
public:
  TU_Method () { transform_info (this); }
  ~TU_Method () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Construction : public ACM_Construction, public TI_Construction {
public:
  TU_Construction () { transform_info (this); }
  ~TU_Construction () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Destruction : public ACM_Destruction, public TI_Destruction {
public:
  TU_Destruction () { transform_info (this); }
  ~TU_Destruction () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Introduction : public ACM_Introduction, public TI_Introduction {
public:
  TU_Introduction () { transform_info (this); }
  ~TU_Introduction () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Order : public ACM_Order, public TI_Order {
public:
  TU_Order () { transform_info (this); }
  ~TU_Order () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Pointcut : public ACM_Pointcut, public TI_Pointcut {
public:
  TU_Pointcut () { transform_info (this); }
  ~TU_Pointcut () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

class TU_Attribute : public ACM_Attribute, public TI_Attribute {
public:
  TU_Attribute () { transform_info (this); }
  ~TU_Attribute () { transform_info (0); }
  virtual ModelNode &jpl () { return *this; }
};

#ifdef FRONTEND_PUMA

#include "PumaModelBuilder.h"
class ModelBuilder : public PumaModelBuilder {
public:
  ModelBuilder (Puma::VerboseMgr &vm, ACErrorStream &err, ACConfig &conf,
      ACProject &project) : PumaModelBuilder (vm, err, conf, project) {}
};

#else // FRONTEND_CLANG

#include "ClangModelBuilder.h"
class ModelBuilder : public ClangModelBuilder {
public:
  ModelBuilder (Puma::VerboseMgr &vm, ACErrorStream &err, ACConfig &conf,
      ACProject &project) : ClangModelBuilder (vm, err, conf, project) {}
};

#endif

#endif // __ModelBuilder_h__
