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

#include <iostream>
using std::cout;
using std::endl;
#include <set>
using std::set;

#include "Puma/CTree.h"
#include "Puma/CObjectInfo.h"
#include "Puma/CFileInfo.h"
#include "Puma/CEnumInfo.h"
#include "Puma/TokenStream.h"
#include "Puma/TokenProvider.h"
#include "Puma/CCSyntax.h"
#include "Puma/CCParser.h"
#include "Puma/PreprocessorParser.h"
#include "Puma/PreMacroManager.h"
#include "Puma/PreFileIncluder.h"
#include "Puma/CPrintVisitor.h"
#include "Puma/StrCol.h"

#include "PumaIntroducer.h"
#include "ACUnit.h"
#include "Plan.h"
#include "CodeWeaver.h"
#include "LineDirectiveMgr.h"
#include "IncludeGraph.h"
#include "Naming.h"
#include "AspectInfo.h"
#include "OrderInfo.h"
#include "IntroductionInfo.h"
#include "ModelBuilder.h"
#include "ACModel/Utils.h"
#include "PointCutContext.h"
#include "PointCutExpr.h"
#include "IntroductionUnit.h"
#include "TransformInfo.h"

using namespace Puma;

// Destructor: release all allocated resources
PumaIntroducer::~PumaIntroducer () {
  for (list<TokenProvider*>::iterator i = _token_providers.begin ();
    i != _token_providers.end (); ++i)
    delete *i;
  for (list<TokenStream*>::iterator i = _token_streams.begin ();
    i != _token_streams.end (); ++i)
    delete *i;
}
  
// called when a new class/union/struct/aspect is created, current scope
// is the global scope
void PumaIntroducer::class_before (CT_ClassDef *cd) {
  enter ();
  assert (cd);
  
  // get the semantic object of the class/struct/aspect definition.
  // unions are filtered out, as they are of incompatible type CUnionInfo.
  // introductions into unions require some extra effort (not implemented yet).
  CClassInfo *ci = cd->Object ()->DefObject ()-> ClassInfo ();
  if (!ci) {
    leave ();
    return;
  }

  ACProject &project = (ACProject&)*ci->SemDB ()->Project ();
  ACErrorStream &err = project.err ();
  const Unit *primary = ci->SourceInfo ()->FileInfo ()->Primary ();

  // create the weaving plan for this class
  ACM_Class *jpl = create_plan (ci);
  if (!jpl || !jpl->has_plan ()) {
    // this class is not the target of an introduction
    leave ();
    return;
  }
  ACM_ClassPlan *plan = jpl->get_plan ();

  // determine the units that should be included in front of the class
  set<const Unit*> units;
  typedef ACM_Container<ACM_MemberIntro, true> MContainer;
  for (MContainer::iterator i = plan->get_member_intros().begin();
      i != plan->get_member_intros().end(); ++i) {
    ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
    units.insert (&TI_ClassSlice::of (*cs)->slice_unit ());
  }
  typedef ACM_Container<ACM_BaseIntro, true> BContainer;
  for (BContainer::iterator i = plan->get_base_intros().begin();
      i != plan->get_base_intros().end(); ++i) {
    ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
    units.insert (&TI_ClassSlice::of (*cs)->slice_unit ());
  }

  const Unit *this_unit = (Unit*)cd->token ()->belonging_to ();
  // TODO: this_unit might be a macro unit!

  // handle introductions into introduced classes (nested introductions)
  const IntroductionUnit *intro_unit = IntroductionUnit::cast (this_unit);
  if (intro_unit) this_unit = intro_unit->final_target_unit ();

//  cout << "included units for " << ci->QualName () << " in "
//       << this_unit->name () << " :" << endl;
  for (set<const Unit*>::iterator iter = units.begin ();
    iter != units.end (); ++iter) {
    const Unit *slice_unit = *iter;
    if (slice_unit == primary) {
      if (this_unit != primary)
        err << sev_error << cd->token ()->location () 
          << "affected by aspect in '" << slice_unit->name ()
          << "'. Move the aspect into a separate aspect header." << endMessage;
    }
    else if (_ig.includes (slice_unit, this_unit)) {
      err << sev_warning << cd->token ()->location () 
          << "can't include '" << slice_unit->name ()
          << "' to avoid include cycle" << endMessage;
    }
    else {
//      cout << "new edge from " << this_unit->name () << " to "
//           << slice_unit->name () << endl;
      _ig.add_edge (this_unit, slice_unit);

      // handling of nested classes -> search the outermost class
      CClassInfo *inscls = ci;
      while (inscls->Parent ()->ClassInfo ())
        inscls = inscls->Parent ()->ClassInfo ();
        
      // namespace should be closed and re-opened
      ostringstream includes;
      _code_weaver.close_namespace (includes, inscls);
            
      // inspos should be infront of the class
      Token *inspos = (Token*)inscls->Tree ()->token ();
    
      Filename incname = project.getRelInclString (this_unit->name (), slice_unit->name ());
      includes << endl << "#ifndef ";
      Naming::guard (includes, (FileUnit*)slice_unit);
      includes << endl << "#define ";
      Naming::guard (includes, (FileUnit*)slice_unit);
      includes << endl;
      includes << "#include \"" << incname << "\"" << endl;
      includes << "#endif" << endl;
      
      // re-open the namespace
      _code_weaver.open_namespace (includes, inscls);

      string inc (includes.str ());
      _code_weaver.insert (_code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE),
        inc);

      if (_included_aspect_headers.find (slice_unit) ==
          _included_aspect_headers.end ()) {
        _included_aspect_headers.insert (slice_unit);

		CCParser *prev_parser = _parser;
        _parser = new CCParser;
        _parser->introducer (this);
        _parser->configure (project.config ());
        // setup the preprocessor
        TokenStream stream;
        stream.push ((Unit*)slice_unit);
        PreprocessorParser *prev_cpp = _cpp;
        _cpp = new PreprocessorParser (&project.err (),
          &project.unitManager (), prev_cpp->locals());
        _cpp->cscanner ().configure (project.config ());
        // cpp.macroManager ()->init (unit->name ());
        PreMacroManager *old_mm = _cpp->macroManager ();
        PreFileIncluder *old_fi = _cpp->fileIncluder ();
        PrePredicateManager *old_pm = _cpp->predicateManager ();
        _cpp->macroManager (prev_cpp->macroManager ());
        _cpp->fileIncluder (prev_cpp->fileIncluder ());
        _cpp->predicateManager (prev_cpp->predicateManager ());
        _cpp->fileIncluder ()->preprocessor (_cpp);
        _cpp->stream (&stream);
//        _cpp->configure (project.config (), false); // do not process --include option

        // initialize semantic analyzer
        _parser->semantic ().init (*ci->SemDB (),
                            *ci->SourceInfo ()->FileInfo ()->Primary ());
        ((ErrorCollector&)_parser->builder ().err ()).index (0);
        _parser->semantic ().error_sink (_parser->builder ().err ());

        TokenProvider provider (*_cpp);        // look ahead token buffer
		CTree *tree = _parser->syntax ().run (provider, &CCSyntax::trans_unit);
        if (!tree)
        	;
//          err << sev_error << cd->token ()->location ()
//              << "Parsing auto-included header failed." << endMessage;
        else
          // TODO: tree should later be freed!
          _ah_trees.push_back (tree);
        
        // print all error messages
        _parser->builder ().errors (err);
//        cout << "--------" << endl;
//        CPrintVisitor printer;
//        cout << "Printing syntax tree..." << endl;
//        printer.print (tree, cout);
        
        _cpp->macroManager (old_mm);
        _cpp->fileIncluder (old_fi);
        _cpp->predicateManager (old_pm);
        delete _cpp;
        _cpp = prev_cpp;
        _cpp->fileIncluder ()->preprocessor (_cpp);
        delete _parser;
        _parser = prev_parser;
      }
//      else
//        cout << "'" << slice_unit->name () << "' not included again!" << endl;
    }
  }
  
