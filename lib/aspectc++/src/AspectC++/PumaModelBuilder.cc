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

#include "ModelBuilder.h"
#include "PumaTrackerDog.h"
#include "IntroductionUnit.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "ACConfig.h"
#include "ACPreprocessor.h"

// ACModel library includes
#include "ACModel/Utils.h"

// Puma library includes
#include "Puma/CFileInfo.h"
#include "Puma/CSemDatabase.h"
#include "Puma/VerboseMgr.h"
#include "Puma/SysCall.h"

using namespace Puma;

static string objname (CObjectInfo *info) {
  string result;
  CTemplateInstance *instance;
  DeducedArgument *arg;

  if (! info->Name ()/* || info->isAnonymous ()*/)
    result = "<null>";
  else
    result = info->Name ();

  instance = info->TemplateInstance ();
  if (instance) {
    result += "<";
    for (unsigned i = 0; i < instance->InstantiationArgs (); i++) {
      arg = instance->InstantiationArg (i);
      // do not list default arguments
      if (arg->isDefaultArg ())
        break;

      if (i)
        result += ",";
      ostringstream out;
      arg->print(out, false);
      result += out.str ();
    }

    if (result[result.length () -1] == '>')
      result += " ";
    result += ">";
  }
  return result;
}

// start phase 1
void PumaModelBuilder::setup_phase1 (FileUnit *unit, int tunit_len) {
  _tunit_file = newTUnit();
  _tunit_file->set_filename(model_filename (unit));
  _tunit_file->set_len(tunit_len);
  _tunit_file->set_time(modification_time (unit));
  get_files().insert(_tunit_file);
  file_map().insert (PumaModelBuilder::FileMapPair (unit, _tunit_file));
  set_root(0); // TODO: to be done by libacmodel

  register_namespace1(0, "::");
}

// start phase 2
void PumaModelBuilder::setup_phase2 (CTranslationUnit& tunit, list<CTree*> &ah_trees) {

  // use Puma's semantic information of this translation unit
  // to fill/update the project model
  build (tunit);

  // run the tracker dog to find function calls in the syntax tree
  PumaTrackerDog tracker (tunit, *this, _conf.dynamic());
  tracker.run ();
}

void PumaModelBuilder::build (CStructure &scope, ACM_Name *jpl) {
  // iterate through all namespace in this scope
  for (unsigned int n = 0; n < scope.Namespaces (); n++) {
    CNamespaceInfo *ni = scope.Namespace (n);

    // check if this namespace belongs to our project
    FileUnit *funit = source_unit (ni->Tree ());
    if (!funit || !get_project ().isBelow (funit))
      continue;

    ACM_Namespace *jpl_namespace = register_namespace (ni, jpl);
    build (*ni, jpl_namespace);
  }

  // first collect all classes and class template instances in this scope
  list<CClassInfo*> classes;
  for (unsigned int c = 0; c < scope.Types (); c++) {
    CClassInfo *ci = scope.Type (c)->ClassInfo ();
    if (!ci)
      continue;
    if (ci->isTemplate()) {
      // insert all class template instances
      CTemplateInfo *ti = ci->TemplateInfo();
      for (unsigned int tii = 0; tii < ti->Instances(); tii++)
        classes.push_back (ti->Instance(tii)->ClassInfo());
    }
    else
      // an ordinary class -> handle as well
      classes.push_back (ci);
  }

  // now handle all collected classes (and class template instances)
  for (list<CClassInfo*>::const_iterator iter = classes.begin();
      iter != classes.end (); ++iter) {
    CClassInfo *ci = *iter;

    // check if this class belongs to our project
    FileUnit *funit = source_unit (ci->DefObject()->Tree ());

    if (!funit || !get_project ().isBelow (funit))
      continue; // found no class declaration or definition that belongs to the project!

    // it is an ordinary class (or aspect)
    ACM_Class *jpl_class = register_aspect (ci->DefObject(),
        !ci->CObjectInfo::QualifiedScope () && ci->Scope () == &scope ? jpl : 0);
    if (!jpl_class)
      jpl_class = register_class (ci, !ci->CObjectInfo::QualifiedScope () &&
        ci->Scope () == &scope ? jpl : 0);
    if (!jpl_class) continue;

    if (ci->isDefined ()) {

      // setup the base class relations
      jpl_class->get_bases().clear();
      for (unsigned b = 0; b < ci->BaseClasses (); b++) {
        CClassInfo *base = ci->BaseClass (b)->Class ();
        ACM_Class *base_loc = register_aspect (base);
        if (!base_loc)
          base_loc = register_class (base);
        // ignore base classes that are not part of the model
        if (base_loc && (base_loc->type_val () & (JPT_Class|JPT_Aspect))) {
          jpl_class->get_bases().insert(base_loc);
          base_loc->get_derived().insert(jpl_class);
        }
//        else
//          cout << "*** base not found: " << ci->QualName () << " " << base->QualName () << endl;
      }

      // recursively go down one step
      build (*ci, jpl_class);

      // setup the built-in member functions
      if (!ci->isStruct () &&
          (get_project ().isBelow (ci->SourceInfo ()->SrcUnit ()) ||
           IntroductionUnit::cast (ci->SourceInfo ()->SrcUnit ()))) {
        for (unsigned f = 0; f < ci->Functions (); f++) {
          CFunctionInfo *fi = ci->Function (f)->DefObject ();
          if (!fi->isBuiltin ())
            continue;
          if (fi->isConstructor ()) {
            if (ci->isTemplateInstance())
              register_function (fi, jpl_class);
            else
              register_construction (register_function (fi, jpl_class));
          }
          else if (fi->isDestructor ()) {
            if (ci->isTemplateInstance())
              register_function (fi, jpl_class);
            else
              register_destruction (register_function (fi, jpl_class));

          }
        }
      }

      // is it a definition?
      if (jpl_class->type_val () == JPT_Aspect) {
        ACM_Aspect *jpl_aspect = (ACM_Aspect*)jpl_class;
        _vm << (::is_abstract (*jpl_aspect) ? "Abstract" : "Concrete")
            << " aspect " << signature (*jpl_aspect) << endvm;
        _vm++;
        advice_infos (jpl_aspect);
        _vm--;
      }
    }
  }

  // insert all functions and pointcuts
  for (unsigned f = 0; f < scope.Functions (); f++) {
    CFunctionInfo *fi = scope.Function (f);

    if (register_pointcut (fi, jpl))
      continue;

    if (fi->isBuiltin ())
      continue;

    // check if this function belongs to our project
    FileUnit *funit = source_unit (fi->Tree ());
    if (!funit || !get_project ().isBelow (funit))
      continue;

    // only functions that belong to the project are registered here
    // other functions might be register later if they are referenced
    register_function (fi,
      !fi->CObjectInfo::QualifiedScope () && fi->Scope () == &scope ? jpl : 0);
  }
}

