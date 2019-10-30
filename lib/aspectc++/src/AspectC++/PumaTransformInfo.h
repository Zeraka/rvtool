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

#ifndef __PumaTransformInfo_h__
#define __PumaTransformInfo__

#include "ACModel/Elements.h"
#include "ThisJoinPoint.h"
#include "PointCutExpr.h"
#include "SyntacticContext.h"
#include "CFlow.h"
#include "WeaverBase.h"


#include "Puma/CSemDatabase.h"
#include "Puma/CNamespaceInfo.h"
#include "Puma/DeducedArgument.h"
#include "Puma/CTemplateInstance.h"
#include "Puma/CFctInstance.h"
#include "Puma/MacroUnit.h"
#include "Puma/FileUnit.h"

class TI_CodeAdvice : public ModelTransformInfo {
  const Condition *_condition;
  AdviceInfo *_advice_info;
public:
  void set_condition (const Condition *c) { _condition = c; }
  const Condition *get_condition () const { return _condition; }
  void set_advice_info (AdviceInfo *ai) { _advice_info = ai; }
  AdviceInfo *get_advice_info () const { return _advice_info; }

  static TI_CodeAdvice *of (const ACM_CodeAdvice &loc) {
    return (TI_CodeAdvice*)loc.transform_info ();
  }
};

class TransformInfo : public ModelTransformInfo {
public:
  virtual ModelNode &jpl () = 0;
  virtual Puma::CTree *tree () const = 0;
  virtual Puma::CObjectInfo *obj_info () const = 0;
  // associated unique parser object (e.g. for name mangling)
  virtual Puma::CObjectInfo *assoc_obj () const = 0;
  // find the unit (non-macro) in which a join point is located
  Puma::Unit *unit () const {
    Puma::CTree *node = tree ();
    if (!node)
      return 0;
    Puma::Token *token = node->token ();
    Puma::Unit *unit   = (Puma::Unit*)token->belonging_to ();
    while (unit && unit->isMacroExp ()) {
      unit = ((Puma::MacroUnit*)unit)->CallingUnit ();
    }
    return unit;
  }

  static inline const TransformInfo *of (const ModelNode &loc);
  static inline Puma::CTree *tree (const ModelNode &loc);
  static inline Puma::Unit *unit (const ModelNode &loc);
  static inline Puma::Location location (const ModelNode &loc);

  // helper function: check if a function is a method (needs a 'this' pointer)
  static bool needs_this (Puma::CFunctionInfo *func) {
    if (func->isMethod () && !func->isStaticMethod ()) {
      return !(func->isOperator () &&
        (strcmp (func->Name (), "operator new") == 0 ||
         strcmp (func->Name (), "operator new[]") == 0 ||
         strcmp (func->Name (), "operator delete") == 0 ||
         strcmp (func->Name (), "operator delete[]") == 0));
    }
    return false;
  }
};

inline const TransformInfo *TransformInfo::of (const ModelNode &loc) {
  return loc.transform_info () ? (TransformInfo*)loc.transform_info () : 0;
}
inline Puma::CTree *TransformInfo::tree (const ModelNode &loc) {
  return loc.transform_info () ?
    ((TransformInfo*)loc.transform_info ())->tree () : 0;
}
inline Puma::Unit *TransformInfo::unit (const ModelNode &loc) {
  return loc.transform_info () ?
    ((TransformInfo*)loc.transform_info ())->unit () : 0;
}
inline Puma::Location TransformInfo::location (const ModelNode &loc) {
  return loc.transform_info () ?
    tree (loc)->token ()->location () : Puma::Location ();
}

class TI_Namespace : public TransformInfo {
  // pointer to the Puma namespace object (for transformation)
  Puma::CNamespaceInfo *_namespace_obj;
public:
  TI_Namespace () : _namespace_obj (0) {}
    
  void namespace_info (Puma::CNamespaceInfo *n) { _namespace_obj = n; }
  Puma::CNamespaceInfo *namespace_info () const { return _namespace_obj; }
  virtual Puma::CObjectInfo *obj_info () const { return _namespace_obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _namespace_obj; }
  virtual Puma::CTree *tree () const { return _namespace_obj->Tree (); }

  static string name (Puma::CNamespaceInfo *n) {
    if (n->GlobalScope ())
      return "::";
    else
      return n->QualName (false, false, true); // the last 'true' means:
                                               // we want to see '<unnamed>
  } 
};

class TI_Class : public TransformInfo {
  Puma::CClassInfo *_class_obj;