//  cd->add_intro (tree);
  
  leave ();
}

// called when a new class/union/struct/aspect is created
void PumaIntroducer::class_begin (CT_ClassDef *cd) {
  enter ();
  assert (cd);
  leave ();
}

// parse code that shall be introduced
CTree *PumaIntroducer::parse (list<Unit*> units, bool (CCSyntax::*rule)(),
  const char *expected_id, ACErrorStream &err) {
    
  // create a token stream and push all units onto it
  TokenStream *stream = new TokenStream;
  _token_streams.push_back (stream);
  for (list<Unit*>::reverse_iterator ui = units.rbegin ();
    ui != units.rend (); ++ui)
    stream->push (*ui);

  // parse it
  TokenProvider *provider = new TokenProvider (*stream); // look ahead buffer
  _token_providers.push_back (provider);
  CTree *tree = _parser->syntax ().run (*provider, rule);
  
  // print all error messages => does that really work?
  _parser->builder ().errors (err);
  if (tree && tree->NodeName () != expected_id)
    tree = 0; // TODO: tree should be deleted!
  return tree;
}

// called when a new class/union/struct/aspect definition ends
// (still in the class scope)
void PumaIntroducer::class_end (CT_ClassDef *cd) {
  enter ();
  assert (cd);
  bool needs_introspection = _conf.introspection(); // --introspection given?

  // get the semantic object of the class/struct/aspect definition.
  // unions are filtered out, as they are of incompatible type CUnionInfo.
  // introductions into unions require some extra effort (not implemented yet).
  CClassInfo *ci = cd->Object ()->DefObject ()-> ClassInfo ();
  if (!ci) {
    leave ();
    return;
  }

  ACErrorStream &err = ci->SemDB ()->Project ()->err ();
  // manipulate the code (will be effective after commit),
  // paste declaration before "}" of the class definition
  Token *inspos = cd->Members ()->end_token ();
  const WeavePos &pos = _code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE);

  // first check with the plan if there are intros for this class
  ACM_Class *jpl = plan_lookup (ci);
  if (jpl) {
    // collect the intros
    list<Unit*> units;
    LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();
    gen_intros (jpl, units, err, ci, lmgr, _conf.introduction_depth ());
    for (list<Unit*>::iterator ui = units.begin (); ui != units.end (); ++ui) {
      // generate introspection code ("JoinPoint") for this intro
      if (IntroductionUnit::cast (*ui)->jp_needed ()) {
        insert_introspection_code (cd, IntroductionUnit::cast (*ui)->precedence ());
        needs_introspection = true; // also generate the final introspection
      }

      // parse the intro
      list<Unit*> tmp_units;
      tmp_units.push_back (*ui);
      CTree *tree = parse (tmp_units, &CCSyntax::member_spec, CT_MembList::NodeId (), err);
      if (tree) {
        // move the introduced members into the normal member list
        CT_MembList *memb_list = (CT_MembList*)tree;
        for (int e = 0; e < memb_list->Entries (); e++) {
          CTree *entry = memb_list->Entry (e);
          cd->Members ()->InsertSon (cd->Members ()->Sons () - 1, entry);
        }
        delete memb_list;

      // paste the generated code and add the units to the manipulator's unit mgr.
        _code_weaver.insert (pos, *ui);
      }
      else {
        err << sev_error << inspos->location()
            << "class member introduction into '" << ci->QualName() << "' failed"
            << endMessage;
  
        // delete all intro units
        delete *ui;
      }
    }
  }

  // register all member functions
  jpl = 0;
  if (_jpm.is_intro_target (ci->DefObject ())) {
    // TODO: this condition is not complete correct here: abstract base classes
    // that are defined outside the project shall be handled here as well.
    // Better check if the current class is in the project model and then add
    // its functions.
    // return if this class is not an introduction target
    // try to register this class (might be a newly introduced class)
    jpl = _jpm.register_aspect (ci);
    if (!jpl) jpl = _jpm.register_class (ci, 0, false);

    // return if this is either not a valid model class or no valid intro target
    if (jpl && jpl->get_intro_target ()) {
      for (unsigned f = 0; f < ci->Functions (); f++) {
        CFunctionInfo *fi = ci->Function (f)->DefObject ();
        if (!_jpm.register_pointcut (fi, jpl))
          _jpm.register_function(fi, jpl);
      }
    }
  }

  // insert introspection code for this class if the configuration option
  // --introspection was given or the Joinpoint type was used in an introduction
  // -> inserted at the end of the class -> all intros are visible
  if (needs_introspection)
    insert_introspection_code (cd);

  // handle further generated injections for this class
  bool have_injections = false;
  ACUnit *unit = new ACUnit (err);
  *unit << endl;

  // insert friend declarations for all aspects
  have_injections |= insert_aspect_friend_decls (*unit, (CRecord*)ci);

  // insert an 'aspectof' function if this is a non-abstract aspect
  // with no user-defined aspectof function
  have_injections |= insert_aspectof_function (*unit, ci, err);

  // finalize the unit
  *unit << endu;

  // now parse the generated code
  if (have_injections) {
    ACToken inspos = ACToken (cd->Members ()->end_token ());
    const WeavePos &pos = _code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE);

    list<Unit*> units;
    units.push_back (unit);
    CTree *tree = parse (units, &CCSyntax::member_spec, CT_MembList::NodeId (), err);
    if (tree) {
      // move the introduced members into the normal member list
      CT_MembList *memb_list = (CT_MembList*)tree;
      for (int e = 0; e < memb_list->Entries (); e++) {
        CTree *entry = memb_list->Entry (e);
        cd->Members ()->InsertSon (cd->Members ()->Sons () - 1, entry);
      }
      _code_weaver.insert (pos, unit);
      delete memb_list;
    }
    else {
      err << sev_error << inspos.location()
          << "parsing weaver-injected code for class '" << ci->QualName()
          << "' failed" << endMessage;
      delete unit;
    }
    // paste a #line directive
    LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();
    ACUnit *dunit = new ACUnit (err);
    lmgr.directive (*dunit, inspos.unit (), inspos);
    *dunit << endu;
    if (dunit->empty ()) delete dunit; else _code_weaver.insert (pos, dunit);
  }

  // done
  leave ();
}

