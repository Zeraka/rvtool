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

#include "AspectIncludes.h"
#include "AspectInfo.h"
#include "Naming.h"
#include "TransformInfo.h"
#include "ACProject.h"

#ifdef FRONTEND_CLANG
#include "clang/Frontend/CompilerInstance.h"
#else
#include "Puma/Unit.h"
#include "Puma/CTree.h"
#include "Puma/CProject.h"
#include "Puma/Filename.h"
#endif

// helper function that find the unit in which an advice code is defined
ACFileID AspectIncludes::aspect_unit (ACM_Aspect *a) const {
#ifdef FRONTEND_CLANG
  clang::SourceManager &sm = _project.get_compiler_instance ()->getSourceManager ();
  clang::FileID fid = sm.getFileID(TransformInfo::decl(*a)->getLocation());
  return sm.getFileEntryForID (fid);
}
#else
  return TI_Aspect::unit (*a);
}
#endif

// this function declares that a unit 'iu' depends on the aspect 'ai'
void AspectIncludes::insert (ACFileID iu, AspectInfo *a, AspectRef::Kind kind) {
  assert (iu.is_valid());

  // check if there is already an entry for this unit
  iterator entry = find (iu);
  if (entry == end ()) {
    // there is no entry for the unit 'iu' yet => make a new one
    AspectRefSet new_set;
    new_set.insert (AspectRef (a, kind));
    UnitAspectRefMap::insert (value_type (iu, new_set));
  }
  else {
    // entry found add the aspect reference to the stored set
    AspectRefSet &set = (*entry).second;
    AspectRefSet::iterator ref_entry =
      set.find (AspectRef (a, AspectRef::AR_UNKNOWN));
    if (ref_entry == set.end ()) {
      // simply insert a new entry if the aspect has not been found
      set.insert (AspectRef (a, kind));
    }
    else {
      // a definition request dominates a declaration request...
      AspectRef &ref = (AspectRef&)(*ref_entry);
      if (kind > ref._kind)
        ref._kind = kind;
    }
  }
}

// generate the #include directives for a given unit
string AspectIncludes::generate (const_iterator entry,
				 const BackEndProblems &bep) const {
  string includes;
  const AspectRefSet &set = (*entry).second;
  for (AspectRefSet::const_iterator iter = set.begin ();
    iter != set.end (); ++iter) {

    switch ((*iter)._kind) {
      case AspectRef::AR_DECL: 
        includes += "class ";
        includes += (*iter)._aspect->name ();
        includes += ";\n";
        break;
      case AspectRef::AR_ADVICE:
        {
          includes += (*iter)._aspect->ifct_decls (bep);
          ACFileID unit = aspect_unit (&(*iter)._aspect->loc ());
          stringstream define;
          define << "#ifndef __ac_need_";
          Naming::mangle_file (define, /*(Puma::FileUnit*)*/unit);
          define << endl << "#define __ac_need_";
          Naming::mangle_file (define, /*(Puma::FileUnit*)*/unit);
          define << endl << "#endif" << endl;
          includes += define.str ();          
        }
        break;
      default:
        cout << "aspect ref type not yet implemented" << endl;
    }
  }
  return includes;
}
