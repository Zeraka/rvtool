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

#include <stdio.h>
#include <stdlib.h>

#include "Repository.h"
#include "version.h"
#include "PointCut.h"
#include "TransformInfo.h"
#include "OrderInfo.h"
#include "AdviceInfo.h"
#include "AspectInfo.h"
#include "RepoXMLDoc.h"

using namespace Puma;

void Repository::open (const char *name, ACErrorStream &err) {

  RepoXMLDoc doc;

  create (name);

  if (!doc.load (name) || !doc.root ().has_name ("ac-project")) {
    err << sev_error << "project repository '" << name << "' cannot be opened" 
      " or is invalid" << endMessage;
    return;
  }

  string version = doc.root ().get_str_prop ("version");
  if (version != ac_version ()) {
    err << sev_warning << "project file version '" 
        << version.c_str () << "' differs from ac++ version" << endMessage;
  }

  RepoXMLNode joinpoints, files, aspects, adv;
  
  // iterate the top-level project elements
  for (RepoXMLNode::iter curr = doc.root ().first_child ();
       curr != doc.root ().end_child (); ++curr) {
    if ((*curr).has_name ("joinpoint-list"))
      joinpoints = *curr;
    else if ((*curr).has_name ("file-list"))
      files = *curr;
    else if ((*curr).has_name ("aspect-list"))
      aspects = *curr;
    else if ((*curr).has_name ("advice-list"))
      adv = *curr;
  }
  
  if (!joinpoints || !files || !aspects || !adv) {
    err << sev_error << "invalid project file, missing list" << endMessage;
    return;
  }

  // now read all join points, files, and aspects
  _jprepo.get_xml (joinpoints);
  _advrepo.get_xml (adv);
  _asprepo.get_xml (aspects);
  _frepo.get_xml (files);
}

void Repository::create (const char *name) {
  _name           = name;
}

void Repository::save (ACErrorStream &err) {

  // create an XML DOM
  RepoXMLDoc doc;
  doc.create ("ac-project");
  doc.root ().set_str_prop ("version", ac_version ());

  // handle join points, files, and aspects
  _jprepo.make_xml (doc.root ());
  _advrepo.make_xml (doc.root ());
  _asprepo.make_xml (doc.root ());
  _frepo.make_xml (doc.root ());

  // save the file
  if (!doc.save (_name)) {
    err << sev_error << "Saving repository '" << _name << "' failed"
	<< endMessage;
  }
}

void Repository::close () {
  _name = (const char *)0;
}

ACM_Source *Repository::source_loc (ACM_Any &jpl) {
  ACM_Source *src = 0;
  typedef ACM_Container<ACM_Source, true> Container;
  Container &locs = (jpl.type_val () &
    (JPT_Execution|JPT_Construction|JPT_Destruction)) ?
    ((ACM_Any*)jpl.get_parent ())->get_source() : jpl.get_source();
  for (Container::iterator i = locs.begin (); i != locs.end (); ++i) {
    if ((*i)->get_kind () != SLK_DECL) {
      src = *i;
      break;
    }
  }
  return src;
}

Repository::REPO_ID Repository::consider (ACM_File &file) {
  return _frepo.insert (file.get_filename ().c_str (), file.get_len (), _primary);
}

Repository::REPO_ID Repository::consider (ACM_Any &jpl, int adv) {
  ACM_Source *src = source_loc (jpl);
  if (!src)
    return -1;
  REPO_ID file_id = consider (*src->get_file());
  string sig = "**unknown**";
  if (jpl.type_val () & JPT_Code)
    sig = signature ((ACM_Code&)jpl);
  else if (jpl.type_val () & JPT_Name)
    sig = signature ((ACM_Name&)jpl);
  // TODO: deal with 'else' here -> fatal error?
  // map type string to old-style
  string type_str = "unsupported";
  if (string(jpl.type_str ()) == "Execution")
    type_str = "exec";
  else if (string(jpl.type_str ()) == "Call")
    type_str = "call";
  else if (string(jpl.type_str ()) == "Construction")
    type_str = "construction";
  else if (string(jpl.type_str ()) == "Destruction")
    type_str = "destruction";
  else if (string(jpl.type_str ()) == "Class")
    type_str = "class";
  else if (string(jpl.type_str ()) == "Aspect")
    type_str = "class";

  REPO_ID jp_id   = _jprepo.insert (file_id, src->get_line (), sig,
                                    type_str, adv, src->get_len ());
  return jp_id;
}