// insert an 'aspectof' function if this is a non-abstract aspect
// with no user-defined aspectof function
// helper functions
static string prot_str (CProtection::Type prot) {
  switch (prot) {
  case CProtection::PROT_PRIVATE: return "AC::PROT_PRIVATE";
  case CProtection::PROT_PROTECTED: return "AC::PROT_PROTECTED";
  case CProtection::PROT_PUBLIC: return "AC::PROT_PUBLIC";
  case CProtection::PROT_NONE: return "AC::PROT_NONE";
  }
  return 0;
}

static string spec_str (bool is_static, bool is_mutable, bool is_virtual) {
  int pos = 0;
  string result;
  if (is_static) {
    if (pos++) result += "|";
    result += "AC::SPEC_STATIC";
  }
  if (is_mutable) {
    if (pos++) result += "|";
    result += "AC::SPEC_MUTABLE";
  }
  if (is_virtual) {
    if (pos++) result += "|";
    result += "AC::SPEC_VIRTUAL";
  }
  if (!pos)
    result = "AC::SPEC_NONE";
  return result;
}

// insert friend declarations for all aspects
bool PumaIntroducer::insert_aspectof_function (ACUnit &unit, CClassInfo *ci,
    ACErrorStream &err) {

  ACM_Aspect *jpl_aspect = _jpm.register_aspect (ci);
  if (!jpl_aspect || ::is_abstract(*jpl_aspect)) {
    return false;
  }

  // iterate over all member functions and find any definitions of
  // 'aspectof' or 'aspectOf'
  CFunctionInfo *aspectOf_func = 0;
  CFunctionInfo *aspectof_func = 0;
  for (unsigned i = 0; i < ci->Functions(); i++) {
    CFunctionInfo *fi = ci->Function(i);
    if (strcmp (fi->Name (), "aspectof") == 0)
      aspectof_func = fi;
    if (strcmp (fi->Name (), "aspectOf") == 0)
      aspectOf_func = fi;
  }

  if (aspectOf_func && !aspectof_func) {
    // rename the function to 'aspectof'
    CT_FctDef *fctdef = (CT_FctDef*)aspectOf_func->Tree ();
    CT_SimpleName *name = ((CT_Declarator*)fctdef->Declarator ())->Name ();
    name = name->Name (); // if it is qualified
    const WeavePos &name_start =
      _code_weaver.weave_pos (name->token (), WeavePos::WP_BEFORE);
    const WeavePos &name_end =
        _code_weaver.weave_pos (name->end_token (), WeavePos::WP_AFTER);
    _code_weaver.replace (name_start, name_end, "aspectof");
    return false; // nothing to introduce
  }

  if (!aspectOf_func && !aspectof_func) {
    unit << endl
      << "public:" << endl
      << "  static " << ci->Name () << " *aspectof () {" << endl
      << "    static " << ci->Name () << " __instance;" << endl
      << "    return &__instance;" << endl
      << "  }" << endl
      << "  static " << ci->Name () << " *aspectOf () {" << endl
      << "    return aspectof ();" << endl
      << "  }" << endl
      << "private:" << endl;
    return true;
  }
  return false;
}