void PumaModelBuilder::advice_infos (ACM_Aspect *jpl_aspect) {
  // add type information to advice nodes (phase 2)
  CClassInfo *cls = TI_Aspect::of(*jpl_aspect)->class_info();
  unsigned f = 0;
  typedef ACM_Container<ACM_AdviceCode, true> Container;
  Container &advices = jpl_aspect->get_advices();
  for (Container::iterator i = advices.begin (); i != advices.end (); ++i) {
    ACM_AdviceCode *advice_code = *i;
    while (f < cls->Functions()) {
      if (strncmp (cls->Function(f)->Name(), "__a", 3) == 0)
        break;
      f++;
    }
    if (f == cls->Functions()) {
      _err << sev_fatal << cls->Tree()->token()->location()
          << "internal problem: advice cannot be associated with advice function in aspect "
          << cls->QualName() << endMessage;
      return;
    }
    CFunctionInfo *fi = cls->Function(f);
    // argument types are the types of the function declaration
    CTypeList *arg_types = fi->TypeInfo ()->ArgTypes ();
    f++;
    unsigned first_context_arg = 0;
    if (fi->Arguments() > 0 && strcmp (fi->Argument(0u)->Name ().c_str(), "tjp") == 0)
      first_context_arg = 1;
    for (unsigned a = first_context_arg; a < arg_types->Entries (); a++) {
      advice_code->get_pointcut()->get_args().insert (register_arg (arg_types->Entry (a),
          fi->Argument(a)->Name().c_str()));
    }
    // tree is the tree of the function definition
    TI_AdviceCode::of(*advice_code)->function (fi);
  }
}


string PumaModelBuilder::model_filename (FileUnit *unit) {
  const char *fname = unit->absolutePath ();
  ACProject &prj = _conf.project ();
  // if the file does not belong to the project return the absolute path
  if (!unit->belongsTo (prj))
    return fname;
  // iterate over all project paths
  for (long p = 0; p < prj.numPaths (); p++) {
    Filename dir_abs;
    if (!SysCall::canonical (prj.src (p), dir_abs)) {
      assert (false);
      return fname;
    }
    int dir_len = strlen (dir_abs.name ());
    if (strncmp (dir_abs.name (), fname, dir_len) == 0) {
      return fname + dir_len + 1;
    }
  }
  // the file has to be below any of the directories => fatal error here
  // assert (false); acgen.c does not belong to the project with this test
  return fname;
}


