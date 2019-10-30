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

#ifndef __AspectIncludes_h__
#define __AspectIncludes_h__

#include <set>
using std::set;
#include <map>
using std::map;
#include <string>
using std::string;

#ifdef FRONTEND_CLANG
#include "clang/Basic/SourceManager.h"
#endif

namespace Puma {
  class Unit;
  class CProject;
} // namespace Puma

#include "ACFileID.h"

class AspectInfo;
class ACM_Aspect;
struct BackEndProblems;
class ACProject;

// this class manages all #includes and forward declarations, which have to be
// generated in order to make the aspects visible at the join points they affect

struct AspectRef {
  AspectInfo *_aspect;
  // Kind of dependency: the value defines the importance
  enum Kind { AR_UNKNOWN = 0, AR_DECL = 1, AR_ADVICE = 2 } _kind;
  bool operator < (const AspectRef &cmp) const {
    return _aspect < cmp._aspect;
  }
  bool operator == (const AspectRef &cmp) const {
    return _aspect == cmp._aspect;
  }
  AspectRef (AspectInfo *a, Kind k) : _aspect (a), _kind (k) {}
};

typedef set<AspectRef> AspectRefSet;
typedef map<ACFileID, AspectRefSet> UnitAspectRefMap;

class AspectIncludes : private UnitAspectRefMap {

  ACProject &_project;

  // helper function that find the file (id) in which an aspect 'ai' is defined
  ACFileID aspect_unit (ACM_Aspect *a) const;
  
public:
  
  AspectIncludes (ACProject &prj) : _project (prj) {}

  // this function declares that a unit 'iu' depends on the aspect 'a'
  // ('kind' specifies the kind of dependency, i.e. class def, decl, or advice)
  void insert (ACFileID iu, AspectInfo *a, AspectRef::Kind kind);
  
  // iterator type and functions to access the elements (only const access)
  typedef UnitAspectRefMap::const_iterator const_iterator;
  const_iterator begin () const { return UnitAspectRefMap::begin (); }
  const_iterator end () const { return UnitAspectRefMap::end (); }

  // searching ...
  using UnitAspectRefMap::find;
  // get the target unit associated with an entry
  ACFileID unit (const_iterator entry) const { return (*entry).first; }

  // generate the #include directives for a given unit
  string generate (const_iterator entry, const BackEndProblems &bep) const;
};

#endif // __AspectIncludes_h__