bool PumaIntroducer::insert_aspect_friend_decls (ACUnit &unit, CRecord *rec) {
  // check whether the insertion is indicated for this class or union

  // no need for template instances; the templates already get the friend injection
  if (rec->isTemplateInstance())
    return false;
  // code introduced by phase 1 (in special namespace AC) is not modified here
  if (strncmp (rec->QualName (), "AC::", 4) == 0)
    return false;
  // nested classes in template instances should also not be modified to avoid double injection
  if (_jpm.inside_template_instance(rec))
    return false;
  // the class has to belong to the project
  Unit *rec_unit = (Unit*)rec->Tree ()->token ()->belonging_to ();
  if (!is_intro (rec) && !_jpm.get_project().isBelow (rec_unit))
    return false;

  // OK, perform the insertion ...

  // get all aspects from the join point model
  ProjectModel::Selection all_aspects;
  _jpm.select (JPT_Aspect, all_aspects);

  // generate the list of aspects
  // also make sure that an aspect is not friend of itself!
  bool result = false;
  for (ProjectModel::Selection::iterator iter = all_aspects.begin ();
       iter != all_aspects.end (); ++iter) {
    ACM_Aspect &jpl = (ACM_Aspect&)**iter;
    if (string (rec->QualName ()) != jpl.get_name()) {
      unit << "  friend class ::" << jpl.get_name() << ";" << endl;
      result = true;
    }
  }

  return result;
}

// insert introspection code
//  * at the end of class definitions, after AspectC++ introductions
void PumaIntroducer::insert_introspection_code (CT_ClassDef *cd, int precedence) {
  if (!cd->Object ()->DefObject ()->ClassInfo())
    return;
  
  CClassInfo *ci = (CClassInfo*)cd->Object ()->DefObject ();
  ACErrorStream &err = ci->SemDB ()->Project ()->err ();

  // return if this class is not an introduction target or has C linkage
  if (!_jpm.is_valid_model_class (ci) || !_jpm.is_intro_target (ci) ||
      ci->Language () == CLanguage::LANG_C)
    return;

  // introspection templates cannot be declared inside of a local class
  if (ci->isLocalClass()) // integrate check into _jpm.is_valid_model_class ?
    return;

  // manipulate the code (will be effective after commit),
  // paste declaration before "}" of the class definition
  ACToken inspos = ACToken (cd->Members ()->end_token ());
  const WeavePos &pos = _code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE);

  list<Unit*> units;
  ACUnit *unit = new ACUnit (err);
  unit->name ("<typeinfo>");
  
  *unit << "public:" << endl;
  *unit << "  struct ";
  if (precedence == -1)
    *unit << "__TI";
  else
    *unit << "__TJP_" << precedence;
  *unit << " {" << endl;

  // generate the signature (fully qualified target class name)
  *unit << "    static const char *signature () { return \"";
  ci->TypeInfo ()->TypeText (*unit);
  *unit << "\"; }" << endl;

  // generate a 32-bit hash code from the fully qualified target class name
  *unit << "    enum { HASHCODE = " << StrCol::hash(ci->QualName()) << "U };" << endl;

  // generate a typedef for the target type
  *unit << "    typedef " << ci->Name () << " That;" << endl;

  // generate a list with all base classes
  *unit << "    template<int I, int __D=0> struct BaseClass {};" << endl;
  unsigned b = 0;
  for (; b < ci->BaseClasses(); b++) {
    *unit << "    template <int __D> struct BaseClass<" << b << ", __D> { typedef ";
    ci->BaseClass (b)->Class ()->TypeInfo ()->TypeText (*unit, "Type", true, true, true);
    *unit << "; ";
    *unit << "static const AC::Protection prot = " << prot_str (ci->BaseClass(b)->Protection ()) << "; ";
    *unit << "static const AC::Specifiers spec = (AC::Specifiers)("
          << spec_str (false, false, ci->BaseClass(b)->isVirtual ()) << "); ";
    *unit << "};" << endl;
  }
  *unit << "    enum { BASECLASSES = " << b << " };" << endl;

  const char *clsname = ci->Name().c_str();
  // generate Member<I> => a list with all attribute types
  *unit << "    template<int I, int __D=0> struct Member {};" << endl;
  unsigned e = 0;
  for (unsigned a = 0; a < ci->Attributes (); a++) {
    CAttributeInfo *attr = ci->Attribute (a);
    if (!is_attribute (attr))
      continue;
    *unit << "    template <int __D> struct Member<" << e << ", __D> { typedef ";
    attr->TypeInfo ()->TypeText (*unit, "Type", true, true, true);
    *unit << "; typedef AC::Referred<Type>::type ReferredType; ";
    *unit << "static const AC::Protection prot = " << prot_str (attr->Protection ()) << "; ";
    *unit << "static const AC::Specifiers spec = (AC::Specifiers)("
          << spec_str (attr->isStatic(), attr->isMutable(), false) << "); ";
    // generate pointer => the typed pointer to attribute I
    *unit << "static ReferredType *pointer (" << clsname << " *obj = 0) { return (ReferredType*)&";
    if (attr->isStatic())
      *unit << clsname << "::" << attr->Name ();
    else
      *unit << "obj->" << attr->Name ();
    *unit << "; } ";
    // generate member_name => the name of attribute i
    *unit << "static const char *name () { return \"" << attr->Name () << "\"; }" << endl;
    *unit << " }; " << endl;
    e++;
  }
//  *unit << "    enum { ELEMENTS = " << e << " };" << endl;
  *unit << "    enum { MEMBERS = " << e << " };" << endl;

