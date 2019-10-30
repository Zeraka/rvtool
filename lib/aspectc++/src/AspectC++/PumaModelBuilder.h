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

#ifndef __PumaModelBuilder_h__
#define __PumaModelBuilder_h__

#include "ACModel/Elements.h"
#include "ACToken.h"
#include "TransformInfo.h"
#include "ACProject.h"
#include "ACErrorStream.h"

namespace Puma {
  class CTranslationUnit;
  class CSemDatabase;
  class CStructure;
  class CScopeInfo;
  class ACSliceInfo;
  class VerboseMgr;
  class FileUnit;
}

class ACConfig;

struct AccessInfo {
  Puma::CAttributeInfo *_info;
  Puma::CTree *_tree;
  AccessInfo () {}
  AccessInfo (Puma::CAttributeInfo *info, Puma::CTree *tree) :
    _info (info), _tree (tree) {}
  
};


class PumaModelBuilder : public ProjectModel {

  Puma::VerboseMgr &_vm;
  ACErrorStream &_err;
  ACConfig &_conf;
  ACProject &_project;
  ACM_TUnit *_tunit_file;

public:
  // map needed to check if a file is already known
  typedef map<Puma::Unit*, ACM_File*> FileMap;
  typedef FileMap::value_type FileMapPair;
  FileMap _file_map;
  FileMap &file_map() { return _file_map; }

private:

  int _tunit_len;

  // TODO: temporary solution
  list<AccessInfo> _access_infos;
  
  int _time;

  void build (Puma::CTranslationUnit& tunit);
  void build (Puma::CStructure &structure, ACM_Name *jpl = 0);
  void advice_infos (ACM_Aspect *jpl_aspect);
//  void check (ACM_Aspect *jpl_aspect, ACIntroductionInfo *acii, ACM_Introduction *intro);
  
  // transform a syntax tree into a string
  static string tree_to_string (Puma::CTree *node);

public:

  // determine a model element filename for a unit
  string model_filename (Puma::FileUnit *unit);
  
  // get the modification time of a file (UNIX Epoch value)
  long modification_time (Puma::FileUnit *unit);

  PumaModelBuilder (Puma::VerboseMgr &vm, ACErrorStream &err, ACConfig &conf, ACProject &project) :
    _vm (vm), _err (err), _conf (conf), _project(project) {}
  ACProject &get_project () const { return _project; }
  void setup_phase1 (Puma::FileUnit *unit, int tunit_len);
  void setup_phase2 (Puma::CTranslationUnit& tunit, list<Puma::CTree*> &ah_trees);

  bool is_valid_model_class (Puma::CClassInfo *ci) const;
  bool is_intro_target (Puma::CClassInfo *ci) const;
  bool is_valid_model_function (Puma::CFunctionInfo *fi) const;
  bool is_valid_model_namespace (Puma::CNamespaceInfo *ni) const;

  bool inside_template (Puma::CScopeInfo *scope) const;
  static bool inside_template_instance (Puma::CScopeInfo *scope);

  // create a type in the join point model from a Puma node
  TU_Type *register_type (Puma::CTypeInfo *ti);

  // create a (pointcut) argument in the join point model from a Puma node
  TU_Arg *register_arg (Puma::CTypeInfo *ti, const string &name);

  // get the translation unit as a model object
  ACM_TUnit *tunit_file () const { return _tunit_file; }

  // create a named pointcut in the join point model (phase 1)
  ACM_Pointcut *register_pointcut1 (ACM_Name *parent, const std::string &name,
      bool is_virtual, const std::string& expr);
  bool overrides_virtual_pointcut (ACM_Name *parent, const string &name); // helper function


  // create a named pointcut in the join point model from a Puma node
  TU_Pointcut *register_pointcut (Puma::CFunctionInfo *fi, ACM_Name *parent);

  // create a function in the join point model from a Puma node
  TU_Function *register_function (Puma::CFunctionInfo *fi, ACM_Name *parent = 0);
  
  // create a global variable or member variable in the join point model
  TU_Variable *register_variable (Puma::CObjectInfo *oi, ACM_Name *parent = 0);

  // create a class slice in the join point model (phase 1)
  ACM_ClassSlice *register_class_slice (ACM_Name *scope, string name, bool is_struct);

//  // create a class slice in the join point model from a Puma node
//  TU_ClassSlice *register_class_slice (ACSliceInfo *acsi, ACM_Name *parent = 0);

  // create a class in the join point model (phase 1)
  ACM_Class *register_class1 (ACM_Name *scope, std::string name, bool in_project = true);

  // create a class in the join point model from a Puma node
  TU_Class *register_class (Puma::CClassInfo *ci, ACM_Name *parent = 0, bool set_source = true);

