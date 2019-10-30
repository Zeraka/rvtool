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

#ifndef __ClangModelBuilder_h__
#define __ClangModelBuilder_h__

#include "ACModel/Elements.h"
#include "ACToken.h"
#include "ACProject.h"
#include "ClangToken.h"
#include "TransformInfo.h"
#include "ACErrorStream.h"
#include "ClangAnnotation.h"
#include "ClangFlowAnalysis.h"

namespace Puma {
  class VerboseMgr;
}

namespace clang {
  class Decl;
  class DeclContext;
  class Stmt;
  class CallExpr;
  class FunctionDecl;
  class NamespaceDecl;
  class RecordDecl;
  class SourceLocation;
  class QualType;
}

namespace llvm {
  class StringRef;
}

class ACConfig;

// FIXME: needs to be ported to clang
//struct AccessInfo {
//  Puma::CAttributeInfo *_info;
//  Puma::CTree *_tree;
//  AccessInfo () {}
//  AccessInfo (Puma::CAttributeInfo *info, Puma::CTree *tree) :
//    _info (info), _tree (tree) {}
//
//};
//

class ClangModelBuilder : public ProjectModel {

  Puma::VerboseMgr &_vm;
  ACErrorStream &_err;
  ACConfig &_conf;
  ACProject &_project;
  ACM_TUnit *_tunit_file;
  ClangFlowAnalysis _flow_analysis;
  AnnotationMap _annotation_map;

public:
  // map needed to check if a file is already known
  typedef map<std::string, ACM_File*> FileMap;
  typedef FileMap::value_type FileMapPair;
  FileMap _file_map;
  FileMap &file_map() { return _file_map; }

  // config is needed for decisions in ASTConsumer
  ACConfig &conf() { return _conf; }

  const AnnotationMap &annotation_map() const { return _annotation_map; }
  AnnotationMap &annotation_map() { return _annotation_map; }

private:

  int _tunit_len;

  // FIXME: needs to be ported to clang
  // TODO: temporary solution
//  list<AccessInfo> _access_infos;
  
  int _time;

public:
  void advice_infos (ACM_Aspect *jpl_aspect);

  // determine a model element filename for a unit
  string model_filename (const char *fname);
  
  // get the modification time of a file (UNIX Epoch value)
  long modification_time (const string &name);

  ClangModelBuilder (Puma::VerboseMgr &vm, ACErrorStream &err, ACConfig &conf, ACProject &project) :
    _vm (vm), _err (err), _conf (conf), _project(project) {}
  ACProject &get_project () const { return _project; }
  void setup_phase1 (const string &tunit_name, int tunit_len);

  bool is_valid_model_class (clang::RecordDecl *ci) const;
  bool is_intro_target (clang::Decl *ci) const;
  bool is_valid_model_function (clang::FunctionDecl *fi) const;
  bool is_valid_model_variable( clang::DeclaratorDecl *vi ) const;
  bool is_valid_model_namespace (clang::NamespaceDecl *ni) const;
  bool is_valid_model_type( const clang::QualType& ) const;

  bool inside_template (clang::DeclContext *scope) const;
  static bool inside_template_instance (clang::DeclContext *scope);

  // create a type in the join point model from a Puma node
  TU_Type *register_type (clang::QualType ti);

  // create a (pointcut) argument in the join point model from a Puma node
  TU_Arg *register_arg (clang::QualType ti, llvm::StringRef name);

  // get the translation unit as a model object
  ACM_TUnit *tunit_file () const { return _tunit_file; }

  // create a named pointcut in the join point model (phase 1)
  ACM_Pointcut *register_pointcut1 (ACM_Name *parent, const std::string &name,
      bool is_virtual, const std::string& expr);
  bool overrides_virtual_pointcut (ACM_Name *parent, const string &name); // helper function

  // create an attribute in the join point model (phase 1)
  TU_Attribute *register_attrdecl1(ACM_Namespace *parent, const std::string &name);

  // find an attribute declaration
  TU_Attribute *find_attrdecl (const std::vector<std::string> &qual_name);

  // create a named pointcut in the join point model from a Puma node
  TU_Pointcut *register_pointcut (clang::FunctionDecl *fi, ACM_Name *parent);

  // create a function in the join point model from a Puma node
  TU_Function *register_function (clang::FunctionDecl *fi,
                                  ACM_Name *parent = 0);

  // create a global variable or member variable in the join point model
  TU_Variable *register_variable( clang::DeclaratorDecl *oi, ACM_Name *parent = 0 );

  // create a class slice in the join point model (phase 1)
  ACM_ClassSlice *register_class_slice (ACM_Name *scope, string name, bool is_struct);

  // create a class in the join point model (phase 1)
  ACM_Class *register_class1 (ACM_Name *scope, std::string name, bool in_project = true);

  //create a reference to the Attribute in the join point model
//  TU_Attribute *register_attribute(clang::RecordDecl *ci, ACM_Name *parent = 0, bool set_source = true);

  // create a class in the join point model from a Puma node
  TU_Class *register_class (clang::RecordDecl *ci, ACM_Name *parent = 0, bool set_source = true);

  // register the implicitly defined constructor/destructor for a class or aspect
  void register_constructor_and_destructor (clang::RecordDecl *ci, ACM_Class *elem);