//  // generate member_name => the name of attribute i
//  *unit << "    static const char *member_name (unsigned i) {";
//  if (ci->Attributes () == 0) {
//  	*unit << " return 0; }";
//  }
//  else {
//    *unit << endl << "      static const char *names[] = { ";
//    bool first = true;
//    for (unsigned a = 0; a < ci->Attributes (); a++) {
//      if (!is_attribute (ci->Attribute (a)))
//        continue;
//  	  if (!first) *unit << ", ";
//  	  *unit << "\"" << ci->Attribute (a)->Name () << "\"";
//      first = false;
//	  }
//	  *unit << " }; return names[i];" << endl
//          << "    }";
//  }
//  *unit << endl;
//
//  // generate member_pointer => the untyped pointer to attribute i
//  *unit << "    static void *member_pointer (unsigned __i, const " << clsname << " *obj) {";
//  if (ci->Attributes () == 0) {
//  	*unit << " return 0; }";
//  }
//  else {
//  	*unit << endl
//  	      << "      switch (__i) { ";
//  	e = 0;
//    for (unsigned a = 0; a < ci->Attributes (); a++) {
//      CAttributeInfo *attr = ci->Attribute (a);
//      if (!is_attribute (attr))
//        continue;
//      *unit << "case " << e << ": return (void*)&";
//      if (attr->isStatic())
//        *unit << clsname << "::" << attr->Name ();
//      else
//        *unit << "obj->" << attr->Name ();
//      *unit << "; ";
//      e++;
//	  }
//	  *unit << "default: return 0; }" << endl
//	        << "    }";
//  }
//  *unit << endl;

  // generate a list with all member functions
  *unit << "    template<int I, int __D=0> struct Function {};" << endl;
  *unit << "    template<int I, int __D=0> struct Constructor {};" << endl;
  *unit << "    template<int I, int __D=0> struct Destructor {};" << endl;
  unsigned int functions=0, constructors=0, destructors=0;
  for (unsigned f = 0; f < ci->Functions(); f++) {
    CFunctionInfo *fi = ci->Function (f);
    if (!_jpm.is_valid_model_function(fi))
      continue;

    if (fi->isConstructor ()) {
      *unit << "    template <int __D> struct Constructor<" << constructors << ", __D> { ";
      constructors++;
    }
    else if (fi->isDestructor ()) {
      *unit << "    template <int __D> struct Destructor<" << destructors << ", __D> { ";
      destructors++;
    }
    else {
      *unit << "    template <int __D> struct Function<" << functions << ", __D> { ";
      functions++;
    }

    *unit << "static const AC::Protection prot = " << prot_str (fi->Protection ()) << "; ";
    *unit << "static const AC::Specifiers spec = (AC::Specifiers)("
          << spec_str (false, false, fi->isVirtual () || fi->overridesVirtual ()) << "); ";
    *unit << "};" << endl;
    e++;
  }
  *unit << "    enum { FUNCTIONS = " << functions << " };" << endl;
  *unit << "    enum { CONSTRUCTORS = " << constructors << " };" << endl;
  *unit << "    enum { DESTRUCTORS = " << destructors << " };" << endl;

  *unit << "  };" << endl;
  *unit << endu;
  
//  if (ci->Name()) cout << "Class: " << ci->Name() << endl;
//  cout << "Unit:" << endl << *unit << endl;
  units.push_back (unit);
  CTree *tree = parse (units, &CCSyntax::member_spec, CT_MembList::NodeId (), err);
  if (tree) {
    // move the introduced members into the normal member list
    CT_MembList *memb_list = (CT_MembList*)tree;
    for (int e = 0; e < memb_list->Entries (); e++) {
      CTree *entry = memb_list->Entry (e);
      cd->Members ()->InsertSon (cd->Members ()->Sons () - 1, entry);
    }
    _code_weaver.insert (pos, unit);
    delete memb_list;
  }
  else {
    err << sev_error << inspos.location()
        << "parsing introspection code for class '" << ci->QualName()
        << "' failed" << endMessage;
  	delete unit;
  }
  // paste a #line directive
  LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();
  ACUnit *dunit = new ACUnit (err);
  lmgr.directive (*dunit, inspos.unit (), inspos);
  *dunit << endu;
  if (dunit->empty ()) delete dunit; else _code_weaver.insert (pos, dunit);
}


// checks if an attribute that us returned by the parser is an attribute
// in the sense of the AspectC++ introspection mechnism
bool PumaIntroducer::is_attribute (CAttributeInfo *obj) {
  if (obj->isAnonymous () ||
      obj->EnumeratorInfo () ||
      // TODO: temporary hack - attributes that have an anonymous type
      //       shall no be ignored in the type list!
      (obj->TypeInfo () &&
       (obj->TypeInfo ()->isBitField() ||
        (obj->TypeInfo ()->Record () && obj->TypeInfo ()->Record ()->isAnonymous ()) ||
        (obj->TypeInfo ()->EnumInfo () && obj->TypeInfo ()->EnumInfo ()->isAnonymous ()))))
    return false;
  return true;
}


// called after the parser tried to parse a base clause
void PumaIntroducer::base_clause_end (CT_ClassDef *cd, Token *open) {
  enter ();
  assert (cd);
  CClassInfo *ci = (CClassInfo*)cd->Object ()->DefObject ();
  ACErrorStream &err = ci->SemDB ()->Project ()->err ();
  ACToken inspos = ACToken (open);

  // first check with the plan if there are intros for this class
  ACM_Class *jpl = plan_lookup (ci);
  if (!jpl) {
    leave ();
    return;
  }
  
  // create a unit with the code that shall be introduced
  LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();

  // collect the intros
  list<Unit*> units;
  gen_base_intros (jpl, units, err, ci, lmgr);

  if (units.size () > 0) {
    // parse the introduced code as a base clause
    CTree *tree = parse (units, &CCSyntax::base_clause,
    		             CT_BaseSpecList::NodeId (), err);
    if (tree) {
      cd->BaseIntros (tree);
      // manipulate the code (will be effective after commit),
      // paste declaration before "{" of the class definition
      const WeavePos &pos = _code_weaver.weave_pos (inspos, WeavePos::WP_BEFORE);
      // paste the generated code and add the unit to the manipulator's unit mgr.
      if (cd->BaseClasses () != 0) {
        // get the first introduced base class unit
        Unit *first_unit = units.front ();
        // delete the ":" token at the beginning of the unit
        first_unit->remove ((ListElement*)first_unit->first ());
        // insert "," instead
        ACUnit comma (err);
        comma << "," << endu;
        first_unit->move_before ((ListElement*)first_unit->first (), comma);
      }
      // now paste all units
      for (list<Unit*>::iterator ui = units.begin ();
        ui != units.end (); ++ui)
        _code_weaver.insert (pos, *ui);
      // paste a #line directive
      ACUnit *dunit = new ACUnit (err);
      lmgr.directive (*dunit, inspos.unit (), inspos);
      *dunit << endu;
      if (dunit->empty ()) delete dunit; else _code_weaver.insert (pos, dunit);
    }
    else {
      err << sev_error << inspos.location()
          << "base class introduction into '" << ci->QualName() << "' failed"
          << endMessage;
      // delete all base into units
      for (list<Unit*>::iterator ui = units.begin (); ui != units.end (); ++ui)
        delete *ui;
    }
  }
  leave ();
}