// get the modification time of a file (UNIX Epoch value)
long PumaModelBuilder::modification_time (FileUnit *unit) {
  FileInfo fileinfo;
  if (! SysCall::stat (unit->absolutePath(), fileinfo))
    return -1; // File does not exists.
  return fileinfo.modi_time ();
}


void PumaModelBuilder::build (CTranslationUnit& tunit) {
  CFileInfo *fi = tunit.db ().FileInfo (0);
  ACM_Namespace *jpl_namespace = register_namespace (fi);
  build (*fi, jpl_namespace);
}

bool PumaModelBuilder::inside_template (CScopeInfo *scope) const {
  if (scope->isTemplate () ||
      (scope->isTemplateInstance () &&
       scope->TemplateInstance ()->isPseudoInstance ()))
    return true;
  if (scope->QualifiedScope ())
    return inside_template (scope->QualifiedScope ());
  if (scope->Parent () != scope)
    return inside_template (scope->Parent ());
  return false;
}

bool PumaModelBuilder::inside_template_instance (CScopeInfo *scope) {
  if (scope->isTemplateInstance ())
    return true;
  if (scope->QualifiedScope ())
    return inside_template_instance (scope->QualifiedScope ());
  if (scope->Parent () != scope)
    return inside_template_instance (scope->Parent ());
  return false;
}

bool PumaModelBuilder::is_valid_model_class (CClassInfo *ci) const {
  // Don't consider
  // * anonymous classes like template instances(?)
  // * the generated class JoinPoint
  // * classes defined in the special namespace AC
  if (strncmp (ci->Name (), "%", 1) == 0 ||
      strcmp (ci->QualName (), "JoinPoint") == 0 ||
      strncmp (ci->QualName (), "AC::", 4) == 0)
    return false;

  // Templates and classes nested in template class are not considered for
  // matching, only instances
  if (inside_template (ci))
    return false;

  return true;
}

bool PumaModelBuilder::is_valid_model_function (CFunctionInfo* fi) const {
  // Don't consider
  // * member functions of an invalid class
  // * ac++ generated functions
  // * pointcuts

  CRecord *cls = fi->ClassScope ();
  if (cls && !is_valid_model_class ((CClassInfo*)cls->DefObject ()))
    return false;

  CFunctionInfo *def = fi->DefObject ();
  if (inside_template (def))
    return false;

  if (strncmp (fi->Name (), "__ac_anon", 9) != 0) {
    if (strncmp (fi->Name (), "%a", 2) == 0 ||
        strncmp (fi->Name (), "__a", 3) == 0 ||
        strcmp (fi->Name (), "aspectof") == 0 ||
        strcmp (fi->Name (), "aspectOf") == 0)
      return false;
  }

  if (!fi->isBuiltin () && !IntroductionUnit::cast (def->SourceInfo ()->SrcUnit ()) &&
      strcmp (def->SourceInfo ()->FileName (), "<anonymous unit>") == 0)
    return false;

  return true;
}

bool PumaModelBuilder::is_valid_model_namespace (CNamespaceInfo *ni) const {
  // no template instance namespace, but anonymous namespaces!
  if ((strcmp(ni->Name (), "<unnamed>") == 0 ||
       strstr (ni->Name (), "<") == 0) &&
      strcmp (ni->QualName (), "AC") != 0 &&
      strncmp (ni->Name (), "__puma", 6) != 0)
    return true;
  return false;
}

TU_Type *PumaModelBuilder::register_type (CTypeInfo *ti) {
  TU_Type *new_elem = newType();
  new_elem->set_signature(TI_Type::name (ti));
  new_elem->type_info (ti);
  return new_elem;
}

TU_Arg *PumaModelBuilder::register_arg (CTypeInfo *ti, const string &name) {
  TU_Arg *new_elem = newArg();
  new_elem->set_type(TI_Type::name (ti));
  new_elem->set_name(name);
  new_elem->type_info (ti);
  return new_elem;
}

ACM_Pointcut *PumaModelBuilder::register_pointcut1 (ACM_Name *parent, const string &name,
    bool is_virtual, const string& expr) {

  TU_Pointcut *elem = 0;
  if (parent)
    elem = (TU_Pointcut*)map_lookup(*parent, name);
  if (!elem || elem->type_val() != JPT_Pointcut) {
    elem = newPointcut();
    elem->set_name(name);
    elem->set_expr (expr);
    elem->set_builtin(false);
    elem->set_kind(PT_NORMAL);
    if (parent) {
      if (is_virtual && expr == "0")
        elem->set_kind(PT_PURE_VIRTUAL);
      else if (is_virtual || overrides_virtual_pointcut(parent, name))
        elem->set_kind(PT_VIRTUAL);
      map_insert(*parent, *elem, name);
      parent->get_children().insert(elem);
    }
  }
  else {
    elem = 0;
  }

  return elem;
}

