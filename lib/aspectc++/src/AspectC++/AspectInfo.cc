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

#include "AspectInfo.h"
#include "AdviceInfo.h"
//
//#include "Puma/CFunctionInfo.h"
//
//// for workaround
//#include "Puma/CSemDatabase.h"
//#include "Puma/CTree.h"
//
//CFunctionInfo *AspectInfo::aspectof () {
//  // make this better!
//  CFunctionInfo *aof = ClassInfo ()->Function ("aspectof");
//  if (!aof)
//    aof = ClassInfo ()->Function ("aspectOf");
//  return aof ? aof->DefObject () : (CFunctionInfo*)0;
//
//}
//
// generate the invocation function forward declarations
string AspectInfo::ifct_decls (const BackEndProblems &bep) const {

  // name of this aspect
  string me (name ());
  
  string decls = string ("\n") +
    "#ifndef __ac_fwd_" + me + "__\n"
    "#define __ac_fwd_" + me + "__\n"
    "class " + me + ";\n"
    "namespace AC {\n";

  stringstream ifct_decls;
  for (list<AdviceInfo*>::const_iterator iter = _advice_infos.begin ();
       iter != _advice_infos.end (); ++iter) {
    (*iter)->gen_invocation_func (ifct_decls, false, bep);
  }
  decls += ifct_decls.str ();
  
  decls += "}\n";
  decls += "#endif\n\n";
    
  return decls;
}

// generate the invocation function definitions
string AspectInfo::ifct_defs (const BackEndProblems &bep) const {
  
  stringstream defs;
  defs << endl;
  defs << "namespace AC {" << endl;
  for (list<AdviceInfo*>::const_iterator iter = _advice_infos.begin ();
       iter != _advice_infos.end (); ++iter) {
    (*iter)->gen_invocation_func (defs, true, bep);
  }
  defs << "} " << endl;
  return defs.str ();
}