// called after the program has been parsed completely
void PumaIntroducer::trans_unit_end (CT_Program *pr) {
  // ignore this translation unit, if it is a nested parser run
  if (_intro_level != 0)
    return;
  enter ();
    
  CFileInfo *fi = pr->Scope ()->FileInfo ();
  assert (fi);
  CSemDatabase &db = *fi->SemDB ();
  ACProject &project = (ACProject&)*db.Project ();
  ACErrorStream &err = project.err ();
  const Unit *primary = fi->Primary ();
  LineDirectiveMgr &lmgr = _code_weaver.line_directive_mgr ();
  
  // ... some unit for formatting
  ACUnit ws (err); ws << " " << endu;

  // loop until there are no more entries in the target map
  while (!_targets.empty ()) {
    
    // get the information from the first entry and delete it
    TargetMap::iterator i = _targets.begin ();
    CClassInfo *ci     = i->first;
    ACM_Class &jp_loc  = *i->second;
    _targets.erase (i);
    
    // create units with the code that shall be introduced
    list<Unit*> intros;
    gen_intros (&jp_loc, intros, err, ci, lmgr, _conf.introduction_depth (), true);
    // ignore this class if there are no non-inline intros for it
    if (intros.size () == 0)
      continue;
    
    CSourceInfo *si = ci->SourceInfo ();
    bool in_header = (strcmp (si->FileName (),
                              si->FileInfo ()->Primary ()->name ()) != 0);
                              
    if (in_header) {
      // check if there is a link-once code element in the class
      CObjectInfo *loo = link_once_object (ci);
      
      // TODO: for now loo has to != 0, later we can exploit the proj. repo
      if (!loo) {
        err << sev_warning << TransformInfo::location (jp_loc)
            << "cannot introduce non-inline function or static attribute"
            << " into \"class " << signature (jp_loc).c_str ()
            << "\". It has to contain link-once code." << endMessage;
        for (list<Unit*>::iterator i = intros.begin (); i != intros.end (); ++i)
          delete *i;
        continue;
      }
      
      // continue silently if this is only a declaration
      if (loo->Scope () == ci) {
        for (list<Unit*>::iterator i = intros.begin (); i != intros.end (); ++i)
          delete *i;
        continue;
      }
    }
      
    // parse the introduced code
    CTree *tree = parse (intros, &CCSyntax::decl_seq,
                         Builder::Container::NodeId (), err);
    if (!tree) {
      err << sev_error << "parsing non-inline introduction code for class '"
          << ci->QualName() << "' failed" << endMessage;
      for (list<Unit*>::iterator i = intros.begin (); i != intros.end (); ++i)
        delete *i;
      leave ();
      return;
    }
    
    // move the introduced members into the normal program node
    Builder::Container *decls = (Builder::Container*)tree;
    for (int e = 0; e < decls->Entries (); e++) {
      CTree *entry = decls->Entry (e);
      pr->AddSon (entry);
    }
    delete decls;
    
    // paste the generated code and add the unit to the manipulator's unit mgr.
    for (list<Unit*>::iterator i = intros.begin (); i != intros.end (); ++i)
      _code_weaver.insert (_code_weaver.footer_pos (), *i, true);

    // determine the units that should be included in front of the intros
    ACM_ClassPlan *plan = jp_loc.get_plan ();
    set<const Unit*> units;
    typedef ACM_Container<ACM_MemberIntro, true> MContainer;
    for (MContainer::iterator i = plan->get_member_intros().begin();
        i != plan->get_member_intros().end(); ++i) {
      ACM_ClassSlice *cs = get_slice (*(*i)->get_intro());
      list<Unit*> &member_units = TI_ClassSlice::of(*cs)->non_inline_member_units();
      for (list<Unit*>::iterator i = member_units.begin ();
          i != member_units.end (); ++i)
        units.insert (*i);
    }
    
    // parse the aspect headers that are needed by this intro
    for (set<const Unit*>::iterator iter = units.begin ();
      iter != units.end (); ++iter) {
      const Unit *slice_unit = *iter;
      if (slice_unit != primary) {
//        cout << "new edge from " << primary->name () << " to "
//             << slice_unit->name () << endl;
        _ig.add_edge (primary, slice_unit);
        // generate a unit with the include
        ostringstream includes;
        Filename incname = project.getRelInclString (primary->name (), slice_unit->name ());
        includes << endl << "#ifndef ";
        Naming::guard (includes, (FileUnit*)slice_unit);
        includes << endl << "#define ";
        Naming::guard (includes, (FileUnit*)slice_unit);
        includes << endl;
        includes << "#include \"" << incname << "\"" << endl;
        includes << "#endif" << endl;
        string inc (includes.str ());
        _code_weaver.insert (_code_weaver.footer_pos (), inc, true);

        if (_included_aspect_headers.find (slice_unit) ==
            _included_aspect_headers.end ()) {
          _included_aspect_headers.insert (slice_unit);
          // prepare a new C preprocessor
          TokenStream stream;           // linearize tokens from several files
          stream.push ((Unit*)slice_unit);
          PreprocessorParser cpp (&project.err (), &project.unitManager (),
            _cpp->locals ());
          PreMacroManager *old_mm = cpp.macroManager ();
          cpp.macroManager (_cpp->macroManager ());
          cpp.stream (&stream);
          cpp.configure (project.config ());
      
          TokenProvider provider (cpp);        // look ahead token buffer
          CTree *tree = _parser->syntax ().run (provider, &CCSyntax::trans_unit);
          // TODO: tree should later be freed!
          _ah_trees.push_back (tree);
        
          // print all error messages
          _parser->builder ().errors (err);
          cpp.macroManager (old_mm);
        }
//        else
//          cout << "'" << slice_unit->name () << "' not included again!" << endl;
      }
    }
  }
  leave ();
}