bool PumaModelBuilder::overrides_virtual_pointcut (ACM_Name *parent, const string &name) {
  if (parent->type_val() != JPT_Class && parent->type_val() != JPT_Aspect)
    return false;
  ACM_Class *cls = (ACM_Class*)parent;
  typedef ACM_Container<ACM_Class, false> BList;
  const BList &blist = cls->get_bases();
  for (BList::const_iterator i = blist.begin (); i != blist.end(); ++i) {
    ACM_Class *base = (ACM_Class*)*i;
    ACM_Name *elem = map_lookup(*base, name);
    if (!elem || elem->type_val() != JPT_Pointcut)
      continue;
    ACM_Pointcut *base_pct = (ACM_Pointcut*)elem;
    if (base_pct->get_kind() == PT_PURE_VIRTUAL ||
        base_pct->get_kind() == PT_VIRTUAL ||
        overrides_virtual_pointcut (base, name))
      return true;
  }
  return false;
}


TU_Pointcut *PumaModelBuilder::register_pointcut (CFunctionInfo *fi,
    ACM_Name *parent) {

  CFunctionInfo *def = fi->DefObject ();

  // find the parent model element
  if (!parent)
    return 0;

  // build the name of the function
  string name = fi->Name ().c_str ();

  TU_Pointcut *elem = (TU_Pointcut*)map_lookup(*parent, name);
  if (!elem || elem->type_val() != JPT_Pointcut)
    return 0;

  elem->get_args().clear();
  // argument types are the types of the function declaration
  CTypeList *arg_types = fi->TypeInfo ()->ArgTypes ();
  for (unsigned a = 0; a < arg_types->Entries (); a++) {
    // TODO: still needed to handle "..." in build-in pointcuts
    if (arg_types->Entry(a)->is_ellipsis())
      break;
    elem->get_args().insert (register_arg (arg_types->Entry (a),
        fi->Argument(a)->Name().c_str()));
  }
  elem->func_info(def);
  add_source_loc (elem, def);

  return elem;
}

TU_Function *PumaModelBuilder::register_function (CFunctionInfo *fi,
  ACM_Name *parent) {
  if (!is_valid_model_function (fi))
    return 0;

  CFunctionInfo *def = fi->DefObject ();

  // find the parent model element
  if (!parent && !(parent = register_scope (def)))
    return 0;

  // build the name of the function for lookup
  string sig  = TI_Function::signature (def);
  
  // register the element
  TU_Function *elem = (TU_Function*)map_lookup(*parent, sig);
  bool hidden_function = (elem && elem->type_val() == JPT_Function &&
      ((fi->isStatic () && !fi->isStaticMethod ()) &&
          (!elem->has_static_in () || elem->get_static_in () != _tunit_file)));
  if (!elem || hidden_function) {
    elem = newFunction();
    elem->set_name(objname(fi));
    if (fi->isStatic () && !fi->isStaticMethod ())
      elem->set_static_in (_tunit_file);
    map_insert(*parent, *elem, sig);
    parent->get_children().insert(elem);
    FunctionType ft = FT_NON_MEMBER;
    if (fi->isConstructor ())
      ft = FT_CONSTRUCTOR;
    else if (fi->isDestructor ()) {
      if (fi->isPureVirtual ())
        ft = FT_PURE_VIRTUAL_DESTRUCTOR;
      else if (fi->isVirtual () || fi->overridesVirtual ())
        ft = FT_VIRTUAL_DESTRUCTOR;
      else
        ft = FT_DESTRUCTOR;
    }
    else if (fi->isMethod ()) {
      if (fi->isStaticMethod ())
        ft = FT_STATIC_MEMBER;
      else if (fi->isPureVirtual ())
        ft = FT_PURE_VIRTUAL_MEMBER;
      else if (fi->isVirtual () || fi->overridesVirtual ())
        ft = FT_VIRTUAL_MEMBER;
      else
        ft = FT_MEMBER;
    }
    else {
      if (fi->isStatic())
        ft = FT_STATIC_NON_MEMBER;
    }
    elem->set_kind (ft);
    elem->set_builtin (fi->isBuiltin ());
    CVQualifiers cvq = CVQ_NONE;
    if (fi->TypeInfo()->isConst())
      cvq = (CVQualifiers)(cvq | CVQ_CONST);
    if (fi->TypeInfo()->isVolatile())
      cvq = (CVQualifiers)(cvq | CVQ_VOLATILE);
    elem->set_cv_qualifiers(cvq);
    elem->func_info (def);

    // TODO: We forget the arg type from arg 0. They will be deleted by jpm.
    elem->get_arg_types().clear();
    CTypeInfo *rtype = (fi->isConversion ()) ?
      fi->ConversionType() : fi->TypeInfo ()->BaseType ();
    if (!rtype->is_undefined())
      elem->set_result_type (register_type (rtype));
    // argument types are the types of the function declaration
    CTypeList *arg_types = fi->TypeInfo ()->ArgTypes ();
    elem->set_variadic_args(false);
    for (unsigned a = 0; a < arg_types->Entries (); a++)
      if (arg_types->Entry(a)->is_ellipsis())
        elem->set_variadic_args(true);
      else
        elem->get_arg_types().insert (register_type (arg_types->Entry (a)));

    if (!fi->isBuiltin () && !fi->isPureVirtual () &&
        !(inside_template_instance (fi)) &&
      (fi->SemDB ()->Project ()->isBelow (fi->SourceInfo ()->SrcUnit ()) ||
       IntroductionUnit::cast (fi->SourceInfo ()->SrcUnit ()))) {
      if (fi->isConstructor ())
        register_construction (elem);
      else if (fi->isDestructor ())
        register_destruction (elem);
      else
        register_execution (elem);

      // constructors and and destructors cannot be called
      if (!(fi->isConstructor () || fi->isDestructor ())) {
        // register a 'pseudo call join point'
        register_call (def, 0, 0, 0);
      }
    }
  }
  else {
    // make sure that elem->func_info() will be the DefObject
    if (fi == def)
      elem->func_info(def);
  }
  add_source_loc (elem, fi, fi->isFctDef () ? SLK_DEF : SLK_DECL);
  return elem;
}