  // internal function that checks whether a given scope is located within
  // a template instance.
  // TODO: this is a duplicate of a ModelBuilder function!
  static bool is_in_template_instance (Puma::CScopeInfo *scope) {
    if (scope->isTemplateInstance ())
      return true;
    if (scope->QualifiedScope ())
      return is_in_template_instance (scope->QualifiedScope ());
    if (scope->Parent () != scope)
      return is_in_template_instance (scope->Parent ());
    return false;
  }

public:
  TI_Class () : _class_obj (0) {}

  bool valid () const { return _class_obj != 0; } // probably the *definition* is external; only phase 1 found a declaration

  void class_info (Puma::CClassInfo *c) {
    _class_obj = c->DefObject ();
  }
  Puma::CClassInfo *class_info () const { return _class_obj; }

  SyntacticContext get_def_context () const { return SyntacticContext (_class_obj); }

  void get_member_contexts (list<PumaSyntacticContext> &member_contexts) const {

    for (unsigned a = 0; a < _class_obj->Attributes (); a++) {
      Puma::CAttributeInfo *attr = _class_obj->Attribute (a);

      // Only certain members are delivered.
      // If this is not the right choice for all use case, add filter flags to
      // the argument list of this function
      if (attr->isStatic () || attr->isAnonymous () || attr->EnumeratorInfo ())
        continue;

      member_contexts.push_back (PumaSyntacticContext (attr));
    }
  }

  enum SMKind { CONSTRUCTOR, COPY_CONSTRUCTOR, DESTRUCTOR };
  bool may_have_implicit (SMKind kind) {
    return may_have_implicit (kind, _class_obj);
  }

  bool may_have_implicit (SMKind kind, Puma::CClassInfo *cls) {

    for (unsigned f = 0; f < cls->Functions (); f++) {
      Puma::CFunctionInfo *curr = cls->Function (f);
      // TODO: checking for one argument is not sufficient to detect a copy constructor!
      if ((curr->isConstructor () && curr->Arguments () == 0 && kind == CONSTRUCTOR) ||
          (curr->isConstructor () && curr->Arguments () == 1 && kind == COPY_CONSTRUCTOR) ||
          (curr->isDestructor () && kind == DESTRUCTOR)) {
        if (curr->Protection () == Puma::CProtection::PROT_PRIVATE)
          return false; // function is private -> problem!
      }
    }

    // check if any base class has a private constructor/copy constructor/destr.
    for (unsigned b = 0; b < cls->BaseClasses (); b++) {
      Puma::CClassInfo *base = cls->BaseClass (b)->Class ();
      if (!may_have_implicit(kind, base))
        return false;
    }

    // check the types of non-static attributes (or array thereof) for private constructor/copy constructor/destr.
    for (unsigned a = 0; a < cls->Attributes (); a++) {
      Puma::CObjectInfo *attr = cls->Attribute(a);
      if (attr->isStatic() || attr->isAnonymous())
        continue;
      Puma::CTypeInfo *type = attr->TypeInfo();
      while (type->isArray())
        type = type->BaseType();
      Puma::CRecord *base = type->Record();
      if (!base ||             // no problem if the attribute is no class/struct/union instance
          base->isAnonymous()) // anonymous structs/classes/unions cannot have a private constrcutor/destructor
        continue;
      if (!base->ClassInfo ())
        continue;
      if (!may_have_implicit(kind, base->ClassInfo ()))
        return false;
    }

    return true; // seems that there is no problem
  }

  virtual Puma::CObjectInfo *obj_info () const { return _class_obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _class_obj; }
  virtual Puma::CTree *tree () const { return _class_obj->Tree (); }
  
  // return the position behind the opening bracket of the class body
  const WeavePos &body_start_pos (WeaverBase &wb) const {
    return wb.weave_pos(((Puma::CT_ClassDef*)_class_obj->Tree ())->Members ()->token (),
        WeavePos::WP_AFTER);
  }

  // return the position in front of the closing bracket of the class body
  const WeavePos &body_end_pos (WeaverBase &wb) const {
    return wb.weave_pos(((Puma::CT_ClassDef*)_class_obj->Tree ())->Members ()->end_token (),
        WeavePos::WP_BEFORE);
  }

  // return the position of the first token of the class definition
  const WeavePos &objdecl_start_pos (WeaverBase &wb) const {
    assert (valid ());
    Puma::CT_ClassDef *cd = (Puma::CT_ClassDef*)_class_obj->Tree();
    return wb.weave_pos (cd->ObjDecl ()->token (), WeavePos::WP_BEFORE);
  }