// manage the intro nesting level and the _cpp pointer
void PumaIntroducer::enter () {
  if (_intro_level == 0)
    _cpp = &(PreprocessorParser&)_parser->syntax ().provider ()->source ();
  _intro_level++;
}
void PumaIntroducer::leave () {
  _intro_level--;
  if (_intro_level == 0)
    _cpp = 0;
}

// create the weaving plan for a given class
ACM_Class *PumaIntroducer::create_plan (CClassInfo *ci) {
  
  // return if this class is not an introduction target
  if (!_jpm.is_intro_target (ci->DefObject ()))
    return 0;
    
  // try to register this class (might be a newly introduced class)
  ACM_Class *jpl = _jpm.register_aspect (ci);
  if (!jpl) jpl = _jpm.register_class (ci, 0, false);
  
  // return if this is either not a valid model class or no valid intro target
  if (!jpl || !jpl->get_intro_target ())
    return 0;
  
  // iterate through all introduction advice in the plan
  PointCutContext context (_jpm, _conf);
  const list<IntroductionInfo*> &intros = _plan.introduction_infos ();
  for (list<IntroductionInfo*>::const_iterator i = intros.begin ();
       i != intros.end (); ++i) {
    IntroductionInfo *intro = *i;
    // TODO: consider stand-alone advice here as well in the future (C-mode)
    // something like ... if (!intro->is_activated ()) continue;
    context.concrete_aspect (intro->aspect ());
    Binding binding;     // binding and condition not used for intros
    Condition condition;
    PointCutExpr *pce = (PointCutExpr*)intro->pointcut_expr().get();
    if (pce->match (*jpl, context, binding, condition))
      _plan.consider (*jpl, &intro->intro ());
  }
  
  if (jpl->has_plan ()) {

    // order the advice & check
    _plan.order (jpl);
    
    // remember the class info and join point location
    _targets.insert (TargetMap::value_type (ci, jpl));
  }
  return jpl;
}
  

ACM_Class *PumaIntroducer::plan_lookup (CClassInfo *ci) {
  TargetMap::iterator i = _targets.find (ci);
  if (i != _targets.end () && i->second->has_plan())
    return i->second;
  return 0;
}


CObjectInfo *PumaIntroducer::link_once_object (CClassInfo *ci) {
  for (unsigned i = 0; i < ci->Functions (); i++) {
    CFunctionInfo *fi = ci->Function (i)->DefObject ();
    // skip template functions and built-in functions
    // they don't need link-once code
    if (fi->isBuiltin () || fi->isTemplate () || is_intro (fi)) {
      continue;
    }
    // if a member function is undefined it is link-once code!
    if (!fi->isDefined ()) {
      return fi;
    }
    // if the function is defined, outside the class scope, and is not inline,
    // we found the implementation
    if (fi->Scope () != ci && !fi->isInline ()) {
      return fi;
    }
  }
  for (unsigned i = 0; i < ci->Attributes (); i++) {
    CAttributeInfo *ai = ci->Attribute (i)->DefObject ();
    // ignore introduced attributes
    if (is_intro (ai))
      continue;
    // if the scope is outside the class, we or definition
    if (ai->Scope () != ci) {
      return ai;
    }
    // initialized, we can us this object
    if (ai->isStatic () && !ai->Init ()) {
      return ai;
    }
  }
  return 0;
}

bool PumaIntroducer::is_intro (CObjectInfo *obj) {
  Unit *unit = obj->SourceInfo ()->SrcUnit ();
  return (IntroductionUnit::cast (unit) != 0);
}

