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

#ifndef __XmlModelReader_h__
#define __XmlModelReader_h__

#include <map>
#include "Elements.h"

class XmlModelReader {

  ProjectModel *model_;
  xmlNodePtr curr_;
  ModelNode **node_map_;
  int ids_;
  typedef std::map<string, xmlNodePtr> ChildMap;
  ChildMap *child_map_;
  
  // initialize a node and its children from an XML node (tree)
  ModelNode *from_xml (ACM_Node &node);
  ModelNode *from_xml (ACM_File &node);
  ModelNode *from_xml (ACM_TUnit &node);
  ModelNode *from_xml (ACM_Header &node);
  ModelNode *from_xml (ACM_Source &node);
  ModelNode *from_xml (ACM_MemberIntro &node);
  ModelNode *from_xml (ACM_BaseIntro &node);
  ModelNode *from_xml (ACM_ClassPlan &node);
  ModelNode *from_xml (ACM_CodeAdvice &node);
  ModelNode *from_xml (ACM_CodePlan &node);
  ModelNode *from_xml (ACM_Any &node);
  ModelNode *from_xml (ACM_Type &node);
  ModelNode *from_xml (ACM_Arg &node);
  ModelNode *from_xml (ACM_Name &node);
  ModelNode *from_xml (ACM_Namespace &node);
  ModelNode *from_xml (ACM_Function &node);
  ModelNode *from_xml (ACM_Variable &node);
  ModelNode *from_xml (ACM_Class &node);
  ModelNode *from_xml (ACM_ClassSlice &node);
  ModelNode *from_xml (ACM_Code &node);
  ModelNode *from_xml (ACM_Access &node);
  ModelNode *from_xml (ACM_Get &node);
  ModelNode *from_xml (ACM_Set &node);
  ModelNode *from_xml (ACM_Call &node);
  ModelNode *from_xml (ACM_Builtin &node);
  ModelNode *from_xml (ACM_Ref &node);
  ModelNode *from_xml (ACM_GetRef &node);
  ModelNode *from_xml (ACM_SetRef &node);
  ModelNode *from_xml (ACM_CallRef &node);
  ModelNode *from_xml (ACM_Execution &node);
  ModelNode *from_xml (ACM_Construction &node);
  ModelNode *from_xml (ACM_Destruction &node);
  ModelNode *from_xml (ACM_Pointcut &node);
  ModelNode *from_xml (ACM_Attribute &node);
  ModelNode *from_xml (ACM_Advice &node);
  ModelNode *from_xml (ACM_AdviceCode &node);
  ModelNode *from_xml (ACM_Introduction &node);
  ModelNode *from_xml (ACM_Order &node);
  ModelNode *from_xml (ACM_Aspect &node);
  ModelNode *from_xml (ACM_Speculation &node);
  ModelNode *from_xml (ACM_Model &node);

  // helper functions
  bool alloc_nodes_with_id (xmlNodePtr node); 
  ModelNode *alloc_node (xmlNodePtr node);
  ModelNode *initialize_node (xmlNodePtr node);
  ChildMap *set_child_map (ChildMap &cm, xmlNodePtr node);
  void unset_child_map (ChildMap *cm);
  xmlNodePtr find_child (string name);
  
public:
  // clear model and load the contents of an XML project file into it
  bool read (ProjectModel &model, const char *filename, int fd = -1);
};

#endif // __XmlModelReader_h__