  // return the position after the ";" of the class definition
  const WeavePos &objdecl_end_pos (WeaverBase &wb) const {
    Puma::CT_ClassDef *cd = (Puma::CT_ClassDef*)_class_obj->Tree();
    return wb.weave_pos (cd->ObjDecl ()->end_token (), WeavePos::WP_AFTER);
  }

  // check whether this is a class or struct
  bool is_class () const { return _class_obj->isClass(); }

  // check whether this is a struct (more restrictive than 'is_class')
  bool is_struct () const { return _class_obj->isStruct(); }

  // check whether the class is defined (=has a body) and not only declared
  bool is_defined () const { return _class_obj->isDefined(); }

  // checks whether this class is a template *instance*
  bool is_template_instance () const { return _class_obj->isTemplateInstance(); }

  // check whether the class is defined in a extern "C" block
  bool is_extern_c () const {
    if (!is_defined())
      return false;
    Puma::CT_ClassDef *cd = (Puma::CT_ClassDef*)_class_obj->DefObject ()->Tree ();
    // TODO: ugly workaround needed to check whether a class is defined in
    // an extern "C" block -> extended Puma, SemDB doesn't contain this info!
    for (Puma::CTree *curr = cd; curr; curr = curr->Parent())
      if (curr->NodeName() == Puma::CT_LinkageSpec::NodeId())
        return true;
    return false;
  }

  // checks whther the class is defined within a template instance
  bool is_in_template_instance () const {
    return is_in_template_instance(_class_obj);
  }

  static string name (Puma::CClassInfo *c) {
    ostringstream class_name;
    c->TypeInfo ()->TypeText (class_name, 0, false, false, false, true);
    return class_name.str ().c_str ();
  }

  static TI_Class *of (const ACM_Class &loc) {
    return (TI_Class*)loc.transform_info ();
  }
};

class TI_Aspect : public TI_Class {
public:

  Puma::CFunctionInfo *aspectof () const {
    // TODO: make this better!
    Puma::CFunctionInfo *aof = class_info()->Function ("aspectof");
    if (!aof)
      aof = class_info()->Function ("aspectOf");
    return aof ? aof->DefObject () : (Puma::CFunctionInfo*)0;
  }

  static const TI_Aspect *of (const ACM_Aspect &loc) {
    return (TI_Aspect*)loc.transform_info ();
  }

};

class TI_Function : public TransformInfo {
  Puma::CFunctionInfo *_func_obj;
  mutable vector<SyntacticContext> _contexts;

public:
  TI_Function () : _func_obj (0) {}
    
  void func_info (Puma::CFunctionInfo *fi) { _func_obj = fi->DefObject (); }
  const vector<SyntacticContext> &syntactic_contexts () const {
    if (_contexts.size() == 0) {
      // Store a pointer to each declaration and the definition (if one exists)
      Puma::CFunctionInfo *curr = _func_obj;
      do {
        _contexts.push_back (SyntacticContext (curr));
        curr = curr->NextObject ()->FunctionInfo ();
      }
      while (curr != _func_obj);
    }
    return _contexts;
  }

  Puma::CFunctionInfo *func_info () const { return _func_obj; }
  virtual Puma::CObjectInfo *obj_info () const { return _func_obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _func_obj; }
  virtual Puma::CTree *tree () const { return _func_obj->Tree (); }

  static string signature (Puma::CFunctionInfo *func_info) {
    ostringstream name;
    // the unique name in a scope start with the function name
    name << func_info->Name ();
      
    // add the template arguments if it is a function template instance
    Puma::CTemplateInstance *instance = func_info->TemplateInstance ();
    if (instance) {
      name << "<";
      for (unsigned a = 0; a < instance->DeducedArgs (); a++) {
        if (a > 0) name << ",";
        Puma::DeducedArgument *arg = instance->DeducedArg (a);
        if (arg->Type ())
          name << *arg->Type ();
        else if (arg->Value ()) {
          if (arg->Value ()->isSigned ())
            // TODO: cast needed here as stdc++ lib can't print __int128! Problems!
            name << (long long)arg->Value ()->convert_to_int ();
          else if (arg->Value ()->isUnsigned ())
            name << (unsigned long long)arg->Value ()->convert_to_uint ();
          else if (arg->Value ()->isFloat ())
            name << arg->Value ()->convert_to_float ();
        }
        else
          name << "*invalid template arg*";
      }
      string name_str = name.str ();
      if (name_str[name_str.length () - 1] == '>')
        name << " ";
      name << ">";
    }

    // add the argument types
    Puma::CTypeFunction *this_type = func_info->TypeInfo ();
    name << "(";
    Puma::CTypeList *argtypes = this_type->ArgTypes ();
    for (unsigned i = 0; i < argtypes->Entries (); i++) {
      if (i > 0) name << ",";
      Puma::CTypeInfo *argtype = argtypes->Entry (i);
      argtype->TypeText (name, "", false, false, false, true);
    }
    name << ")";

    // add CV qualifiers
    if (this_type->isConst ())
      name << " const";
    if (this_type->isVolatile ())
      name << " volatile";

    return name.str ();
  }