TU_Variable *PumaModelBuilder::register_variable (CObjectInfo *oi, ACM_Name *parent) {
  CObjectInfo *def = oi->DefObject ();

  // find the parent model element
  if (!parent && !(parent = register_scope (def)))
    return 0;

  // get the name of the variable
  string sig = TI_Variable::name (def);

  // register the element
  // TODO: classes and variables in the same scope may have the same name!
  // This might lead to invalid results here.
  TU_Variable *elem = (TU_Variable*)map_lookup(*parent, sig);
  if (!elem || elem->type_val() != JPT_Variable) {
    elem = newVariable ();
    elem->set_name(sig);
    elem->set_kind(VT_UNKNOWN);
    elem->set_builtin(false);
    TU_Type *var_type = register_type(oi->TypeInfo());
    elem->set_type(var_type);
    elem->obj_info(def);
    map_insert(*parent, *elem, sig);
    parent->get_children().insert(elem);
  }
  return elem;
}

ACM_ClassSlice *PumaModelBuilder::register_class_slice (ACM_Name *scope, string name,
    bool is_struct) {

  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_ClassSlice)
    return (ACM_ClassSlice*)found;

  ACM_ClassSlice *new_elem = (ACM_ClassSlice*)newClassSlice();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  new_elem->set_is_struct(is_struct);
  map_insert(*scope, *new_elem, name);
  scope->get_children().insert(new_elem);
  return new_elem;
}


ACM_Class * PumaModelBuilder::register_class1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_Class)
    return (TU_Class*)found;
  ACM_Class *new_elem = newClass();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  if (!in_project) new_elem->get_tunits().insert (_tunit_file);
  new_elem->set_intro_target(in_project);
  scope->get_children().insert(new_elem);
  map_insert(*scope, *new_elem, name);
  return new_elem;
}


TU_Class *PumaModelBuilder::register_class (CClassInfo *ci, ACM_Name *parent, bool set_source) {

  // only classes are relevant
  if (!is_valid_model_class (ci))
    return 0;

  // find the parent model element
  if (!parent && !(parent = register_scope (ci)))
    return 0;

  string name = objname(ci);
  TU_Class *elem = 0;
  ACM_Name *found = map_lookup(*parent, name);
  if (!found || !(found->type_val() & (JPT_Class|JPT_Aspect))) {
    elem = (TU_Class*)newClass();
    elem->set_name(name);
    elem->set_builtin(false);
    parent->get_children().insert(elem);
    map_insert(*parent, *elem, name);
    bool intro_target = is_intro_target (ci->DefObject ());
    elem->set_intro_target (intro_target);
    if (!intro_target) elem->get_tunits().insert (_tunit_file);
  }
  else
    elem = (TU_Class*)found;
  // set the class attributes
  elem->class_info (ci);
  if (set_source && elem->type_val () != JPT_Aspect)
    add_source_loc (elem, ci, ci->isDefined () ? SLK_DEF : SLK_DECL);
  return elem;
}