  // create an aspect in the join point model (phase 1)
  ACM_Aspect *register_aspect1 (ACM_Name *scope, std::string name,
      bool in_project = true);

  // create an aspect in the join point model from a Puma node
  TU_Aspect *register_aspect (Puma::CClassInfo *ci, ACM_Name *parent = 0);

  // create a Namespace in the join point model (phase 1)
  ACM_Namespace *register_namespace1 (ACM_Name *scope, std::string name,
      bool in_project = true);

  // create a Namespace in the join point model from a Puma node (phase 2)
  TU_Namespace *register_namespace (Puma::CNamespaceInfo *ni, ACM_Name *parent = 0);

  // create a new call join point in the join point model
  TU_MethodCall *register_call (Puma::CFunctionInfo *called, Puma::CT_Call *call_node,
      Puma::CObjectInfo *caller, int local_id);

  // create a new execution join point
  TU_Method *register_execution (ACM_Function *ef);
  
  // create a new construction join point
  TU_Construction *register_construction (ACM_Function *cf);
  
  // create a new construction join point
  TU_Destruction *register_destruction (ACM_Function *df);
  
  // TODO: temporary solution for dac++
  void register_attr_access (Puma::CAttributeInfo *attr, Puma::CTree *node);

  // register the scope of a given element
  ACM_Name *register_scope (Puma::CObjectInfo *obj);
  
  // extended types...
  // factory methods that remember all objects of this model
  virtual TU_CodeAdvice *newCodeAdvice() { return remember (new TU_CodeAdvice, JPTI_CodeAdvice); }
  virtual TU_Type *newType() { return remember (new TU_Type, JPTI_Type); }
  virtual TU_Arg *newArg() { return remember (new TU_Arg, JPTI_Arg); }
  virtual TU_Namespace *newNamespace() { return remember (new TU_Namespace, JPTI_Namespace); }
  virtual TU_Variable *newVariable() { return remember (new TU_Variable, JPTI_Variable); }
  virtual TU_Function *newFunction() { return remember (new TU_Function, JPTI_Function); }
  virtual TU_Class *newClass() { return remember (new TU_Class, JPTI_Class); }
  virtual TU_ClassSlice *newClassSlice() { return remember (new TU_ClassSlice, JPTI_ClassSlice); }
  virtual TU_MethodCall *newCall() { return remember (new TU_MethodCall, JPTI_Call); }
  virtual TU_Method *newExecution() { return remember (new TU_Method, JPTI_Execution); }
  virtual TU_Construction *newConstruction() { return remember (new TU_Construction, JPTI_Construction); }
  virtual TU_Destruction *newDestruction() { return remember (new TU_Destruction, JPTI_Destruction); }
  virtual TU_Pointcut *newPointcut() { return remember (new TU_Pointcut, JPTI_Pointcut); }
  virtual TU_AdviceCode *newAdviceCode() { return remember (new TU_AdviceCode, JPTI_AdviceCode); }
  virtual TU_Introduction *newIntroduction() { return remember (new TU_Introduction, JPTI_Introduction); }
  virtual TU_Order *newOrder() { return remember (new TU_Order, JPTI_Order); }
  virtual TU_Aspect *newAspect() { return remember (new TU_Aspect, JPTI_Aspect); }

  // return the name of the scope of an object
  static string scope_name (Puma::CObjectInfo *oi);
  
  // return the scope of an object
  static Puma::CScopeInfo *scope_obj (Puma::CObjectInfo *oi);

  // add the source location to a model element by using a given CObjectInfo
  void add_source_loc (ACM_Any *name, Puma::CObjectInfo *obj, SourceLocKind kind = SLK_NONE) {
    add_source_loc (name, obj->Tree (), kind);
  }
  // add the source location to a model element by using the syntax tree node
  void add_source_loc (ACM_Any *name, Puma::CTree *tree, SourceLocKind kind = SLK_NONE) {
    if (tree)
      add_source_loc (name, ACToken (tree->token ()), ACToken (tree->end_token()), kind);
  }
  // add the source location to a model element by using the token information
  void add_source_loc (ACM_Any *name, ACToken token, ACToken end_token, SourceLocKind kind = SLK_NONE);

  // determine the file unit, which contains a specific token (macro expansions and introdutions are followed)
  Puma::FileUnit *source_unit (ACToken token) const;
  // determine the file unit, which contains a specific syntax (sub-)tree
  Puma::FileUnit *source_unit (Puma::CTree *tree) const;
  
  // TODO: temporary solution
  const list<AccessInfo> &access_infos () const { return _access_infos; }
};

#endif