  bool is_const () const {
    return _func_obj->TypeInfo()->isConst();
  }

  bool is_conversion_operator () const {
    return _func_obj->isConversion();
  }


  static const TI_Function *of (const ACM_Function &loc) {
    return (TI_Function*)loc.transform_info ();
  }
};

class TI_Variable : public TransformInfo {
  Puma::CObjectInfo *_obj;
public:
  TI_Variable () : _obj (0) {}

  void obj_info (Puma::CObjectInfo *oi) { _obj = oi->DefObject (); }
  virtual Puma::CObjectInfo *obj_info () const { return _obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _obj; }
  virtual Puma::CTree *tree () const { return _obj->Tree (); }
public:
  static string name (Puma::CObjectInfo *obj_info) {
    return obj_info->Name ().c_str ();
  }
  static const TI_Variable *of (const ACM_Variable &loc) {
    return (TI_Variable*)loc.transform_info ();
  }
};

class TI_Type : public TransformInfo {
  Puma::CTypeInfo *_type_info;
public:
  TI_Type () : _type_info (0) {}
  
  void type_info (Puma::CTypeInfo *ti) { _type_info = ti; }
  Puma::CTypeInfo *type_info () const { return _type_info; }

  virtual Puma::CObjectInfo *obj_info () const { return 0; }
  virtual Puma::CObjectInfo *assoc_obj () const { return 0; }
  virtual Puma::CTree *tree () const { return 0; }

  static string name (Puma::CTypeInfo *type_info) {
    ostringstream out;
    type_info->TypeText(out,"?", false, false, false, true);
    string result = out.str();
    int last = result.length() - 1;
    if (result[last] == '?') last--;
    while(result[last] == ' ') last--;
    return result.substr(0, last + 1);
  }

  static const TI_Type *of (const ACM_Type &loc) {
    return (TI_Type*)loc.transform_info ();
  }
};

class TI_Arg : public TransformInfo {
  Puma::CTypeInfo *_type_info;
public:
  TI_Arg () : _type_info (0) {}

  void type_info (Puma::CTypeInfo *ti) { _type_info = ti; }
  Puma::CTypeInfo *type_info () const { return _type_info; }

  virtual Puma::CObjectInfo *obj_info () const { return 0; }
  virtual Puma::CObjectInfo *assoc_obj () const { return 0; }
  virtual Puma::CTree *tree () const { return 0; }

  static string signature (Puma::CTypeInfo *type_info, const string &name) {
    ostringstream out;
    type_info->TypeText(out, name.c_str(), false, false, false, true);
    return out.str();
  }

  static const TI_Arg *of (const ACM_Arg &loc) {
    return (TI_Arg*)loc.transform_info ();
  }
};

class TI_Code : public TransformInfo {

  CFlowList _triggers;

public:

  // consider a necessary cflow trigger at this join point
  bool consider (const CFlow &cflow) {
    _triggers.push_back (cflow);
    return true;
  }

  // return the list of cflows that must be entered/left here
  const CFlowList &cflows () const { return _triggers; }

  // that types (for the JoinPoint-API)
  virtual Puma::CTypeInfo *that_type () const = 0;
  virtual std::string that_type_string () const { return "void"; }

  // target type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *target_type () const = 0;
  virtual std::string target_type_string () const { return "void"; }
  
  // helper functions for derived classes
  static Puma::CTypeInfo *get_that_type (Puma::CObjectInfo *obj) {
    Puma::CFunctionInfo *func = obj->FunctionInfo ();
    if (func) {
      Puma::CObjectInfo *this_obj = func->Attribute ("this");
      if (this_obj) {
        Puma::CTypeInfo *type = this_obj->TypeInfo ();
        // find the type which is referenced by 'this'
        while (type && !type->isRecord ())
          type = type->BaseType ();
        assert (type);
        return type;
      }
    }
    if (obj->ClassScope ())
      return obj->ClassScope ()->TypeInfo ();
    return &Puma::CTYPE_VOID;
  }