ACM_Aspect * PumaModelBuilder::register_aspect1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Name *found = map_lookup(*scope, name);
  if (found && found->type_val() == JPT_Aspect)
    return (TU_Aspect*)found;
  ACM_Aspect *new_elem = newAspect();
  new_elem->set_name(name);
  new_elem->set_builtin(false);
  if (!in_project) new_elem->get_tunits().insert (_tunit_file);
  new_elem->set_intro_target(in_project);
  scope->get_children().insert(new_elem);
  map_insert(*scope, *new_elem, name);
  return new_elem;
}


TU_Aspect *PumaModelBuilder::register_aspect (CClassInfo *ci, ACM_Name *parent) {
  // only classes are relevant
  if (!is_valid_model_class (ci))
    return 0;

  // find the parent model element
  if (!parent && !(parent = register_scope (ci)))
    return 0;

  string name = ci->Name().c_str();
  TU_Aspect *elem = 0;
  ACM_Name *found = map_lookup(*parent, name);
  if (!found || found->type_val() != JPT_Aspect)
    return 0;
  elem = (TU_Aspect*)found;

  // set the aspect attributes
  elem->class_info(ci);
  return elem;
}

bool PumaModelBuilder::is_intro_target (CClassInfo *def) const {
  return IntroductionUnit::cast (def->SourceInfo ()->SrcUnit ()) ||
		  !(!get_project ().isBelow (def->SourceInfo ()->SrcUnit ()) ||
           def->isTemplateInstance () ||
           !def->isDefined ());
}

ACM_Namespace * PumaModelBuilder::register_namespace1 (ACM_Name *scope, string name, bool in_project) {
  ACM_Namespace *result = 0;
  assert(scope || name == "::");
  ACM_Name *found = (scope ? map_lookup(*scope, name) : get_root());
  if (!found || found->type_val() != JPT_Namespace) {
    result = newNamespace();
    result->set_name(name);
    result->set_builtin(false);
    if (!in_project) result->get_tunits().insert (_tunit_file);
    if (scope) {
      scope->get_children().insert(result);
      map_insert(*scope, *result, name);
    }
    else {
      set_root(result);
    }
  }
  else
    result = (TU_Namespace*)found;
  return result;
}


TU_Namespace *PumaModelBuilder::register_namespace (CNamespaceInfo *n,
  ACM_Name *parent) {
    // not all Puma namespaces should be registered
  if (!is_valid_model_namespace (n))
    return 0;

  // find the parent model element
  if (!parent && !n->GlobalScope () && !(parent = register_scope (n)))
    return 0;

  TU_Namespace *new_elem = 0;
  if (!parent) {
    if (get_root())
      return (TU_Namespace*)get_root();
    new_elem = newNamespace();
    new_elem->set_name("::");
    new_elem->set_builtin(true);
    set_root(new_elem);
  }
  else {
    string name = n->Name().c_str();
    ACM_Name *found = map_lookup(*parent, name);
    if (found && found->type_val() == JPT_Namespace)
      return (TU_Namespace*)found;
    new_elem = newNamespace();
    new_elem->set_name(name);
    new_elem->set_builtin(false);
    parent->get_children().insert(new_elem);
    map_insert(*parent, *new_elem, name);
  }

  // set namespace attributes
  new_elem->namespace_info (n);
  if (new_elem != get_root ())
    add_source_loc (new_elem, n);
  return new_elem;
}