void PumaIntroducer::gen_intros (ACM_Class *jpl, list<Unit*> &units,
    ACErrorStream &err, CStructure *target, LineDirectiveMgr &lmgr, int introduction_depth,
  bool non_inline) const {

  // ... some unit for formatting
  ACUnit nl (err);
  nl << endl << endu;
  ACUnit ws (err);
  ws << " " << endu;

  // handle all intros
  typedef ACM_Container<ACM_MemberIntro, true> Container;
  Container &intros = jpl->get_plan()->get_member_intros();
  int i = 0;
  for (Container::iterator iter = intros.begin(); iter != intros.end(); ++iter, ++i) {
    ACM_Introduction *ii = (*iter)->get_intro();

    // create the new unit
    IntroductionUnit *unit =
      new IntroductionUnit (err, (Unit*)target->Tree ()->token ()->belonging_to ());
    unit->intro (ii);
    unit->precedence (i);

    // at least on token is needed in the unit for 'move' (see below)
    unit->append (*((Token*)ws.first ())->duplicate ());

    // TODO: clean up; a lot of duplicated code here
    TI_ClassSlice *ti = TI_ClassSlice::of (*get_slice (*ii));

    // generate non-inline introduction instance
    if (ti->non_inline_members ().size () > 0 && non_inline) {
      // create a unit with the target class name
      ACUnit target_name (err);
      target_name << target->Name () << endu;
      ACUnit target_qual_name (err);
      target_qual_name << target->QualName () << endu;
      // generate a token with the name of the JoinPoint class for this slice
      ACUnit jp_name (err);
      jp_name << "__TJP_" << unit->precedence() << endu;

      std::list<Unit*> &units = ti->non_inline_members ();
      for (std::list<Unit*>::iterator i = units.begin (); i != units.end(); ++i) {
        Unit *pattern = *i;
        int name_tokens = 0;
        Token *token = pattern->first();
        token = pattern->next (token); // skip 'slice' keyword
        while (token) {
          Token *next = pattern->next (token);
          if (name_tokens == 0 && next && next->type() == 0) {
            name_tokens = 1;
            next = pattern->next (next);
          }
          else if (name_tokens > 0 && token->type() == 0) {
            unit->append (*((Token*)ws.first ())->duplicate ());
            *unit += target_qual_name;
            name_tokens = 0;
            if (next && next->type () == TOK_COLON_COLON) {
              token = next;
              next = pattern->next (token);
              unit->append (*token->duplicate());
              if (next && next->type () == TOK_TILDE) {
                token = next;
                next = pattern->next (token);
                unit->append (*token->duplicate());
              }
              if (next && next->type () == TOK_ID) {
                token = next;
                next = pattern->next (token);
                if (get_slice (*ii)->get_name() == token->text()) {
                  *unit += target_name;
                }
                else
                  unit->append (*token->duplicate());
              }
            }
          }
          else {
            if (name_tokens == 0) {
              unit->append (*((Token*)ws.first ())->duplicate ());
              if (token->type () == TOK_ID &&
                  string ("JoinPoint") == token->text ()) {
                unit->append (*jp_name.first ()->duplicate());
                unit->jp_needed (true);
              }
              else {
                unit->append (*token->duplicate());
              }
            }
            else
              name_tokens++;
          }
          token = next;
        }
      }
//          cout << "Generated intro instance " << *unit << endl;
    }
    // generate inline introduction instance
    if (ti->get_tokens() && !non_inline) {
      // create a unit with the target class name
      ACUnit target_name (err);
      target_name << target->Name () << endu;
      // generate a token with the name of the JoinPoint class for this slice
      ACUnit jp_name (err);
      jp_name << "__TJP_" << unit->precedence() << endu;

      ACUnit slice_start (err);
      if (get_slice(*ii)->get_is_struct())
        slice_start << "  public:" << endl;
      else
        slice_start << "  private:" << endl;
      // add "  typedef <target-name> <slice-name>;\n"
      if (get_slice(*ii)->get_name()[0] != '<') {
        slice_start << "  typedef " << target->Name () << " "
                    << get_slice(*ii)->get_name() << ";" << endl;
      }
      slice_start << endu;
      unit->List::move ((Token*)unit->last (), slice_start);

      Unit *pattern = ti->get_tokens();
      Token *token = pattern->first();
      while (token->type () != TOK_OPEN_CURLY)
        token = pattern->next (token);
      token = pattern->next (token);
      ACUnit dir (err);
      lmgr.directive (dir, pattern, ACToken (token));
      dir << endu;
      if (!dir.empty ())
        unit->List::move ((Token*)unit->last (), dir);
      Location loc = token->location ();
      int level = 1;
      while (true) {
        if (token->type () == TOK_CLOSE_CURLY) {
          level--;
          if (level == 0)
            break;
        }
        else if (token->type () == TOK_OPEN_CURLY)
          level++;

        // insert whitespace or newline for proper formatting
        if (token->location ().line () != loc.line ()) {
          for (int l = loc.line (); l < token->location ().line (); l++)
            unit->append (*((Token*)nl.first ())->duplicate ());
          loc = token->location ();
        }
        else
          unit->append (*((Token*)ws.first ())->duplicate ());

        // TODO: this is a hack - make sure not to replace too often!
        if (level == 1 && token->type () == TOK_ID &&
            get_slice (*ii)->get_name() == token->text ())
          unit->append (*target_name.first ()->duplicate());
        else if (token->type () == TOK_ID &&
            string ("JoinPoint") == token->text ()) {
          unit->append (*jp_name.first ()->duplicate());
          unit->jp_needed (true);
        }
        else
          unit->append (*token->duplicate());
        token = pattern->next (token);
      }
//          cout << "Generated intro instance " << *unit << endl;
    }

    // if there was no introduction, delete the unit -> no result
    if (unit->first () == unit->last ())
       delete unit;
     // check whether this is a deeply nested introduction
    else if (unit->nesting_level () > introduction_depth) {
      err << sev_error << target->DefObject()->Tree ()->token ()->location ()
          << "maximum level of nested introductions (" << introduction_depth
          << ") for class '"
          << target->QualName () << "' exceeded" << endMessage;
      err << sev_error
          << Location (Filename (filename (*ii).c_str ()), line (*ii))
          << "invalid introduction defined here" << endMessage;
      delete unit;
    }
    else
      units.push_back (unit);
  }
}

void PumaIntroducer::gen_base_intros (ACM_Class *jpl, list<Unit*> &units,
    ACErrorStream &err, CClassInfo *target, LineDirectiveMgr &lmgr) const {

  typedef ACM_Container<ACM_BaseIntro, true> Container;
  Container &bases = jpl->get_plan()->get_base_intros();
  bool first = true;
  for (Container::iterator i = bases.begin(); i != bases.end (); ++i) {
    // get the current introduction
    ACM_Introduction *ii = (*i)->get_intro();

    // create the new unit
    IntroductionUnit *unit =
      new IntroductionUnit (err, (Unit*)target->Tree ()->token ()->belonging_to ());
    unit->intro (ii);

    // generate the code for this base class introduction
    gen_base_intro (*unit, ii, first);
    first = false;

    // store the result for the caller
    units.push_back (unit);
  }
}

void PumaIntroducer::gen_base_intro (IntroductionUnit &intro_unit,
  ACM_Introduction *ii, bool first) const {
  TI_ClassSlice *ti = TI_ClassSlice::of (*get_slice(*ii));
  // generate the code for this entry
  intro_unit << (first ? ": " : ", ") << ti->base_intro() << endu;
}