  static TI_Code *of (const ACM_Code &loc) {
    return (TI_Code*)loc.transform_info ();
  }

};

class TI_Method : public TI_Code {
  Puma::CFunctionInfo *_func_info;
public:
  TI_Method () : _func_info (0) {}

  void func_info (Puma::CFunctionInfo *f) { _func_info = f; }
  virtual Puma::CObjectInfo *obj_info () const { return _func_info; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _func_info; }
  virtual Puma::CTree *tree () const { return _func_info->Tree (); }

  // that type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *that_type () const { return get_that_type (_func_info); }

  // target type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *target_type () const {
    Puma::CRecord *scope = _func_info->ClassScope ();
    return scope ? scope->TypeInfo () : &Puma::CTYPE_VOID;
  }
};

class TI_MethodCall : public TI_Code {
  Puma::CFunctionInfo *called_func;
  Puma::CObjectInfo *caller_obj;
  Puma::CT_Call *node;
  SyntacticContext _call_context;
  SyntacticContext _dest_func_context;
  Puma::CT_Expression *_target_expr;
  bool _target_is_ptr;
public:
  TI_MethodCall () : called_func (0), caller_obj (0), node (0),
    _target_expr (0), _target_is_ptr (false) {}

  void called (Puma::CFunctionInfo *c) { called_func = c; _dest_func_context.set_object(c); }
//  CFunctionInfo *called () const { return called_func; }
  void caller (Puma::CObjectInfo *c) { caller_obj = c; _call_context.set_object(c); }
  Puma::CFunctionInfo *caller () {
    return caller_obj ? caller_obj->FunctionInfo () : 0;
  }
  void tree (Puma::CT_Call *n) { node = n; if (n) _target_expr = target_expr (_target_is_ptr); }
  bool has_target_expr () const { return _target_expr; }
  bool target_is_ptr () const { return _target_is_ptr; }
  virtual Puma::CTree *tree () const { return node; }
  virtual Puma::CObjectInfo *obj_info () const { return called_func; }
  virtual Puma::CObjectInfo *assoc_obj () const { return caller_obj; }

  static TI_MethodCall *of( const ACM_Call &loc ) {
    return (TI_MethodCall*)loc.transform_info ();
  }
  
  const SyntacticContext &call_context () const { return _call_context; }
  const SyntacticContext &dest_func_context () const { return _dest_func_context; }

  const WeavePos &before_pos (WeaverBase &wb) {
    return wb.weave_pos(CallNode()->token(), WeavePos::WP_BEFORE);
  }
  const WeavePos &after_pos (WeaverBase &wb) {
    return wb.weave_pos(CallNode()->end_token(), WeavePos::WP_AFTER);
  }

  // the result type
  Puma::CTypeInfo *result_type_info () const {
    return called_func->isConversion () ?
        called_func->ConversionType () :
        called_func->TypeInfo()->TypeFunction()->BaseType ();
  }

  bool has_result () const {
    return !result_type_info ()->isVoid ();
  }

  // that type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *that_type () const { return get_that_type (caller_obj); }

  Puma::CClassInfo *target_class () const {
    Puma::CTypeInfo *type = target_type ();
    if (type) {
      type = type->UnqualType ();
      if (type->TypePointer ())
        type = type->BaseType ();
      if (type != &Puma::CTYPE_VOID && type->Record () &&
          type->Record ()->ClassInfo()) {
        Puma::CClassInfo *ci = type->Record()->ClassInfo ()->DefObject ();
        return ci;
      }
    }
    return 0;
  }

  bool target_is_const() const {
    return target_type ()->isConst ();
  }

