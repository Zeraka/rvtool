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

#include <iostream>
#include <sstream>
using std::istringstream;
#include <string.h>

#include "XmlModelReader.h"


ModelNode *XmlModelReader::from_xml (ACM_Node &node) {
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_File &node) {
  char *str_filename = (char*)xmlGetProp (curr_, (xmlChar*)"filename");
  if (str_filename) { 
    node.set_filename (str_filename);
    xmlFree (str_filename);
  }
  char *str_len = (char*)xmlGetProp (curr_, (xmlChar*)"len");
  if (str_len) { 
    int int_len = atoi (str_len);
    node.set_len ((int)int_len);
    xmlFree (str_len);
  }
  char *str_time = (char*)xmlGetProp (curr_, (xmlChar*)"time");
  if (str_time) { 
    int int_time = atoi (str_time);
    node.set_time ((int)int_time);
    xmlFree (str_time);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_TUnit &node) {
  node.set_dirty_flag (false);
  char *str_dirty_flag = (char*)xmlGetProp (curr_, (xmlChar*)"dirty_flag");
  if (str_dirty_flag) { 
    node.set_dirty_flag (strcmp (str_dirty_flag, "true") == 0);
    xmlFree (str_dirty_flag);
  }
  from_xml ((ACM_File&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Header &node) {
  char *str_in = (char*)xmlGetProp (curr_, (xmlChar*)"in");
  if (str_in) { 
    istringstream in (str_in, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_in ().insert ((ACM_TUnit*)node_map_[id]);
    }
    xmlFree (str_in);
  }
  from_xml ((ACM_File&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Source &node) {
  char *str_kind = (char*)xmlGetProp (curr_, (xmlChar*)"kind");
  if (str_kind) { 
    int int_kind = atoi (str_kind);
    node.set_kind ((SourceLocKind)int_kind);
    xmlFree (str_kind);
  }
  char *str_file = (char*)xmlGetProp (curr_, (xmlChar*)"file");
  if (str_file) { 
    int file_id = atoi (str_file);
    node.set_file ((ACM_File*)node_map_[file_id]);
    xmlFree (str_file);
  }
  char *str_line = (char*)xmlGetProp (curr_, (xmlChar*)"line");
  if (str_line) { 
    int int_line = atoi (str_line);
    node.set_line ((int)int_line);
    xmlFree (str_line);
  }
  char *str_len = (char*)xmlGetProp (curr_, (xmlChar*)"len");
  if (str_len) { 
    int int_len = atoi (str_len);
    node.set_len ((int)int_len);
    xmlFree (str_len);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_MemberIntro &node) {
  char *str_intro = (char*)xmlGetProp (curr_, (xmlChar*)"intro");
  if (str_intro) { 
    int intro_id = atoi (str_intro);
    node.set_intro ((ACM_Introduction*)node_map_[intro_id]);
    xmlFree (str_intro);
  }
  char *str_members = (char*)xmlGetProp (curr_, (xmlChar*)"members");
  if (str_members) { 
    istringstream in (str_members, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_members ().insert ((ACM_Name*)node_map_[id]);
    }
    xmlFree (str_members);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_BaseIntro &node) {
  char *str_intro = (char*)xmlGetProp (curr_, (xmlChar*)"intro");
  if (str_intro) { 
    int intro_id = atoi (str_intro);
    node.set_intro ((ACM_Introduction*)node_map_[intro_id]);
    xmlFree (str_intro);
  }
  char *str_bases = (char*)xmlGetProp (curr_, (xmlChar*)"bases");
  if (str_bases) { 
    istringstream in (str_bases, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_bases ().insert ((ACM_Class*)node_map_[id]);
    }
    xmlFree (str_bases);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_ClassPlan &node) {
  xmlNodePtr child_base_intros = find_child ("base_intros");
  if (child_base_intros) {
    node.get_base_intros ().set_xml_node (child_base_intros);
    for (xmlNodePtr curr = child_base_intros->children; curr; curr = curr->next)
      node.get_base_intros ().insert ((ACM_BaseIntro*)initialize_node (curr));
  }
  xmlNodePtr child_member_intros = find_child ("member_intros");
  if (child_member_intros) {
    node.get_member_intros ().set_xml_node (child_member_intros);
    for (xmlNodePtr curr = child_member_intros->children; curr; curr = curr->next)
      node.get_member_intros ().insert ((ACM_MemberIntro*)initialize_node (curr));
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_CodeAdvice &node) {
  char *str_advice = (char*)xmlGetProp (curr_, (xmlChar*)"advice");
  if (str_advice) { 
    int advice_id = atoi (str_advice);
    node.set_advice ((ACM_AdviceCode*)node_map_[advice_id]);
    xmlFree (str_advice);
  }
  node.set_conditional (false);
  char *str_conditional = (char*)xmlGetProp (curr_, (xmlChar*)"conditional");
  if (str_conditional) { 
    node.set_conditional (strcmp (str_conditional, "true") == 0);
    xmlFree (str_conditional);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_CodePlan &node) {
  xmlNodePtr child_before = find_child ("before");
  if (child_before) {
    node.get_before ().set_xml_node (child_before);
    for (xmlNodePtr curr = child_before->children; curr; curr = curr->next)
      node.get_before ().insert ((ACM_CodeAdvice*)initialize_node (curr));
  }
  xmlNodePtr child_around = find_child ("around");
  if (child_around)
    node.set_around ((ACM_CodeAdvice*)initialize_node (child_around->children));
  xmlNodePtr child_after = find_child ("after");
  if (child_after) {
    node.get_after ().set_xml_node (child_after);
    for (xmlNodePtr curr = child_after->children; curr; curr = curr->next)
      node.get_after ().insert ((ACM_CodeAdvice*)initialize_node (curr));
  }
  xmlNodePtr child_next_level = find_child ("next_level");
  if (child_next_level)
    node.set_next_level ((ACM_CodePlan*)initialize_node (child_next_level->children));
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Any &node) {
  char *str_jpid = (char*)xmlGetProp (curr_, (xmlChar*)"jpid");
  if (str_jpid) { 
    int int_jpid = atoi (str_jpid);
    node.set_jpid ((int)int_jpid);
    xmlFree (str_jpid);
  }
  xmlNodePtr child_source = find_child ("source");
  if (child_source) {
    node.get_source ().set_xml_node (child_source);
    for (xmlNodePtr curr = child_source->children; curr; curr = curr->next)
      node.get_source ().insert ((ACM_Source*)initialize_node (curr));
  }
  char *str_attributes = (char*)xmlGetProp (curr_, (xmlChar*)"attributes");
  if (str_attributes) { 
    istringstream in (str_attributes, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_attributes ().insert ((ACM_Attribute*)node_map_[id]);
    }
    xmlFree (str_attributes);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Type &node) {
  char *str_signature = (char*)xmlGetProp (curr_, (xmlChar*)"signature");
  if (str_signature) { 
    node.set_signature (str_signature);
    xmlFree (str_signature);
  }
  from_xml ((ACM_Any&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Arg &node) {
  char *str_type = (char*)xmlGetProp (curr_, (xmlChar*)"type");
  if (str_type) { 
    node.set_type (str_type);
    xmlFree (str_type);
  }
  char *str_name = (char*)xmlGetProp (curr_, (xmlChar*)"name");
  if (str_name) { 
    node.set_name (str_name);
    xmlFree (str_name);
  }
  from_xml ((ACM_Any&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Name &node) {
  xmlNodePtr child_children = find_child ("children");
  if (child_children) {
    node.get_children ().set_xml_node (child_children);
    for (xmlNodePtr curr = child_children->children; curr; curr = curr->next)
      node.get_children ().insert ((ACM_Any*)initialize_node (curr));
  }
  char *str_name = (char*)xmlGetProp (curr_, (xmlChar*)"name");
  if (str_name) { 
    node.set_name (str_name);
    xmlFree (str_name);
  }
  node.set_builtin (false);
  char *str_builtin = (char*)xmlGetProp (curr_, (xmlChar*)"builtin");
  if (str_builtin) { 
    node.set_builtin (strcmp (str_builtin, "true") == 0);
    xmlFree (str_builtin);
  }
  char *str_tunits = (char*)xmlGetProp (curr_, (xmlChar*)"tunits");
  if (str_tunits) { 
    istringstream in (str_tunits, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_tunits ().insert ((ACM_TUnit*)node_map_[id]);
    }
    xmlFree (str_tunits);
  }
  from_xml ((ACM_Any&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Namespace &node) {
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Function &node) {
  xmlNodePtr child_result_type = find_child ("result_type");
  if (child_result_type)
    node.set_result_type ((ACM_Type*)initialize_node (child_result_type->children));
  xmlNodePtr child_arg_types = find_child ("arg_types");
  if (child_arg_types) {
    node.get_arg_types ().set_xml_node (child_arg_types);
    for (xmlNodePtr curr = child_arg_types->children; curr; curr = curr->next)
      node.get_arg_types ().insert ((ACM_Type*)initialize_node (curr));
  }
  char *str_kind = (char*)xmlGetProp (curr_, (xmlChar*)"kind");
  if (str_kind) { 
    int int_kind = atoi (str_kind);
    node.set_kind ((FunctionType)int_kind);
    xmlFree (str_kind);
  }
  node.set_variadic_args (false);
  char *str_variadic_args = (char*)xmlGetProp (curr_, (xmlChar*)"variadic_args");
  if (str_variadic_args) { 
    node.set_variadic_args (strcmp (str_variadic_args, "true") == 0);
    xmlFree (str_variadic_args);
  }
  char *str_cv_qualifiers = (char*)xmlGetProp (curr_, (xmlChar*)"cv_qualifiers");
  if (str_cv_qualifiers) { 
    int int_cv_qualifiers = atoi (str_cv_qualifiers);
    node.set_cv_qualifiers ((CVQualifiers)int_cv_qualifiers);
    xmlFree (str_cv_qualifiers);
  }
  char *str_static_in = (char*)xmlGetProp (curr_, (xmlChar*)"static_in");
  if (str_static_in) { 
    int static_in_id = atoi (str_static_in);
    node.set_static_in ((ACM_TUnit*)node_map_[static_in_id]);
    xmlFree (str_static_in);
  }
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Variable &node) {
  xmlNodePtr child_type = find_child ("type");
  if (child_type)
    node.set_type ((ACM_Type*)initialize_node (child_type->children));
  char *str_kind = (char*)xmlGetProp (curr_, (xmlChar*)"kind");
  if (str_kind) { 
    int int_kind = atoi (str_kind);
    node.set_kind ((VariableType)int_kind);
    xmlFree (str_kind);
  }
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Class &node) {
  char *str_bases = (char*)xmlGetProp (curr_, (xmlChar*)"bases");
  if (str_bases) { 
    istringstream in (str_bases, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_bases ().insert ((ACM_Class*)node_map_[id]);
    }
    xmlFree (str_bases);
  }
  xmlNodePtr child_plan = find_child ("plan");
  if (child_plan)
    node.set_plan ((ACM_ClassPlan*)initialize_node (child_plan->children));
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_ClassSlice &node) {
  node.set_is_struct (false);
  char *str_is_struct = (char*)xmlGetProp (curr_, (xmlChar*)"is_struct");
  if (str_is_struct) { 
    node.set_is_struct (strcmp (str_is_struct, "true") == 0);
    xmlFree (str_is_struct);
  }
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Code &node) {
  xmlNodePtr child_plan = find_child ("plan");
  if (child_plan)
    node.set_plan ((ACM_CodePlan*)initialize_node (child_plan->children));
  from_xml ((ACM_Any&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Access &node) {
  char *str_lid = (char*)xmlGetProp (curr_, (xmlChar*)"lid");
  if (str_lid) { 
    int int_lid = atoi (str_lid);
    node.set_lid ((int)int_lid);
    xmlFree (str_lid);
  }
  char *str_target_object_lid = (char*)xmlGetProp (curr_, (xmlChar*)"target_object_lid");
  if (str_target_object_lid) { 
    int int_target_object_lid = atoi (str_target_object_lid);
    node.set_target_object_lid ((int)int_target_object_lid);
    xmlFree (str_target_object_lid);
  }
  char *str_cfg_block_lid = (char*)xmlGetProp (curr_, (xmlChar*)"cfg_block_lid");
  if (str_cfg_block_lid) { 
    int int_cfg_block_lid = atoi (str_cfg_block_lid);
    node.set_cfg_block_lid ((int)int_cfg_block_lid);
    xmlFree (str_cfg_block_lid);
  }
  char *str_target_class = (char*)xmlGetProp (curr_, (xmlChar*)"target_class");
  if (str_target_class) { 
    int target_class_id = atoi (str_target_class);
    node.set_target_class ((ACM_Class*)node_map_[target_class_id]);
    xmlFree (str_target_class);
  }
  from_xml ((ACM_Code&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Get &node) {
  char *str_variable = (char*)xmlGetProp (curr_, (xmlChar*)"variable");
  if (str_variable) { 
    int variable_id = atoi (str_variable);
    node.set_variable ((ACM_Variable*)node_map_[variable_id]);
    xmlFree (str_variable);
  }
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Set &node) {
  char *str_variable = (char*)xmlGetProp (curr_, (xmlChar*)"variable");
  if (str_variable) { 
    int variable_id = atoi (str_variable);
    node.set_variable ((ACM_Variable*)node_map_[variable_id]);
    xmlFree (str_variable);
  }
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Call &node) {
  char *str_target = (char*)xmlGetProp (curr_, (xmlChar*)"target");
  if (str_target) { 
    int target_id = atoi (str_target);
    node.set_target ((ACM_Function*)node_map_[target_id]);
    xmlFree (str_target);
  }
  xmlNodePtr child_variadic_arg_types = find_child ("variadic_arg_types");
  if (child_variadic_arg_types) {
    node.get_variadic_arg_types ().set_xml_node (child_variadic_arg_types);
    for (xmlNodePtr curr = child_variadic_arg_types->children; curr; curr = curr->next)
      node.get_variadic_arg_types ().insert ((ACM_Type*)initialize_node (curr));
  }
  char *str_default_args = (char*)xmlGetProp (curr_, (xmlChar*)"default_args");
  if (str_default_args) { 
    int int_default_args = atoi (str_default_args);
    node.set_default_args ((int)int_default_args);
    xmlFree (str_default_args);
  }
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Builtin &node) {
  char *str_target = (char*)xmlGetProp (curr_, (xmlChar*)"target");
  if (str_target) { 
    int target_id = atoi (str_target);
    node.set_target ((ACM_Function*)node_map_[target_id]);
    xmlFree (str_target);
  }
  xmlNodePtr child_implicit_access = find_child ("implicit_access");
  if (child_implicit_access) {
    node.get_implicit_access ().set_xml_node (child_implicit_access);
    for (xmlNodePtr curr = child_implicit_access->children; curr; curr = curr->next)
      node.get_implicit_access ().insert ((ACM_Access*)initialize_node (curr));
  }
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Ref &node) {
  char *str_variable = (char*)xmlGetProp (curr_, (xmlChar*)"variable");
  if (str_variable) { 
    int variable_id = atoi (str_variable);
    node.set_variable ((ACM_Variable*)node_map_[variable_id]);
    xmlFree (str_variable);
  }
  xmlNodePtr child_type = find_child ("type");
  if (child_type)
    node.set_type ((ACM_Type*)initialize_node (child_type->children));
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_GetRef &node) {
  xmlNodePtr child_type = find_child ("type");
  if (child_type)
    node.set_type ((ACM_Type*)initialize_node (child_type->children));
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_SetRef &node) {
  xmlNodePtr child_type = find_child ("type");
  if (child_type)
    node.set_type ((ACM_Type*)initialize_node (child_type->children));
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_CallRef &node) {
  xmlNodePtr child_type = find_child ("type");
  if (child_type)
    node.set_type ((ACM_Type*)initialize_node (child_type->children));
  from_xml ((ACM_Access&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Execution &node) {
  from_xml ((ACM_Code&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Construction &node) {
  from_xml ((ACM_Code&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Destruction &node) {
  from_xml ((ACM_Code&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Pointcut &node) {
  char *str_expr = (char*)xmlGetProp (curr_, (xmlChar*)"expr");
  if (str_expr) { 
    node.set_expr (str_expr);
    xmlFree (str_expr);
  }
  xmlNodePtr child_args = find_child ("args");
  if (child_args) {
    node.get_args ().set_xml_node (child_args);
    for (xmlNodePtr curr = child_args->children; curr; curr = curr->next)
      node.get_args ().insert ((ACM_Arg*)initialize_node (curr));
  }
  char *str_kind = (char*)xmlGetProp (curr_, (xmlChar*)"kind");
  if (str_kind) { 
    int int_kind = atoi (str_kind);
    node.set_kind ((PointcutType)int_kind);
    xmlFree (str_kind);
  }
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Attribute &node) {
  xmlNodePtr child_args = find_child ("args");
  if (child_args) {
    node.get_args ().set_xml_node (child_args);
    for (xmlNodePtr curr = child_args->children; curr; curr = curr->next)
      node.get_args ().insert ((ACM_Arg*)initialize_node (curr));
  }
  from_xml ((ACM_Name&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Advice &node) {
  xmlNodePtr child_pointcut = find_child ("pointcut");
  if (child_pointcut)
    node.set_pointcut ((ACM_Pointcut*)initialize_node (child_pointcut->children));
  char *str_lid = (char*)xmlGetProp (curr_, (xmlChar*)"lid");
  if (str_lid) { 
    int int_lid = atoi (str_lid);
    node.set_lid ((int)int_lid);
    xmlFree (str_lid);
  }
  from_xml ((ACM_Any&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_AdviceCode &node) {
  char *str_kind = (char*)xmlGetProp (curr_, (xmlChar*)"kind");
  if (str_kind) { 
    int int_kind = atoi (str_kind);
    node.set_kind ((AdviceCodeType)int_kind);
    xmlFree (str_kind);
  }
  char *str_context = (char*)xmlGetProp (curr_, (xmlChar*)"context");
  if (str_context) { 
    int int_context = atoi (str_context);
    node.set_context ((AdviceCodeContext)int_context);
    xmlFree (str_context);
  }
  from_xml ((ACM_Advice&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Introduction &node) {
  char *str_named_slice = (char*)xmlGetProp (curr_, (xmlChar*)"named_slice");
  if (str_named_slice) { 
    int named_slice_id = atoi (str_named_slice);
    node.set_named_slice ((ACM_ClassSlice*)node_map_[named_slice_id]);
    xmlFree (str_named_slice);
  }
  xmlNodePtr child_anon_slice = find_child ("anon_slice");
  if (child_anon_slice)
    node.set_anon_slice ((ACM_ClassSlice*)initialize_node (child_anon_slice->children));
  from_xml ((ACM_Advice&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Order &node) {
  xmlNodePtr child_aspect_exprs = find_child ("aspect_exprs");
  if (child_aspect_exprs) {
    node.get_aspect_exprs ().set_xml_node (child_aspect_exprs);
    for (xmlNodePtr curr = child_aspect_exprs->children; curr; curr = curr->next)
      node.get_aspect_exprs ().insert ((ACM_Pointcut*)initialize_node (curr));
  }
  from_xml ((ACM_Advice&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Aspect &node) {
  xmlNodePtr child_intros = find_child ("intros");
  if (child_intros) {
    node.get_intros ().set_xml_node (child_intros);
    for (xmlNodePtr curr = child_intros->children; curr; curr = curr->next)
      node.get_intros ().insert ((ACM_Introduction*)initialize_node (curr));
  }
  xmlNodePtr child_advices = find_child ("advices");
  if (child_advices) {
    node.get_advices ().set_xml_node (child_advices);
    for (xmlNodePtr curr = child_advices->children; curr; curr = curr->next)
      node.get_advices ().insert ((ACM_AdviceCode*)initialize_node (curr));
  }
  xmlNodePtr child_orders = find_child ("orders");
  if (child_orders) {
    node.get_orders ().set_xml_node (child_orders);
    for (xmlNodePtr curr = child_orders->children; curr; curr = curr->next)
      node.get_orders ().insert ((ACM_Order*)initialize_node (curr));
  }
  from_xml ((ACM_Class&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Speculation &node) {
  char *str_advice = (char*)xmlGetProp (curr_, (xmlChar*)"advice");
  if (str_advice) { 
    int advice_id = atoi (str_advice);
    node.set_advice ((ACM_Advice*)node_map_[advice_id]);
    xmlFree (str_advice);
  }
  char *str_tunit = (char*)xmlGetProp (curr_, (xmlChar*)"tunit");
  if (str_tunit) { 
    int tunit_id = atoi (str_tunit);
    node.set_tunit ((ACM_TUnit*)node_map_[tunit_id]);
    xmlFree (str_tunit);
  }
  char *str_true_jps = (char*)xmlGetProp (curr_, (xmlChar*)"true_jps");
  if (str_true_jps) { 
    istringstream in (str_true_jps, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_true_jps ().insert ((ACM_Any*)node_map_[id]);
    }
    xmlFree (str_true_jps);
  }
  char *str_false_jps = (char*)xmlGetProp (curr_, (xmlChar*)"false_jps");
  if (str_false_jps) { 
    istringstream in (str_false_jps, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_false_jps ().insert ((ACM_Any*)node_map_[id]);
    }
    xmlFree (str_false_jps);
  }
  char *str_conditional_jps = (char*)xmlGetProp (curr_, (xmlChar*)"conditional_jps");
  if (str_conditional_jps) { 
    istringstream in (str_conditional_jps, istringstream::in);
    int id;
    while (!in.eof ()) {
      in >> id;
      if (in.fail ())
        break;
      node.get_conditional_jps ().insert ((ACM_Any*)node_map_[id]);
    }
    xmlFree (str_conditional_jps);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}

ModelNode *XmlModelReader::from_xml (ACM_Model &node) {
  xmlNodePtr child_files = find_child ("files");
  if (child_files) {
    node.get_files ().set_xml_node (child_files);
    for (xmlNodePtr curr = child_files->children; curr; curr = curr->next)
      node.get_files ().insert ((ACM_File*)initialize_node (curr));
  }
  xmlNodePtr child_root = find_child ("root");
  if (child_root)
    node.set_root ((ACM_Namespace*)initialize_node (child_root->children));
  xmlNodePtr child_speculations = find_child ("speculations");
  if (child_speculations) {
    node.get_speculations ().set_xml_node (child_speculations);
    for (xmlNodePtr curr = child_speculations->children; curr; curr = curr->next)
      node.get_speculations ().insert ((ACM_Speculation*)initialize_node (curr));
  }
  char *str_version = (char*)xmlGetProp (curr_, (xmlChar*)"version");
  if (str_version) { 
    node.set_version (str_version);
    xmlFree (str_version);
  }
  from_xml ((ACM_Node&)node);
  return &node;
}


bool XmlModelReader::alloc_nodes_with_id (xmlNodePtr node) {
  char *str_id = (char*)xmlGetProp (node, (xmlChar*)"id");
  if (str_id) {
    int id = atoi (str_id);
    free (str_id);
    if (id < 0 || id >= ids_ || node_map_[id] != 0) {
      std::cout << "invalid or duplicate id " << id << " in project file." << std::endl;
      return false;
    }
    node_map_[id] = alloc_node (node);
  }
  for (xmlNodePtr curr = node->children; curr; curr = curr->next)
    if (!alloc_nodes_with_id (curr))
      return false;
  return true;
}
    
ModelNode *XmlModelReader::alloc_node (xmlNodePtr node) {
  const char *name = (const char*)node->name;
  if (strcmp (name, "TUnit") == 0)
    return model_->newTUnit ();
else  if (strcmp (name, "Header") == 0)
    return model_->newHeader ();
else  if (strcmp (name, "Source") == 0)
    return model_->newSource ();
else  if (strcmp (name, "MemberIntro") == 0)
    return model_->newMemberIntro ();
else  if (strcmp (name, "BaseIntro") == 0)
    return model_->newBaseIntro ();
else  if (strcmp (name, "ClassPlan") == 0)
    return model_->newClassPlan ();
else  if (strcmp (name, "CodeAdvice") == 0)
    return model_->newCodeAdvice ();
else  if (strcmp (name, "CodePlan") == 0)
    return model_->newCodePlan ();
else  if (strcmp (name, "Type") == 0)
    return model_->newType ();
else  if (strcmp (name, "Arg") == 0)
    return model_->newArg ();
else  if (strcmp (name, "Namespace") == 0)
    return model_->newNamespace ();
else  if (strcmp (name, "Function") == 0)
    return model_->newFunction ();
else  if (strcmp (name, "Variable") == 0)
    return model_->newVariable ();
else  if (strcmp (name, "Class") == 0)
    return model_->newClass ();
else  if (strcmp (name, "ClassSlice") == 0)
    return model_->newClassSlice ();
else  if (strcmp (name, "Get") == 0)
    return model_->newGet ();
else  if (strcmp (name, "Set") == 0)
    return model_->newSet ();
else  if (strcmp (name, "Call") == 0)
    return model_->newCall ();
else  if (strcmp (name, "Builtin") == 0)
    return model_->newBuiltin ();
else  if (strcmp (name, "Ref") == 0)
    return model_->newRef ();
else  if (strcmp (name, "GetRef") == 0)
    return model_->newGetRef ();
else  if (strcmp (name, "SetRef") == 0)
    return model_->newSetRef ();
else  if (strcmp (name, "CallRef") == 0)
    return model_->newCallRef ();
else  if (strcmp (name, "Execution") == 0)
    return model_->newExecution ();
else  if (strcmp (name, "Construction") == 0)
    return model_->newConstruction ();
else  if (strcmp (name, "Destruction") == 0)
    return model_->newDestruction ();
else  if (strcmp (name, "Pointcut") == 0)
    return model_->newPointcut ();
else  if (strcmp (name, "Attribute") == 0)
    return model_->newAttribute ();
else  if (strcmp (name, "AdviceCode") == 0)
    return model_->newAdviceCode ();
else  if (strcmp (name, "Introduction") == 0)
    return model_->newIntroduction ();
else  if (strcmp (name, "Order") == 0)
    return model_->newOrder ();
else  if (strcmp (name, "Aspect") == 0)
    return model_->newAspect ();
else  if (strcmp (name, "Speculation") == 0)
    return model_->newSpeculation ();
else  if (strcmp (name, "Model") == 0)
    return model_->newModel ();

  return 0;
}
  
ModelNode *XmlModelReader::initialize_node (xmlNodePtr node) {
  ModelNode *result = 0;
  char *str_id = (char*)xmlGetProp (node, (xmlChar*)"id");
  if (str_id) {
    result = node_map_[atoi (str_id)];
    free (str_id);
  }
  const char *name = (const char*)node->name;
  xmlNodePtr old_curr = curr_;
  curr_ = node;
  ChildMap child_map;
  ChildMap *old_child_map = set_child_map (child_map, node);

  if (strcmp (name, "TUnit") == 0)
    result = from_xml (result ? *(ACM_TUnit*)result : *model_->newTUnit ());
  else  if (strcmp (name, "Header") == 0)
    result = from_xml (result ? *(ACM_Header*)result : *model_->newHeader ());
  else  if (strcmp (name, "Source") == 0)
    result = from_xml (result ? *(ACM_Source*)result : *model_->newSource ());
  else  if (strcmp (name, "MemberIntro") == 0)
    result = from_xml (result ? *(ACM_MemberIntro*)result : *model_->newMemberIntro ());
  else  if (strcmp (name, "BaseIntro") == 0)
    result = from_xml (result ? *(ACM_BaseIntro*)result : *model_->newBaseIntro ());
  else  if (strcmp (name, "ClassPlan") == 0)
    result = from_xml (result ? *(ACM_ClassPlan*)result : *model_->newClassPlan ());
  else  if (strcmp (name, "CodeAdvice") == 0)
    result = from_xml (result ? *(ACM_CodeAdvice*)result : *model_->newCodeAdvice ());
  else  if (strcmp (name, "CodePlan") == 0)
    result = from_xml (result ? *(ACM_CodePlan*)result : *model_->newCodePlan ());
  else  if (strcmp (name, "Type") == 0)
    result = from_xml (result ? *(ACM_Type*)result : *model_->newType ());
  else  if (strcmp (name, "Arg") == 0)
    result = from_xml (result ? *(ACM_Arg*)result : *model_->newArg ());
  else  if (strcmp (name, "Namespace") == 0)
    result = from_xml (result ? *(ACM_Namespace*)result : *model_->newNamespace ());
  else  if (strcmp (name, "Function") == 0)
    result = from_xml (result ? *(ACM_Function*)result : *model_->newFunction ());
  else  if (strcmp (name, "Variable") == 0)
    result = from_xml (result ? *(ACM_Variable*)result : *model_->newVariable ());
  else  if (strcmp (name, "Class") == 0)
    result = from_xml (result ? *(ACM_Class*)result : *model_->newClass ());
  else  if (strcmp (name, "ClassSlice") == 0)
    result = from_xml (result ? *(ACM_ClassSlice*)result : *model_->newClassSlice ());
  else  if (strcmp (name, "Get") == 0)
    result = from_xml (result ? *(ACM_Get*)result : *model_->newGet ());
  else  if (strcmp (name, "Set") == 0)
    result = from_xml (result ? *(ACM_Set*)result : *model_->newSet ());
  else  if (strcmp (name, "Call") == 0)
    result = from_xml (result ? *(ACM_Call*)result : *model_->newCall ());
  else  if (strcmp (name, "Builtin") == 0)
    result = from_xml (result ? *(ACM_Builtin*)result : *model_->newBuiltin ());
  else  if (strcmp (name, "Ref") == 0)
    result = from_xml (result ? *(ACM_Ref*)result : *model_->newRef ());
  else  if (strcmp (name, "GetRef") == 0)
    result = from_xml (result ? *(ACM_GetRef*)result : *model_->newGetRef ());
  else  if (strcmp (name, "SetRef") == 0)
    result = from_xml (result ? *(ACM_SetRef*)result : *model_->newSetRef ());
  else  if (strcmp (name, "CallRef") == 0)
    result = from_xml (result ? *(ACM_CallRef*)result : *model_->newCallRef ());
  else  if (strcmp (name, "Execution") == 0)
    result = from_xml (result ? *(ACM_Execution*)result : *model_->newExecution ());
  else  if (strcmp (name, "Construction") == 0)
    result = from_xml (result ? *(ACM_Construction*)result : *model_->newConstruction ());
  else  if (strcmp (name, "Destruction") == 0)
    result = from_xml (result ? *(ACM_Destruction*)result : *model_->newDestruction ());
  else  if (strcmp (name, "Pointcut") == 0)
    result = from_xml (result ? *(ACM_Pointcut*)result : *model_->newPointcut ());
  else  if (strcmp (name, "Attribute") == 0)
    result = from_xml (result ? *(ACM_Attribute*)result : *model_->newAttribute ());
  else  if (strcmp (name, "AdviceCode") == 0)
    result = from_xml (result ? *(ACM_AdviceCode*)result : *model_->newAdviceCode ());
  else  if (strcmp (name, "Introduction") == 0)
    result = from_xml (result ? *(ACM_Introduction*)result : *model_->newIntroduction ());
  else  if (strcmp (name, "Order") == 0)
    result = from_xml (result ? *(ACM_Order*)result : *model_->newOrder ());
  else  if (strcmp (name, "Aspect") == 0)
    result = from_xml (result ? *(ACM_Aspect*)result : *model_->newAspect ());
  else  if (strcmp (name, "Speculation") == 0)
    result = from_xml (result ? *(ACM_Speculation*)result : *model_->newSpeculation ());
  else  if (strcmp (name, "Model") == 0)
    result = from_xml (result ? *(ACM_Model*)result : *model_->newModel ());
  
  if (result) {
    result->set_xml_node (node);
    result->set_xml_dirty (false);
  }
  curr_ = old_curr;
  unset_child_map (old_child_map);
  return result;
}

XmlModelReader::ChildMap *XmlModelReader::set_child_map (ChildMap &cm, xmlNodePtr node) {
  ChildMap *old = child_map_;
  child_map_ = &cm;
  for (xmlNodePtr c = node->children; c; c = c->next)
    child_map_->insert (ChildMap::value_type ((const char*)c->name, c));
  return old;
}

void XmlModelReader::unset_child_map (ChildMap *cm) {
  child_map_ = cm;
}

xmlNodePtr XmlModelReader::find_child (string name) {
  ChildMap::iterator i = child_map_->find (name);
  return (i == child_map_->end () ? (xmlNodePtr)0 : (*i).second); 
}

// load the contents of an XML project file into the model 
bool XmlModelReader::read (ProjectModel &model, const char *filename, int fd) {
  model_ = &model;
  model.clear ();

  xmlDocPtr doc;
  if (fd >= 0)
    doc = xmlReadFd (fd, filename, 0,
                     XML_PARSE_NODICT | XML_PARSE_NOERROR |
                     XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS);
  else 
    doc = xmlReadFile (filename, 0, XML_PARSE_NODICT |
      XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NOBLANKS);
  if (!doc)
    return false;

  model.xml_doc (doc);
  curr_ = doc->children;
  if (!curr_)
    return false;
  model.set_xml_node(curr_);
  ChildMap child_map;
  ChildMap *old_child_map = set_child_map (child_map, curr_);
  char *str_ids = (char*)xmlGetProp (curr_, (xmlChar*)"ids");
  if (!str_ids) return false;
  ids_ = atoi (str_ids);
  free (str_ids);
  node_map_ = new ModelNode*[ids_];
  memset (node_map_, 0, sizeof (ModelNode*) * ids_);
  bool result = alloc_nodes_with_id (curr_);
  if (result) {
    from_xml (model);
    model.set_xml_dirty (false);
  }
  delete[] node_map_;
  unset_child_map (old_child_map);

  // fix jpid management data in the project model
  ProjectModel::Selection all;
  model.select (JPT_Any, all);
  for (ProjectModel::Selection::iterator iter = all.begin ();
      iter != all.end (); ++iter) {
    ACM_Any &jpl = (ACM_Any&)**iter;
    if (jpl.has_jpid ())
      model.alloc_jpid (jpl.get_jpid ());
  }
  return result;
}