  // create an aspect in the join point model (phase 1)
  ACM_Aspect *register_aspect1 (ACM_Name *scope, std::string name,
      bool in_project = true);

  // create an aspect in the join point model from a Puma node
  TU_Aspect *register_aspect (clang::RecordDecl *ci, ACM_Name *parent = 0);

  // create a Namespace in the join point model (phase 1)
  ACM_Namespace *register_namespace1 (ACM_Name *scope, std::string name,
      bool in_project = true);

  // create a Namespace in the join point model from a Puma node (phase 2)
  TU_Namespace *register_namespace (clang::NamespaceDecl *ni,
                                    ACM_Name *parent = 0);

  // classes to hold parameters when registering access joinpoints
  struct JoinpointContext {
    ACM_Any *parent;
    clang::Decl *parent_decl;
    unsigned local_id;
  };

  template<typename ElementType, typename DeclType> struct EntityAccessInfo {
    ElementType *element;
    DeclType *decl;
    clang::Expr *tree_node;
    clang::Expr *ref_node;
  };
  typedef EntityAccessInfo<ACM_Variable, clang::DeclaratorDecl> VarAccessInfo;

  // This method creates a new built-in operator-"pseudo"-function and a call
  // of this function and registers both in the AspectC++-join-point-model.
  TU_Builtin* register_builtin_operator_call(clang::Expr*, clang::DeclaratorDecl*, int);

  // create a new call join point in the join point model
  ACM_Access *register_call (clang::FunctionDecl *called, clang::CallExpr
                             *call_node, clang::DeclaratorDecl *caller,
                             int local_id);

  // create a new get join point
  ACM_Access *register_get( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src = 0 );

  // create a new set join point
  ACM_Access *register_set( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src = 0 );

  // create a new ref join point
  TU_Ref *register_ref( VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src = 0 );

  // fill common information in Access node
  template<class NODE> void fillAccessNode( NODE *new_elem, VarAccessInfo &var, JoinpointContext& context, TU_Builtin *var_src );

  // fill common information in Access nodes using ref only
  template<class NODE> void fillRefAccessNode( NODE *new_elem, VarAccessInfo &var, JoinpointContext& context );

  // create a new execution join point
  TU_Method *register_execution (ACM_Function *ef);
  
  // create a new construction join point
  TU_Construction *register_construction (ACM_Function *cf);
  
  // create a new construction join point
  TU_Destruction *register_destruction (ACM_Function *df);
  
  // TODO: temporary solution for dac++
  // FIXME: needs to be ported to clang
//  void register_attr_access (Puma::CAttributeInfo *attr, Puma::CTree *node);

  // register the scope of a given element
  ACM_Name *register_scope( clang::DeclaratorDecl *obj );
  ACM_Name *register_scope (clang::DeclContext *obj, bool parent = true);
  
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
  virtual TU_Builtin *newBuiltin() { return remember (new TU_Builtin, JPTI_Builtin); }
  virtual TU_Get *newGet() { return remember( new TU_Get(), JPTI_Get ); }
  virtual TU_Set *newSet() { return remember( new TU_Set(), JPTI_Set ); }
  virtual TU_Ref *newRef() { return remember( new TU_Ref(), JPTI_Ref ); }
  virtual TU_GetRef *newGetRef() { return remember( new TU_GetRef(), JPTI_GetRef ); }
  virtual TU_SetRef *newSetRef() { return remember( new TU_SetRef(), JPTI_SetRef ); }
  virtual TU_Method *newExecution() { return remember (new TU_Method, JPTI_Execution); }
  virtual TU_Construction *newConstruction() { return remember (new TU_Construction, JPTI_Construction); }
  virtual TU_Destruction *newDestruction() { return remember (new TU_Destruction, JPTI_Destruction); }
  virtual TU_Pointcut *newPointcut() { return remember (new TU_Pointcut, JPTI_Pointcut); }
  virtual TU_Attribute *newAttribute() { return remember (new TU_Attribute, JPTI_Attribute); }
  virtual TU_AdviceCode *newAdviceCode() { return remember (new TU_AdviceCode, JPTI_AdviceCode); }
  virtual TU_Introduction *newIntroduction() { return remember (new TU_Introduction, JPTI_Introduction); }
  virtual TU_Order *newOrder() { return remember (new TU_Order, JPTI_Order); }
  virtual TU_Aspect *newAspect() { return remember (new TU_Aspect, JPTI_Aspect); }

  void add_source_loc (ACM_Any *name, clang::SourceLocation location, clang::SourceLocation endlocation, SourceLocKind kind = SLK_NONE);
  void add_source_loc (ACM_Any *name, clang::Decl *tree, SourceLocKind kind = SLK_NONE);
  void add_source_loc (ACM_Any *name, clang::Stmt *tree, SourceLocKind kind = SLK_NONE);
  void add_source_loc (ACM_Any *name, ClangToken token, ClangToken end_token, SourceLocKind kind = SLK_NONE);

  // Analyze a declaration and add found attributes to the model element 'name'
  void handle_attributes (ACM_Name *name, clang::Decl *decl);

  // TODO: temporary solution
  // FIXME: needs to be ported to clang
//  const list<AccessInfo> &access_infos () const { return _access_infos; }
};

#endif