  // target type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *target_type () const {
    bool is_ptr;
    Puma::CTree *expr = target_expr (is_ptr);
    if (expr) {
      Puma::CTypeInfo *type = expr->Type ();
      // if this is a pointer or reference, take the base type
      while (type && (type->TypeAddress () || !type->isRecord ()))
        type = type->BaseType ();
      assert (type);
      return type;
    }
    else if (called_func->isMethod ()) {
      if (called_func->isStaticMethod ()) {
        return called_func->ClassScope ()->TypeInfo ();
      }
      else {
        if (caller_obj->FunctionInfo ()) {
          assert (caller_obj->ClassScope ());
          return caller_obj->ClassScope ()->TypeInfo ();
        }
        else
          return &Puma::CTYPE_VOID;
      }
    }
    else
      return &Puma::CTYPE_VOID;
  }

  bool is_call_op () const {
    return strcmp (called_func->Name().c_str (), "operator ()") == 0;
  }

  // the target object of the call or NULL
  Puma::CT_Expression *target_expr (bool &is_ptr) const {
  
    // check if this call has a target object
    if (!called_func->isMethod ())
      return 0;
      
    Puma::CTree *result = 0;
  
    // what kind of node was used for the call?
    const char *calltype = node->NodeName ();
    
    // in most case we have no pointer
    is_ptr = false;
  
    if (calltype == Puma::CT_CallExpr::NodeId ()) {
      Puma::CTree *expr = ((Puma::CT_CallExpr*)node)->Expr ();
      // if the target function is given in parenthesis, skip it:
      // e.g. (foo->bar)(42);
      while (expr->NodeName () == Puma::CT_BracedExpr::NodeId ())
        expr = ((Puma::CT_BracedExpr*)expr)->Expr ();
      // find the target object if there is any
      const char *fctcalltype = expr->NodeName ();
      if (fctcalltype == Puma::CT_MembRefExpr::NodeId ()) {
        // <target>.method()
        result = expr->Son (0);
      } else if (fctcalltype == Puma::CT_MembPtrExpr::NodeId ()) {
        // <target-ptr>->method()
        is_ptr = true;
        result = expr->Son (0);
      } else {
        if (is_call_op ()) {
          // <target>()
          result = expr;
        }
        else {
        // method()
        is_ptr = true; // here 'this' is passed implicitly
        }
      }
    }
    else if (calltype == Puma::CT_BinaryExpr::NodeId ()) {
      // <target> <op> <arg>
      result = node->Son (0);
    }
    else if (calltype == Puma::CT_UnaryExpr::NodeId () ||
              calltype == Puma::CT_DerefExpr::NodeId ()) {
      // <op> <target>
      result = node->Son (1);
    }
    else if (calltype == Puma::CT_PostfixExpr::NodeId ()) {
      // <target> <op>
      result = node->Son (0);
    }
    else if (calltype == Puma::CT_IndexExpr::NodeId ()) {
      // <target> [ <index> ]
      result = node->Son (0);
    }
    else if (calltype == Puma::CT_ImplicitCall::NodeId ()) {
      // <target>
      result = node->Son (0);
    }
    else {
      cout << "unexpected node type " << node->NodeName () << " in "
           << "JPL_MethodCall::target_expr()" << endl;
    }
    return (Puma::CT_Expression*)result;
  }
      
  // the call expression node in the syntax tree
  Puma::CT_Call *CallNode() const { return node;}

  // checks if the original call uses a qualified target function name
  bool is_qualified () const {
    if (node->NodeName () != Puma::CT_CallExpr::NodeId ())
      return false;
    Puma::CTree *expr = ((Puma::CT_CallExpr*)node)->Expr ();
    const char *nodename = expr->NodeName ();
    if (nodename == Puma::CT_MembPtrExpr::NodeId () ||
        nodename == Puma::CT_MembRefExpr::NodeId ()) {
      expr = expr->Son (2); // some access function in PUMA missing!
      nodename = expr->NodeName ();
    }
    return nodename == Puma::CT_QualName::NodeId () ||
           nodename == Puma::CT_RootQualName::NodeId ();
  }
  
  // checks whether the call uses explicit template parameters
  bool has_explicit_template_params () const {
    if (node->NodeName () != Puma::CT_CallExpr::NodeId ())
      return false;
    Puma::CTree *expr = ((Puma::CT_CallExpr*)node)->Expr ();
    const char *nodename = expr->NodeName ();
    if (nodename == Puma::CT_MembPtrExpr::NodeId () ||
        nodename == Puma::CT_MembRefExpr::NodeId ()) {
      expr = expr->Son (2); // some access function in PUMA missing!
      nodename = expr->NodeName ();
    }
    return expr->IsSimpleName() &&
        expr->IsSimpleName()->Name()->NodeName() == Puma::CT_TemplateName::NodeId();
  }

  // returns true if the call needs special access rights
  bool needs_rights () const {
    // get the target object type
    Puma::CTypeInfo *type = target_type ()->UnqualType ();
    
    // no member function => no accessibility problem
    if (type->isVoid ())
      return false;
      
    // static member => no problem only if public
    if (called_func->isStaticMethod ())
      return (called_func->Protection () != Puma::CProtection::PROT_PUBLIC);
      
    // normal member function => look up the accessibility
    if (type->ClassInfo () &&
      type->ClassInfo ()->Accessibility (called_func) == Puma::CProtection::PROT_PUBLIC)
      return false;
    
    return true;
  }
};

