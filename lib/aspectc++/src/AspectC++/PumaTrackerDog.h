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

#ifndef __PumaTrackerDog_h__
#define __PumaTrackerDog_h__

#include "Puma/CVisitor.h"
#include "Puma/CTranslationUnit.h"
#include "Puma/Stack.h"

namespace Puma {
  class CAttributeInfo;
  class CFunctionInfo;
} // namespace Puma

class PumaModelBuilder;

class PumaTrackerDog : public Puma::CVisitor {
  Puma::CTranslationUnit &_tunit;
  PumaModelBuilder &_jpm;
  Puma::CT_BinaryExpr *assignment;
  Puma::CT_InitDeclarator *last_init_declarator;
  Puma::Stack<Puma::CFunctionInfo*> func_stack;
  bool in_memb_init;
  int local_id;
  bool _track_attrs;

  Puma::CSemDatabase &db () { return _tunit.db (); }

  virtual void pre_visit (Puma::CTree *node);
  virtual void post_visit (Puma::CTree *node);

  void pre_Call (Puma::CT_Call *node);
  void pre_FctDef (Puma::CT_FctDef *node);
  void post_FctDef (Puma::CT_FctDef *node);
  void pre_InitDeclarator (Puma::CT_InitDeclarator *node);
  void post_InitDeclarator (Puma::CT_InitDeclarator *node);
  void pre_MembPtrExpr (Puma::CT_MembPtrExpr *node);
  void pre_SimpleName (Puma::CT_SimpleName *node);
  void pre_NamespaceDef (Puma::CT_NamespaceDef *node);
  void pre_ClassDef (Puma::CT_ClassDef *node);
  void post_ClassDef (Puma::CT_ClassDef *node);
  bool register_access (Puma::CObjectInfo *obj, Puma::CTree *node);
  
public:
  
  PumaTrackerDog (Puma::CTranslationUnit &tunit, PumaModelBuilder &jpm, bool track_attrs = false):
    _tunit (tunit), _jpm (jpm), assignment (0), last_init_declarator (0),
    in_memb_init (false), _track_attrs (track_attrs) {
    func_stack.push ((Puma::CFunctionInfo*)0);
  }
  virtual ~PumaTrackerDog() {}

  // search all code join points
  void run () { visit (_tunit.tree ()); }
  void run (Puma::CTree *tree) { visit (tree); }
    
  // check if a function should be considered by ac++
  static Puma::CFunctionInfo *filter_func (Puma::CFunctionInfo* fi);
  static Puma::CObjectInfo *unique_object (Puma::CObjectInfo *obj);
};

#endif // __PumaTrackerDog_h__
