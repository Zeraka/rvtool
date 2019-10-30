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

#ifndef __CSemVisitor_h__
#define __CSemVisitor_h__

#include "Puma/ErrorSink.h"
#include "Puma/CVisitor.h"
#include "Puma/Limits.h"
#include "Puma/Config.h"
#include "Puma/CTree.h"

namespace Puma {


class CStructure;
class CObjectInfo;
class CTypeInfo;

class CSemVisitor : private CVisitor {
protected:
  CStructure *current_scope;
  ErrorSink &err;

public:
  CSemVisitor (ErrorSink &, CStructure * = (CStructure*)0);

  void run (CTree *, CStructure * = (CStructure*)0);
  void configure (Config &) {}

  CTypeInfo *resolveExpr (CTree *, CTree *) const;
  void resolveInit (CObjectInfo *info, CTree *, CTree *) const;
  
protected:
  void pre_visit (CTree *);
  void post_visit (CTree *);

protected:
  void pre_action (CT_Program *);
  void pre_action (CT_ArgDeclList *);
  void pre_action (CT_ArgNameList *);
  void pre_action (CT_ArgDeclSeq *);
  void pre_action (CT_MembList *);
  void pre_action (CT_CmpdStmt *);
  void pre_action (CT_IfStmt *);
  void pre_action (CT_IfElseStmt *);
  void pre_action (CT_SwitchStmt *);
  void pre_action (CT_ForStmt *);
  void pre_action (CT_WhileStmt *);
  void pre_action (CT_DoStmt *);
  void pre_action (CT_ExprStmt *);
  void pre_action (CT_CaseStmt *);
  void pre_action (CT_GotoStmt *);
  void pre_action (CT_ReturnStmt *);
  void pre_action (CT_InitDeclarator *);
  void pre_action (CT_ArrayDeclarator *);
  void pre_action (CT_BitFieldDeclarator *);
  void pre_action (CT_ExprList *);
  void pre_action (CT_CmpdLiteral *);
  
protected:
  void post_action (CT_Program *);
  void post_action (CT_ArgDeclList *);
  void post_action (CT_ArgNameList *);
  void post_action (CT_ArgDeclSeq *);
  void post_action (CT_MembList *);
  void post_action (CT_CmpdStmt *);
  void post_action (CT_IfStmt *);
  void post_action (CT_IfElseStmt *);
  void post_action (CT_SwitchStmt *);
  void post_action (CT_ForStmt *);
  void post_action (CT_WhileStmt *);

  CT_SimpleName *findName (CTree *) const;

private:
  void check_goto_label (CTree *);
  
};

} // namespace Puma

#endif /* __CSemVisitor_h__ */