// create a new call join point in the join point model
TU_MethodCall *PumaModelBuilder::register_call (CFunctionInfo *called, CT_Call *call_node,
    CObjectInfo *caller, int local_id) {

  // find the called function in the join point model
  ACM_Function *called_func = register_function (called);
  if (!called_func) {
//  if a called function is, for instance, a member of a local class, it is
//  perfectly valid that we don't find it in the model -> ignore call join-point
//    _err << sev_error << "called function \'" << called->QualName ()
//         << "\' not found in join point model" << endMessage;
    return 0;
  }

  // what is the lexical scope of this call?
  ACM_Name *lexical = 0;
  if (!caller) {
    lexical = 0; // a pseudo call join point
  }
  else if (caller->FunctionInfo ()) {
    // TODO: better cache the JPL object of the current function
    ACM_Any *loc = register_function (caller->FunctionInfo (), 0);
    if (!loc) {
      // TODO: calls in advice code are silently ignored here at the moment
//      _err << sev_error << call_node->token ()->location ()
//           << "location of function call invalid" << endMessage;
      return 0;
    }
    lexical = (ACM_Name*)loc;
  }
  else {
    lexical = register_variable (caller);
    if (!lexical) {
      _err << sev_error << call_node->token ()->location ()
           << "location of function call invalid" << endMessage;
      return 0;
    }
  }

  TU_MethodCall *new_elem = newCall();
  new_elem->set_target(called_func);
  called_func->get_calls().insert(new_elem);
  new_elem->set_lid(local_id);
  new_elem->called (called);
  new_elem->caller (caller);
  new_elem->tree (call_node);
  if (call_node)
    add_source_loc (new_elem, call_node);
  // set the parent in the join point model structure
  if (lexical) { // pseudo-calls are invisible
    lexical->get_children().insert(new_elem);
  }

  // For functions with default arguments, not more than the number of args
  // in the call expression is used.
  bool no_operator = (call_node &&
                      call_node->NodeName () == CT_CallExpr::NodeId () &&
                      ((CT_CallExpr *)call_node)->Arguments ());
  if (no_operator) {
    // argument types are the types from the target function declaration
    CTypeList *formal_arg_types = called->TypeInfo ()->ArgTypes ();
    int args = (int)formal_arg_types->Entries ();
    CT_ExprList *current_arg_list = ((CT_CallExpr *)call_node)->Arguments ();
    int call_args = current_arg_list->Entries ();
    if (called_func->get_variadic_args()) {
      args--; // ignore the ellipsis
      for (int a = args; a < current_arg_list->Entries (); a++) {
        CTypeInfo *arg_type =
          ((CT_Expression*)current_arg_list->Entry (a))->Type ();
        new_elem->get_variadic_arg_types().insert(register_type (arg_type));
      }
    }
    else if (call_args < args) {
      new_elem->set_default_args(args - call_args);
    }
  }

  // analyze the target object type of this call
  if (call_node) {
    CTypeInfo *target_type = new_elem->target_type ();
    if (target_type != &CTYPE_VOID && target_type->Record () &&
        target_type->Record ()->ClassInfo()) {
      CClassInfo *ci = target_type->Record()->ClassInfo ()->DefObject ();
      ACM_Class *target_class = register_aspect(ci);
      if (!target_class) target_class = register_class(ci);
      assert (target_class);
      new_elem->set_target_class(target_class);
    }
  }

  return new_elem;
}

// create a new execution join point
TU_Method *PumaModelBuilder::register_execution (ACM_Function *ef) {
  CFunctionInfo *func = ((TI_Function*)ef->transform_info ())->func_info ();
  TU_Method *new_elem = newExecution();
  ef->get_children().insert(new_elem);
  new_elem->func_info (func);
  return new_elem;
}

// create a new construction join point
TU_Construction *PumaModelBuilder::register_construction (ACM_Function *cf) {
  assert (cf);
  CFunctionInfo *func = ((TI_Function*)cf->transform_info ())->func_info ();
  TU_Construction *new_elem = newConstruction();
  cf->get_children().insert(new_elem);
  new_elem->func_info (func);
  return new_elem;
}

// create a new construction join point
TU_Destruction *PumaModelBuilder::register_destruction (ACM_Function *df) {
  assert (df);
  CFunctionInfo *func = ((TI_Function*)df->transform_info ())->func_info ();
  TU_Destruction *new_elem = newDestruction();
  df->get_children().insert(new_elem);
  new_elem->func_info (func);
  return new_elem;
}

// TODO: temporary solution for dac++
void PumaModelBuilder::register_attr_access (CAttributeInfo *attr, CTree *node) {
  _access_infos.push_back (AccessInfo (attr, node));
}

ACM_Name *PumaModelBuilder::register_scope (CObjectInfo *obj) {
  ACM_Name *result = 0;
  CScopeInfo *scope = scope_obj (obj);
  if (scope) {
    if (scope->NamespaceInfo ())
      result = register_namespace (scope->NamespaceInfo ());
    else if (scope->ClassInfo ()) {
      result = register_aspect (scope->ClassInfo ());
      if (!result)
        result = register_class (scope->ClassInfo ());
    }
  }

//  in some cases, e.g. join-points within local classes it can happen that
//  the scope of a join-point is not known in the model -> no error!
//  if (!result) {
//    _err << sev_error << "parent '" << scope_name (obj).c_str ()
//       << "' of model element " << obj->QualName () << " not found"
//       << endMessage;
//  }

  return result;
}