class TI_Construction : public TI_Code {
  Puma::CFunctionInfo *_func_info;
public:
  TI_Construction () : _func_info (0) {}

  void func_info (Puma::CFunctionInfo *f) { _func_info = f; }
  virtual Puma::CObjectInfo *obj_info () const { return _func_info; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _func_info; }
  virtual Puma::CTree *tree () const { return _func_info->ClassScope ()->Tree (); }
  
  // that type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *that_type () const { return get_that_type (_func_info); }

  // target type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *target_type () const {
    Puma::CRecord *scope = _func_info->ClassScope ();
    return scope ? scope->TypeInfo () : &Puma::CTYPE_VOID;
  }
};

class TI_Destruction : public TI_Code {
  Puma::CFunctionInfo *_func_info;
public:
  TI_Destruction () : _func_info (0) {}

  void func_info (Puma::CFunctionInfo *f) { _func_info = f; }
  virtual Puma::CObjectInfo *obj_info () const { return _func_info; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _func_info; }
  virtual Puma::CTree *tree () const { return _func_info->ClassScope ()->Tree (); }

  // that type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *that_type () const { return get_that_type (_func_info); }

  // target type (for the JoinPoint-API)
  virtual Puma::CTypeInfo *target_type () const {
    Puma::CRecord *scope = _func_info->ClassScope ();
    return scope ? scope->TypeInfo () : &Puma::CTYPE_VOID;
  }
};

class TI_AdviceCode : public TransformInfo {
  // CT_AdviceDecl *_tree;
  Puma::CFunctionInfo *_func;
  ThisJoinPoint _this_join_point;
  
public:
  TI_AdviceCode () : _func (0) {}
  
  void function(Puma::CFunctionInfo *f) { _func = f; }
  virtual Puma::CTree *tree () const { return _func ? _func->Tree() : 0; }
  
  virtual Puma::CObjectInfo *obj_info () const {
    return _func;
  }
  virtual Puma::CObjectInfo *assoc_obj () const { return obj_info (); }

  Puma::CScopeInfo *Scope () const {
    return _func ? _func->Puma::CObjectInfo::QualifiedScope () : 0;
  }
  const char *name () const {
    return _func ? _func->Name () : 0;
  }
  string qual_name () {
    return _func ? _func->QualName () : "";
  }
  
  Puma::CFunctionInfo *function() const {
    return _func;
  }

  ThisJoinPoint &this_join_point () { return _this_join_point; }
  const ThisJoinPoint &this_join_point () const { return _this_join_point; }

  static TI_AdviceCode *of (const ACM_AdviceCode &loc) {
    return (TI_AdviceCode*)loc.transform_info ();
  }
};

class TI_Introduction : public TransformInfo {
public:
  virtual Puma::CObjectInfo *obj_info () const { return 0; }
  virtual Puma::CObjectInfo *assoc_obj () const { return 0; }
  //  virtual Puma::CT_AdviceDecl *tree () const { return 0; }
  virtual Puma::CTree *tree () const { return 0; }

  static TI_Introduction *of (const ACM_Introduction &loc) {
    return (TI_Introduction*)loc.transform_info ();
  }
};

class TI_Order : public TransformInfo {
//  Puma::CT_AdviceDecl *_tree;
public:
//  TI_Order () : _tree (0) {}
//  void tree (Puma::CT_AdviceDecl *ad) { _tree = ad; }
//  virtual Puma::CObjectInfo *obj_info () const {
//    return ((Puma::CT_FctDef*)_tree->Decl ())->Object ();
//  }
  virtual Puma::CObjectInfo *obj_info () const { return 0; }
//  virtual Puma::CObjectInfo *assoc_obj () const { return obj_info (); }
  virtual Puma::CObjectInfo *assoc_obj () const { return 0; }
//  virtual Puma::CTree *tree () const { return _tree; }
  virtual Puma::CTree *tree () const { return 0; }

  static TI_Order *of (const ACM_Order &loc) {
    return (TI_Order*)loc.transform_info ();
  }
};

class TI_Pointcut : public TransformInfo {
  Puma::CFunctionInfo *_func_obj;
  Puma::Location _loc;
public:
  TI_Pointcut () : _func_obj(0) {}

  void func_info (Puma::CFunctionInfo *fi) { _func_obj = fi->DefObject (); }
  Puma::CFunctionInfo *func_info () const { return _func_obj; }
  virtual Puma::CObjectInfo *obj_info () const { return _func_obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _func_obj; }
  virtual Puma::CTree *tree () const { return _func_obj->Tree (); }