Repository::REPO_ID Repository::consider (ACM_Aspect &jpl) {
  ACM_Source *src = source_loc (jpl);
  if (!src)
    return -1;
  REPO_ID file_id = consider (*src->get_file());
  return _asprepo.insert (file_id, src->get_line (), signature (jpl).c_str());
}

Repository::REPO_ID Repository::consider (ACM_Introduction &intro) {
  ACM_Source *src = source_loc (intro);
  if (!src)
    return -1;
  // remember aspect and advice information in the project repository
  REPO_ID aspect_id = consider (*(ACM_Aspect*)intro.get_parent ());
  REPO_ID file_id   = consider (*src->get_file());
  REPO_ID advice_id = _advrepo.insert (file_id, src->get_line (),
    intro.type_str (), aspect_id, src->get_len ());
  return advice_id;
}

void Repository::setup (ACM_TUnit* prim_unit) {
  _frepo.noref ();
  _jprepo.noref ();
  _advrepo.noref ();
  _asprepo.noref ();
  _primary = -1;
  _primary = consider (*prim_unit);
}

void Repository::cleanup () {
  set<int> dep_files;
  _frepo.dependent (_primary, dep_files);
  _jprepo.cleanup (dep_files);
  _advrepo.cleanup (dep_files);
  _asprepo.cleanup (dep_files);
  _frepo.cleanup (_primary);
}

void Repository::update (ACM_Introduction &intro, ACM_Class &cls) {
  // remember aspect information in the project repository
  REPO_ID advice_id = consider (intro);
  REPO_ID jp_id = consider (cls, advice_id);
  if (_set_jpid)
    cls.set_jpid (jp_id); // TODO: set JPID not what we really need here
}

void Repository::update (AdviceInfo &ai, PointCut &target) {
  // remember aspect information in the project repository
  REPO_ID aspect_id = consider (ai.aspect ());
  ACM_Source *src = source_loc (ai.code ());
  if (!src)
    return;
  REPO_ID file_id   = consider (*src->get_file());
  // determine advice type (before/after/around) as string
  string type_str = "unsupported";
  switch (ai.code ().get_kind ()) {
  case ACT_BEFORE: type_str = "before"; break;
  case ACT_AFTER:  type_str = "after";  break;
  case ACT_AROUND: type_str = "around"; break;
  }
  REPO_ID advice_id = _advrepo.insert (file_id, src->get_line (),
    type_str, aspect_id, src->get_len ());
  
  for (PointCut::iterator iter = target.begin ();
       iter != target.end (); ++iter) {
    ACM_Any &jpl = *((*iter).location ());
    ACM_Source *src = source_loc (jpl);
    if (!src)
      continue;
    REPO_ID jp_id = consider (jpl, advice_id);
    if (_set_jpid)
      jpl.set_jpid (jp_id); // TODO: set JPID not what we really need here
  }
}

void Repository::update (OrderInfo &oi) {
  // remember order advice information in the project repository
  REPO_ID aspect_id = consider (oi.aspect ());
  ACM_Source *src = source_loc (oi.order ());
  if (!src)
    return;
  REPO_ID file_id = consider (*src->get_file());
  /* REPO_ID advice_id = */ _advrepo.insert (file_id, src->get_line (),
    "order", aspect_id, src->get_len ());
}