CScopeInfo *PumaModelBuilder::scope_obj (CObjectInfo *oi) {
  CScopeInfo *scope = 0;
  if (oi->TemplateInstance ()) {
    scope = oi->TemplateInstance ()->Template ()->CObjectInfo::QualifiedScope ();
    if (!scope) scope = oi->TemplateInstance ()->Template ()->Scope ();
  }
  else {
    scope = oi->QualifiedScope ();
    if (!scope) scope = oi->Scope ();
  }
  // if this is a template instance scope, go to the parent
  if (!(strcmp (scope->Name(), "<unnamed>") == 0)) {
    while (scope->isAnonymous () && strstr (scope->Name (), "<")) {
      scope = scope->Parent ();
    }
  }
  return scope;
}

string PumaModelBuilder::scope_name (CObjectInfo *oi) {
  CScopeInfo *scope = scope_obj (oi);
  assert (scope);
  // is it the globale scope
  if (scope->GlobalScope ())
    return "::";
  ostringstream scope_name;
  if (scope->TypeInfo () && !scope->TypeInfo ()->isUndefined ())
    scope_name << *scope->TypeInfo ();
  else if (scope->isAnonymous ())
    scope_name << "<noname>";
  else
    scope_name << scope->QualName ();
  return scope_name.str ();
}


// add the source location to a model element by using the syntax tree node
void PumaModelBuilder::add_source_loc (ACM_Any *name, ACToken token, ACToken end_token, SourceLocKind kind) {

  assert(name);

  // check if this file belong to our project
  FileUnit *funit = ACPreprocessor::source_unit (token);
  if (!funit)
    return;

  if (!get_project ().isBelow (funit)) {
    if (name->type_val () & JPT_Name) {
      ACM_Name *jpl_name = (ACM_Name*)name;
      // TODO: really use linear search here?
      typedef ACM_Container<ACM_TUnit, false> Container;
      const Container &tunits = jpl_name->get_tunits ();
      bool found = false;
      for (Container::const_iterator i = tunits.begin(); i != tunits.end(); ++i) {
        if (*i == _tunit_file) {
          found = true;
          break;
        }
      }
      if (!found)
        jpl_name->get_tunits ().insert (_tunit_file);
    }
    return;
  }

  PumaModelBuilder::FileMap::iterator i = file_map ().find (funit);
  ACM_File *file = 0;
  if (i != file_map ().end ())
    file = i->second;
  else {
    // TODO: temporary hack
    int len = ((Token*)funit->last ())->location ().line ();
    // TODO: in the future, handle aspect headers differently
    ACM_Header *new_file = newHeader();
    get_files().insert(new_file);
    new_file->set_filename(model_filename (funit));
    new_file->set_len(len);
    new_file->get_in().insert(_tunit_file);
    new_file->set_time (modification_time (funit));
    file = new_file;
    file_map ().insert (PumaModelBuilder::FileMapPair (funit, file));
  }
  int line = token.location ().line ();
  int len = end_token.location ().line () - line + 1;
  // TODO: really use linear search here?
  typedef ACM_Container<ACM_Source, true> Container;
  const Container &sources = name->get_source ();
  bool found = false;
  for (Container::const_iterator i = sources.begin(); i != sources.end(); ++i) {
    if ((*i)->get_line () == line &&
        (*i)->get_file () == file &&
        (*i)->get_kind () == kind &&
        (*i)->get_len () == len) {
//      if ((*i)->get_len () != len)
//        _err << sev_warning << "Internal problem: Length differs for source at same location (file='" << file->get_filename().c_str () << "', line="
//        << line << ")" << endMessage;
      found = true;
      break;
    }
  }
  if (!found) {
    ACM_Source *source = newSource();
    source->set_file(file);
    source->set_line(line);
    source->set_len(len);
    source->set_kind(kind);
    name->get_source().insert(source);
  }
}


FileUnit *PumaModelBuilder::source_unit (CTree *tree) const {
  if (!tree || !tree->token ())
    return 0;
  return ACPreprocessor::source_unit (ACToken (tree->token()));
}

string PumaModelBuilder::tree_to_string (CTree *node) {
  string result;
  if (node->NodeName () == CT_Token::NodeId ())
    result = node->token ()->text ();
  else {
    for (int s = 0; s < node->Sons (); s++) {
      if (s > 0) result += " ";
      result += tree_to_string (node->Son (s));
    }
  }
  return result;
}