  void set_location (const Puma::Location &loc) { _loc = loc; }
  const Puma::Location &get_location () const { return _loc; }

  static TI_Pointcut *of (const ACM_Pointcut &loc) {
    return (TI_Pointcut*)loc.transform_info ();
  }
};

class TI_Attribute : public TransformInfo {
  Puma::CObjectInfo *_obj;
  int _phase;
  Puma::Location _loc;
  string _params;
public:
  TI_Attribute () : _obj(0), _phase (0)/*, _pce (0)*/ {}
  ~TI_Attribute () {}
  void obj_info (Puma::CObjectInfo *fi) { _obj = fi->DefObject (); }
  virtual Puma::CObjectInfo *obj_info () const { return _obj; }
  virtual Puma::CObjectInfo *assoc_obj () const { return _obj; }

//  void setParamStr(string params) {_params = params;}
//  const string& getParamStr() {return _params;}
//  void set_attr (ACM_Attribute *attr) { _attr = attr; }
//  ACM_Attribute *get_attr () const { return _attr; }
  void set_location (const Puma::Location &loc) { _loc = loc; }
  const Puma::Location &get_location () const { return _loc; }

  static TI_Attribute *of (const ACM_Attribute &loc) {
    return static_cast<TI_Attribute*>(loc.transform_info ());
  }
};


class TI_ClassSlice : public TransformInfo {
  Puma::CObjectInfo *_obj;
  const Puma::Unit *_slice_unit;
  Puma::Unit _pattern;

  // new phase 1 implementation:
  Puma::Unit *_tokens;                             // class slice body
  std::list<Puma::Unit*> _non_inline_members;      // members defined outside of body
  std::list<Puma::Unit*> _non_inline_member_units; // corresponding source units

public:
  TI_ClassSlice () : _obj (0), _slice_unit (0), _tokens (0) {}

  // new phase 1 implementation:
  ~TI_ClassSlice () {
    delete _tokens;
    for (std::list<Puma::Unit*>::const_iterator i = _non_inline_members.begin ();
        i != _non_inline_members.end (); ++i)
      delete *i;
  }
  void set_tokens (Puma::Unit *unit) { _tokens = unit; }
  Puma::Unit *get_tokens () const { return _tokens; }
  std::list<Puma::Unit*> &non_inline_members () { return _non_inline_members; }
  std::list<Puma::Unit*> &non_inline_member_units () { return _non_inline_member_units; }
  void analyze_tokens (bool &has_base_intro, bool &has_member_intro) {
    has_base_intro = false;
    has_member_intro = false;
    Puma::Token *token = _tokens->first();
    while (token && token->type () != Puma::TOK_SEMI_COLON) {
      if (token->type () == Puma::TOK_COLON)
        has_base_intro = true;
      else if (token->type () == Puma::TOK_OPEN_CURLY) {
        has_member_intro = true;
        break;
      }
      token = _tokens->next(token);
    }
  }
  std::string base_intro () const {
    std::string result = "";
    Puma::Token *token = _tokens->first();
    token = _tokens->next(token);
    while (token && token->type () != Puma::TOK_COLON)
      token = _tokens->next(token);
    token = _tokens->next(token); // skip ':'
    while (token && token->type () != Puma::TOK_SEMI_COLON &&
        token->type () != Puma::TOK_OPEN_CURLY) {
      result += " ";
      result += token->text ();
      token = _tokens->next(token);
    }
    return result;
  }
  // end - new phase 1 implementation

  void obj_info (Puma::CObjectInfo *obj) { _obj = obj; }
  virtual Puma::CObjectInfo *obj_info () const { return _obj; }
  void slice_unit (const Puma::Unit *su) { _slice_unit = su; }
  const Puma::Unit &slice_unit () const { return *_slice_unit; }

  virtual Puma::CObjectInfo *assoc_obj () const { return obj_info (); }
//  virtual CTree *tree () const { return _acsi->def_node (); }
  virtual Puma::CTree *tree () const { return _obj->Tree (); }


  const Puma::Unit &pattern () const { return _pattern; }
  Puma::Unit &pattern () { return _pattern; }
  static TI_ClassSlice *of (const ACM_ClassSlice &loc) {
    return (TI_ClassSlice*)loc.transform_info ();
  }
  
//  static string name (Puma::ACSliceInfo *acsi) {
//    return acsi->object ()->QualName ();
//  }
};

#endif // __PumaTransformInfo_h__
