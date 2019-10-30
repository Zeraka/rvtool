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

#ifndef __Repository_h__
#define __Repository_h__

#include <map>
#include <string>
using namespace std;
#include <stdio.h>

#include "FileRepo.h"
#include "AspectRepo.h"
#include "AdviceRepo.h"
#include "JoinPointRepo.h"
#include "TransformInfo.h"
#include "ACErrorStream.h"

class PointCut;
class AspectInfo;
class OrderInfo;
class AdviceInfo;
class ACM_Any;
class ACM_Introduction;
class ACM_Aspect;
class ACM_Class;
class ACM_Advice;

class Repository {
  typedef int REPO_ID;

  bool _set_jpid;
  const char *_name;
  REPO_ID _primary;

  FileRepo _frepo;
  AspectRepo _asprepo;
  AdviceRepo _advrepo;
  JoinPointRepo _jprepo;

  ACM_Source *source_loc (ACM_Any &jpl);

public:
  Repository (bool set_jpid) : _set_jpid (set_jpid) { _name = (const char *)0; }
  ~Repository () { close (); }
  bool initialized () const { return _name != (const char*)0; }
  void open (const char *name, ACErrorStream &err);
  void create (const char *name);
  void save (ACErrorStream &err);
  void close ();
  void setup (ACM_TUnit* prim_unit);
  REPO_ID consider (ACM_File &file);
  REPO_ID consider (ACM_Any &jpl, int adv);
  REPO_ID consider (ACM_Introduction &intro);
  REPO_ID consider (ACM_Aspect &ai);
  void update (ACM_Introduction &intro, ACM_Class &cls);
  void cleanup ();
  void update (AdviceInfo &ii, PointCut &target);
  void update (OrderInfo &oi);
};

#endif // __Repository_h__
