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

#ifndef __Naming_h__
#define __Naming_h__

#include <iostream>
#include <string>
using namespace std;
// This class encapsulates the naming scheme for generated functions etc.

class ACM_Any;
class ACM_Code;
class ACM_Access;
class ACM_Call;
class ACM_Execution;
class ACM_Aspect;
class ACM_Class;
class ACM_Function;
class ACM_Name;
class ACM_Type;
class ACM_Arg;

#include "ACFileID.h"

class AdviceInfo;
class AspectInfo;

class Naming {
  static bool is_unary_op (ACM_Function *func);
  static void scope_name (ostream &out, ACM_Name &scope);
  
public:
  static void op_name (ostream &out, ACM_Function *func);
  static void constr_name (ostream &out, ACM_Function *func);
  static void destr_name (ostream &out, ACM_Function *func);
  static void mangle (ostream &out, ACM_Name *obj);
  static void mangle (ostream &out, ACM_Type *type);
  static void mangle (ostream &out, const ACM_Arg *arg);
  static void bypass_id_class (ostream& out, ACM_Call *jpl);
  static void access_wrapper (ostream& out, ACM_Access *jpl, unsigned depth, int wrapper_number = -1);
  static void exec_inner (ostream& out, ACM_Code *jpl);
  static void action_wrapper (ostream& out, ACM_Any *loc, unsigned depth);
  static void exec_advice (ostream& out, ACM_Execution *jpl, AdviceInfo *ad);
  static void call_advice (ostream& out, ACM_Call *jpl, AdviceInfo *ad);
  static void local_id (ostream& out, ACM_Code *jpl);
  static void tjp_struct(ostream& out, ACM_Code *loc, int depth);
  static void tjp_instance(ostream& out, ACM_Code *loc);
  static void tjp_args_array(ostream& out, ACM_Code *loc);
  static void tjp_argtypes(ostream& out, ACM_Code *loc);
  static void cflow (ostream& out, ACM_Aspect &jpl_aspect, int index);
  static string bypass_caller_class (ACM_Class *cls);
  static void tjp_typedef (ostream& out, const char *name);
  static void type_check_func (ostream &out, ACM_Class &in, const string &name);
  static void guard (ostream &out, ACFileID unit);
  static void mangle_file (ostream &out, ACFileID file_id);
  static void mangle_file (ostream &out, const char *name);
  
  // checks
  static bool is_tjp_object (const char *candidate);
};

#endif // __Naming_h__
