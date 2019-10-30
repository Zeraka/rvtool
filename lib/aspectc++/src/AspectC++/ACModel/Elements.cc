
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

#include "Elements.h"
#include <iostream>
using std::cout;
using std::endl;
#include <sstream>
using std::ostringstream;

std::string ACM_Node::key () const {
  std::string result;
  result = type_str ();
  result += "|";
  return result;
}
std::string ACM_File::key () const {
  std::string result;
  result += ACM_Node::key ();
    std::ostringstream str_filename_;
    str_filename_ << filename_;
    result += str_filename_.str() + "|";
  return result;
}
std::string ACM_TUnit::key () const {
  std::string result;
  result += ACM_File::key ();
  return result;
}
std::string ACM_Header::key () const {
  std::string result;
  result += ACM_File::key ();
  return result;
}
std::string ACM_Source::key () const {
  std::string result;
  result += ACM_Node::key ();
    std::ostringstream str_kind_;
    str_kind_ << kind_;
    result += str_kind_.str() + "|";
    result += file_->key (); 
    std::ostringstream str_line_;
    str_line_ << line_;
    result += str_line_.str() + "|";
  return result;
}
std::string ACM_MemberIntro::key () const {
  std::string result;
  result += ACM_Node::key ();
    result += intro_->key (); 
  return result;
}
std::string ACM_BaseIntro::key () const {
  std::string result;
  result += ACM_Node::key ();
    result += intro_->key (); 
  return result;
}
std::string ACM_ClassPlan::key () const {
  std::string result;
  result += ACM_Node::key ();
  return result;
}
std::string ACM_CodeAdvice::key () const {
  std::string result;
  result += ACM_Node::key ();
    result += advice_->key (); 
  return result;
}
std::string ACM_CodePlan::key () const {
  std::string result;
  result += ACM_Node::key ();
  return result;
}
std::string ACM_Any::key () const {
  std::string result;
  result += ACM_Node::key ();
  return result;
}
std::string ACM_Type::key () const {
  std::string result;
  result += ACM_Any::key ();
    std::ostringstream str_signature_;
    str_signature_ << signature_;
    result += str_signature_.str() + "|";
  return result;
}
std::string ACM_Arg::key () const {
  std::string result;
  result += ACM_Any::key ();
    std::ostringstream str_name_;
    str_name_ << name_;
    result += str_name_.str() + "|";
  return result;
}
std::string ACM_Name::key () const {
  std::string result;
  result += ACM_Any::key ();
    std::ostringstream str_name_;
    str_name_ << name_;
    result += str_name_.str() + "|";
  return result;
}
std::string ACM_Namespace::key () const {
  std::string result;
  result += ACM_Name::key ();
  return result;
}
std::string ACM_Function::key () const {
  std::string result;
  result += ACM_Name::key ();
    result += arg_types_.key (); 
    std::ostringstream str_cv_qualifiers_;
    str_cv_qualifiers_ << cv_qualifiers_;
    result += str_cv_qualifiers_.str() + "|";
    if (has_static_in ()) result += static_in_->key (); else result += "-|"; 
  return result;
}
std::string ACM_Variable::key () const {
  std::string result;
  result += ACM_Name::key ();
  return result;
}
std::string ACM_Class::key () const {
  std::string result;
  result += ACM_Name::key ();
  return result;
}
std::string ACM_ClassSlice::key () const {
  std::string result;
  result += ACM_Name::key ();
  return result;
}
std::string ACM_Code::key () const {
  std::string result;
  result += ACM_Any::key ();
  return result;
}
std::string ACM_Access::key () const {
  std::string result;
  result += ACM_Code::key ();
    std::ostringstream str_lid_;
    str_lid_ << lid_;
    result += str_lid_.str() + "|";
  return result;
}
std::string ACM_Get::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_Set::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_Call::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_Builtin::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_Ref::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_GetRef::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_SetRef::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_CallRef::key () const {
  std::string result;
  result += ACM_Access::key ();
  return result;
}
std::string ACM_Execution::key () const {
  std::string result;
  result += ACM_Code::key ();
  return result;
}
std::string ACM_Construction::key () const {
  std::string result;
  result += ACM_Code::key ();
  return result;
}
std::string ACM_Destruction::key () const {
  std::string result;
  result += ACM_Code::key ();
  return result;
}
std::string ACM_Pointcut::key () const {
  std::string result;
  result += ACM_Name::key ();
    result += args_.key (); 
  return result;
}
std::string ACM_Attribute::key () const {
  std::string result;
  result += ACM_Name::key ();
    result += args_.key (); 
  return result;
}
std::string ACM_Advice::key () const {
  std::string result;
  result += ACM_Any::key ();
    std::ostringstream str_lid_;
    str_lid_ << lid_;
    result += str_lid_.str() + "|";
  return result;
}
std::string ACM_AdviceCode::key () const {
  std::string result;
  result += ACM_Advice::key ();
  return result;
}
std::string ACM_Introduction::key () const {
  std::string result;
  result += ACM_Advice::key ();
  return result;
}
std::string ACM_Order::key () const {
  std::string result;
  result += ACM_Advice::key ();
  return result;
}
std::string ACM_Aspect::key () const {
  std::string result;
  result += ACM_Class::key ();
  return result;
}
std::string ACM_Speculation::key () const {
  std::string result;
  result += ACM_Node::key ();
    result += advice_->key (); 
    result += tunit_->key (); 
  return result;
}
std::string ACM_Model::key () const {
  std::string result;
  result += ACM_Node::key ();
  return result;
}


void ACM_Node::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Node (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Node::dump_attr (std::ostream &out) const {
}

void ACM_Node::dump_refs (std::ostream &out) const {
}

void ACM_Node::dump_aggr (std::ostream &out, int indent) const {
}

void ACM_File::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "File (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_File::dump_attr (std::ostream &out) const {
  out << " filename=" << "\"" << filename_ << "\"";
  out << " len=" << len_;
  out << " time=" << time_;
  ACM_Node::dump_attr (out);
}

void ACM_File::dump_refs (std::ostream &out) const {
  ACM_Node::dump_refs (out);
}

void ACM_File::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_TUnit::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "TUnit (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_TUnit::dump_attr (std::ostream &out) const {
  out << " dirty_flag=" << dirty_flag_;
  ACM_File::dump_attr (out);
}

void ACM_TUnit::dump_refs (std::ostream &out) const {
  ACM_File::dump_refs (out);
}

void ACM_TUnit::dump_aggr (std::ostream &out, int indent) const {
  ACM_File::dump_aggr (out, indent);
}

void ACM_Header::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Header (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Header::dump_attr (std::ostream &out) const {
  ACM_File::dump_attr (out);
}

void ACM_Header::dump_refs (std::ostream &out) const {
  out << " in =";
  for (ACM_Container<ACM_TUnit, false> ::const_iterator i = in_.begin(); i != in_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_File::dump_refs (out);
}

void ACM_Header::dump_aggr (std::ostream &out, int indent) const {
  ACM_File::dump_aggr (out, indent);
}

void ACM_Source::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Source (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Source::dump_attr (std::ostream &out) const {
  out << " kind=" << kind_;
  out << " line=" << line_;
  out << " len=" << len_;
  ACM_Node::dump_attr (out);
}

void ACM_Source::dump_refs (std::ostream &out) const {
  out << " file =";
  out << " " << (void*)file_;
  ACM_Node::dump_refs (out);
}

void ACM_Source::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_MemberIntro::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "MemberIntro (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_MemberIntro::dump_attr (std::ostream &out) const {
  ACM_Node::dump_attr (out);
}

void ACM_MemberIntro::dump_refs (std::ostream &out) const {
  out << " intro =";
  out << " " << (void*)intro_;
  out << " members =";
  for (ACM_Container<ACM_Name, false> ::const_iterator i = members_.begin(); i != members_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Node::dump_refs (out);
}

void ACM_MemberIntro::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_BaseIntro::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "BaseIntro (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_BaseIntro::dump_attr (std::ostream &out) const {
  ACM_Node::dump_attr (out);
}

void ACM_BaseIntro::dump_refs (std::ostream &out) const {
  out << " intro =";
  out << " " << (void*)intro_;
  out << " bases =";
  for (ACM_Container<ACM_Class, false> ::const_iterator i = bases_.begin(); i != bases_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Node::dump_refs (out);
}

void ACM_BaseIntro::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_ClassPlan::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "ClassPlan (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_ClassPlan::dump_attr (std::ostream &out) const {
  ACM_Node::dump_attr (out);
}

void ACM_ClassPlan::dump_refs (std::ostream &out) const {
  ACM_Node::dump_refs (out);
}

void ACM_ClassPlan::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_BaseIntro, true> ::const_iterator i = base_intros_.begin(); i != base_intros_.end(); ++i)
    (*i)->dump (out, indent + 1);
  for (ACM_Container<ACM_MemberIntro, true> ::const_iterator i = member_intros_.begin(); i != member_intros_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Node::dump_aggr (out, indent);
}

void ACM_CodeAdvice::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "CodeAdvice (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_CodeAdvice::dump_attr (std::ostream &out) const {
  out << " conditional=" << conditional_;
  ACM_Node::dump_attr (out);
}

void ACM_CodeAdvice::dump_refs (std::ostream &out) const {
  out << " advice =";
  out << " " << (void*)advice_;
  ACM_Node::dump_refs (out);
}

void ACM_CodeAdvice::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_CodePlan::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "CodePlan (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_CodePlan::dump_attr (std::ostream &out) const {
  ACM_Node::dump_attr (out);
}

void ACM_CodePlan::dump_refs (std::ostream &out) const {
  ACM_Node::dump_refs (out);
}

void ACM_CodePlan::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_CodeAdvice, true> ::const_iterator i = before_.begin(); i != before_.end(); ++i)
    (*i)->dump (out, indent + 1);
    if (has_around ())
  around_->dump (out, indent + 1);
  for (ACM_Container<ACM_CodeAdvice, true> ::const_iterator i = after_.begin(); i != after_.end(); ++i)
    (*i)->dump (out, indent + 1);
    if (has_next_level ())
  next_level_->dump (out, indent + 1);
  ACM_Node::dump_aggr (out, indent);
}

void ACM_Any::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Any (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Any::dump_attr (std::ostream &out) const {
  if (has_jpid ())
  out << " jpid=" << jpid_;
  ACM_Node::dump_attr (out);
}

void ACM_Any::dump_refs (std::ostream &out) const {
  out << " attributes =";
  for (ACM_Container<ACM_Attribute, false> ::const_iterator i = attributes_.begin(); i != attributes_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Node::dump_refs (out);
}

void ACM_Any::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Source, true> ::const_iterator i = source_.begin(); i != source_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Node::dump_aggr (out, indent);
}

void ACM_Type::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Type (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Type::dump_attr (std::ostream &out) const {
  out << " signature=" << "\"" << signature_ << "\"";
  out << " match_sig=" << match_sig_;
  ACM_Any::dump_attr (out);
}

void ACM_Type::dump_refs (std::ostream &out) const {
  ACM_Any::dump_refs (out);
}

void ACM_Type::dump_aggr (std::ostream &out, int indent) const {
  ACM_Any::dump_aggr (out, indent);
}

void ACM_Arg::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Arg (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Arg::dump_attr (std::ostream &out) const {
  out << " type=" << "\"" << type_ << "\"";
  out << " name=" << "\"" << name_ << "\"";
  ACM_Any::dump_attr (out);
}

void ACM_Arg::dump_refs (std::ostream &out) const {
  ACM_Any::dump_refs (out);
}

void ACM_Arg::dump_aggr (std::ostream &out, int indent) const {
  ACM_Any::dump_aggr (out, indent);
}

void ACM_Name::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Name (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Name::dump_attr (std::ostream &out) const {
  out << " name=" << "\"" << name_ << "\"";
  out << " match_sig=" << match_sig_;
  out << " name_map=" << name_map_;
  out << " builtin=" << builtin_;
  ACM_Any::dump_attr (out);
}

void ACM_Name::dump_refs (std::ostream &out) const {
  out << " tunits =";
  for (ACM_Container<ACM_TUnit, false> ::const_iterator i = tunits_.begin(); i != tunits_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Any::dump_refs (out);
}

void ACM_Name::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Any, true> ::const_iterator i = children_.begin(); i != children_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Any::dump_aggr (out, indent);
}

void ACM_Namespace::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Namespace (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Namespace::dump_attr (std::ostream &out) const {
  ACM_Name::dump_attr (out);
}

void ACM_Namespace::dump_refs (std::ostream &out) const {
  ACM_Name::dump_refs (out);
}

void ACM_Namespace::dump_aggr (std::ostream &out, int indent) const {
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Function::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Function (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Function::dump_attr (std::ostream &out) const {
  out << " kind=" << kind_;
  out << " variadic_args=" << variadic_args_;
  out << " cv_qualifiers=" << cv_qualifiers_;
  ACM_Name::dump_attr (out);
}

void ACM_Function::dump_refs (std::ostream &out) const {
  out << " static_in =";
  out << " " << (void*)static_in_;
  out << " calls =";
  for (ACM_Container<ACM_Call, false> ::const_iterator i = calls_.begin(); i != calls_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Name::dump_refs (out);
}

void ACM_Function::dump_aggr (std::ostream &out, int indent) const {
    if (has_result_type ())
  result_type_->dump (out, indent + 1);
  for (ACM_Container<ACM_Type, true> ::const_iterator i = arg_types_.begin(); i != arg_types_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Variable::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Variable (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Variable::dump_attr (std::ostream &out) const {
  out << " kind=" << kind_;
  ACM_Name::dump_attr (out);
}

void ACM_Variable::dump_refs (std::ostream &out) const {
  ACM_Name::dump_refs (out);
}

void ACM_Variable::dump_aggr (std::ostream &out, int indent) const {
  type_->dump (out, indent + 1);
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Class::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Class (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Class::dump_attr (std::ostream &out) const {
  out << " intro_target=" << intro_target_;
  ACM_Name::dump_attr (out);
}

void ACM_Class::dump_refs (std::ostream &out) const {
  out << " bases =";
  for (ACM_Container<ACM_Class, false> ::const_iterator i = bases_.begin(); i != bases_.end(); ++i)
    out << " " << (void*)(*i);
  out << " derived =";
  for (ACM_Container<ACM_Class, false> ::const_iterator i = derived_.begin(); i != derived_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Name::dump_refs (out);
}

void ACM_Class::dump_aggr (std::ostream &out, int indent) const {
    if (has_plan ())
  plan_->dump (out, indent + 1);
  ACM_Name::dump_aggr (out, indent);
}

void ACM_ClassSlice::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "ClassSlice (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_ClassSlice::dump_attr (std::ostream &out) const {
  out << " is_struct=" << is_struct_;
  ACM_Name::dump_attr (out);
}

void ACM_ClassSlice::dump_refs (std::ostream &out) const {
  ACM_Name::dump_refs (out);
}

void ACM_ClassSlice::dump_aggr (std::ostream &out, int indent) const {
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Code::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Code (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Code::dump_attr (std::ostream &out) const {
  ACM_Any::dump_attr (out);
}

void ACM_Code::dump_refs (std::ostream &out) const {
  ACM_Any::dump_refs (out);
}

void ACM_Code::dump_aggr (std::ostream &out, int indent) const {
    if (has_plan ())
  plan_->dump (out, indent + 1);
  ACM_Any::dump_aggr (out, indent);
}

void ACM_Access::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Access (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Access::dump_attr (std::ostream &out) const {
  out << " lid=" << lid_;
  if (has_target_object_lid ())
  out << " target_object_lid=" << target_object_lid_;
  if (has_cfg_block_lid ())
  out << " cfg_block_lid=" << cfg_block_lid_;
  ACM_Code::dump_attr (out);
}

void ACM_Access::dump_refs (std::ostream &out) const {
  out << " target_class =";
  out << " " << (void*)target_class_;
  ACM_Code::dump_refs (out);
}

void ACM_Access::dump_aggr (std::ostream &out, int indent) const {
  ACM_Code::dump_aggr (out, indent);
}

void ACM_Get::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Get (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Get::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_Get::dump_refs (std::ostream &out) const {
  out << " variable =";
  out << " " << (void*)variable_;
  ACM_Access::dump_refs (out);
}

void ACM_Get::dump_aggr (std::ostream &out, int indent) const {
  ACM_Access::dump_aggr (out, indent);
}

void ACM_Set::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Set (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Set::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_Set::dump_refs (std::ostream &out) const {
  out << " variable =";
  out << " " << (void*)variable_;
  ACM_Access::dump_refs (out);
}

void ACM_Set::dump_aggr (std::ostream &out, int indent) const {
  ACM_Access::dump_aggr (out, indent);
}

void ACM_Call::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Call (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Call::dump_attr (std::ostream &out) const {
  if (has_default_args ())
  out << " default_args=" << default_args_;
  ACM_Access::dump_attr (out);
}

void ACM_Call::dump_refs (std::ostream &out) const {
  out << " target =";
  out << " " << (void*)target_;
  ACM_Access::dump_refs (out);
}

void ACM_Call::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Type, true> ::const_iterator i = variadic_arg_types_.begin(); i != variadic_arg_types_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_Builtin::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Builtin (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Builtin::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_Builtin::dump_refs (std::ostream &out) const {
  out << " target =";
  out << " " << (void*)target_;
  ACM_Access::dump_refs (out);
}

void ACM_Builtin::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Access, true> ::const_iterator i = implicit_access_.begin(); i != implicit_access_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_Ref::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Ref (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Ref::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_Ref::dump_refs (std::ostream &out) const {
  out << " variable =";
  out << " " << (void*)variable_;
  ACM_Access::dump_refs (out);
}

void ACM_Ref::dump_aggr (std::ostream &out, int indent) const {
  type_->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_GetRef::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "GetRef (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_GetRef::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_GetRef::dump_refs (std::ostream &out) const {
  ACM_Access::dump_refs (out);
}

void ACM_GetRef::dump_aggr (std::ostream &out, int indent) const {
  type_->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_SetRef::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "SetRef (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_SetRef::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_SetRef::dump_refs (std::ostream &out) const {
  ACM_Access::dump_refs (out);
}

void ACM_SetRef::dump_aggr (std::ostream &out, int indent) const {
  type_->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_CallRef::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "CallRef (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_CallRef::dump_attr (std::ostream &out) const {
  ACM_Access::dump_attr (out);
}

void ACM_CallRef::dump_refs (std::ostream &out) const {
  ACM_Access::dump_refs (out);
}

void ACM_CallRef::dump_aggr (std::ostream &out, int indent) const {
  type_->dump (out, indent + 1);
  ACM_Access::dump_aggr (out, indent);
}

void ACM_Execution::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Execution (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Execution::dump_attr (std::ostream &out) const {
  ACM_Code::dump_attr (out);
}

void ACM_Execution::dump_refs (std::ostream &out) const {
  ACM_Code::dump_refs (out);
}

void ACM_Execution::dump_aggr (std::ostream &out, int indent) const {
  ACM_Code::dump_aggr (out, indent);
}

void ACM_Construction::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Construction (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Construction::dump_attr (std::ostream &out) const {
  ACM_Code::dump_attr (out);
}

void ACM_Construction::dump_refs (std::ostream &out) const {
  ACM_Code::dump_refs (out);
}

void ACM_Construction::dump_aggr (std::ostream &out, int indent) const {
  ACM_Code::dump_aggr (out, indent);
}

void ACM_Destruction::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Destruction (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Destruction::dump_attr (std::ostream &out) const {
  ACM_Code::dump_attr (out);
}

void ACM_Destruction::dump_refs (std::ostream &out) const {
  ACM_Code::dump_refs (out);
}

void ACM_Destruction::dump_aggr (std::ostream &out, int indent) const {
  ACM_Code::dump_aggr (out, indent);
}

void ACM_Pointcut::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Pointcut (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Pointcut::dump_attr (std::ostream &out) const {
  out << " expr=" << "\"" << expr_ << "\"";
  out << " kind=" << kind_;
  out << " parsed_expr=" << parsed_expr_;
  ACM_Name::dump_attr (out);
}

void ACM_Pointcut::dump_refs (std::ostream &out) const {
  ACM_Name::dump_refs (out);
}

void ACM_Pointcut::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Arg, true> ::const_iterator i = args_.begin(); i != args_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Attribute::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Attribute (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Attribute::dump_attr (std::ostream &out) const {
  ACM_Name::dump_attr (out);
}

void ACM_Attribute::dump_refs (std::ostream &out) const {
  ACM_Name::dump_refs (out);
}

void ACM_Attribute::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Arg, true> ::const_iterator i = args_.begin(); i != args_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Name::dump_aggr (out, indent);
}

void ACM_Advice::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Advice (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Advice::dump_attr (std::ostream &out) const {
  out << " lid=" << lid_;
  ACM_Any::dump_attr (out);
}

void ACM_Advice::dump_refs (std::ostream &out) const {
  ACM_Any::dump_refs (out);
}

void ACM_Advice::dump_aggr (std::ostream &out, int indent) const {
  pointcut_->dump (out, indent + 1);
  ACM_Any::dump_aggr (out, indent);
}

void ACM_AdviceCode::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "AdviceCode (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_AdviceCode::dump_attr (std::ostream &out) const {
  out << " kind=" << kind_;
  out << " context=" << context_;
  ACM_Advice::dump_attr (out);
}

void ACM_AdviceCode::dump_refs (std::ostream &out) const {
  ACM_Advice::dump_refs (out);
}

void ACM_AdviceCode::dump_aggr (std::ostream &out, int indent) const {
  ACM_Advice::dump_aggr (out, indent);
}

void ACM_Introduction::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Introduction (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Introduction::dump_attr (std::ostream &out) const {
  ACM_Advice::dump_attr (out);
}

void ACM_Introduction::dump_refs (std::ostream &out) const {
  out << " named_slice =";
  out << " " << (void*)named_slice_;
  ACM_Advice::dump_refs (out);
}

void ACM_Introduction::dump_aggr (std::ostream &out, int indent) const {
    if (has_anon_slice ())
  anon_slice_->dump (out, indent + 1);
  ACM_Advice::dump_aggr (out, indent);
}

void ACM_Order::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Order (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Order::dump_attr (std::ostream &out) const {
  ACM_Advice::dump_attr (out);
}

void ACM_Order::dump_refs (std::ostream &out) const {
  ACM_Advice::dump_refs (out);
}

void ACM_Order::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Pointcut, true> ::const_iterator i = aspect_exprs_.begin(); i != aspect_exprs_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Advice::dump_aggr (out, indent);
}

void ACM_Aspect::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Aspect (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Aspect::dump_attr (std::ostream &out) const {
  ACM_Class::dump_attr (out);
}

void ACM_Aspect::dump_refs (std::ostream &out) const {
  ACM_Class::dump_refs (out);
}

void ACM_Aspect::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_Introduction, true> ::const_iterator i = intros_.begin(); i != intros_.end(); ++i)
    (*i)->dump (out, indent + 1);
  for (ACM_Container<ACM_AdviceCode, true> ::const_iterator i = advices_.begin(); i != advices_.end(); ++i)
    (*i)->dump (out, indent + 1);
  for (ACM_Container<ACM_Order, true> ::const_iterator i = orders_.begin(); i != orders_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Class::dump_aggr (out, indent);
}

void ACM_Speculation::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Speculation (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Speculation::dump_attr (std::ostream &out) const {
  ACM_Node::dump_attr (out);
}

void ACM_Speculation::dump_refs (std::ostream &out) const {
  out << " advice =";
  out << " " << (void*)advice_;
  out << " tunit =";
  out << " " << (void*)tunit_;
  out << " true_jps =";
  for (ACM_Container<ACM_Any, false> ::const_iterator i = true_jps_.begin(); i != true_jps_.end(); ++i)
    out << " " << (void*)(*i);
  out << " false_jps =";
  for (ACM_Container<ACM_Any, false> ::const_iterator i = false_jps_.begin(); i != false_jps_.end(); ++i)
    out << " " << (void*)(*i);
  out << " conditional_jps =";
  for (ACM_Container<ACM_Any, false> ::const_iterator i = conditional_jps_.begin(); i != conditional_jps_.end(); ++i)
    out << " " << (void*)(*i);
  ACM_Node::dump_refs (out);
}

void ACM_Speculation::dump_aggr (std::ostream &out, int indent) const {
  ACM_Node::dump_aggr (out, indent);
}

void ACM_Model::dump (std::ostream &out, int indent) const {
  for (int i = 0; i < indent; i++) cout << "  ";
  out << "Model (" << this << " " << get_parent() << ")"; dump_attr (out); dump_refs (out);
  out << endl;
  dump_aggr (out, indent);
}

void ACM_Model::dump_attr (std::ostream &out) const {
  out << " version=" << "\"" << version_ << "\"";
  ACM_Node::dump_attr (out);
}

void ACM_Model::dump_refs (std::ostream &out) const {
  ACM_Node::dump_refs (out);
}

void ACM_Model::dump_aggr (std::ostream &out, int indent) const {
  for (ACM_Container<ACM_File, true> ::const_iterator i = files_.begin(); i != files_.end(); ++i)
    (*i)->dump (out, indent + 1);
  root_->dump (out, indent + 1);
  for (ACM_Container<ACM_Speculation, true> ::const_iterator i = speculations_.begin(); i != speculations_.end(); ++i)
    (*i)->dump (out, indent + 1);
  ACM_Node::dump_aggr (out, indent);
}

void ACM_Node::to_xml (SaveContext &context) {
  remove_property ("id");
  if (get_id () != -1)
    set_property ("id", get_id ());
}

void ACM_File::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("filename", filename_);
  if (get_xml_dirty ()) set_property ("len", len_);
  if (get_xml_dirty ()) set_property ("time", time_);
  ACM_Node::to_xml (context);
}

void ACM_TUnit::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("dirty_flag", dirty_flag_);
  ACM_File::to_xml (context);
}

void ACM_Header::to_xml (SaveContext &context) {
  if (in_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_TUnit, false> ::iterator i = in_.begin (); i != in_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("in", idrefs.str ());
  }
  else {
    remove_property ("in");
  }
  ACM_File::to_xml (context);
}

void ACM_Source::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("kind", kind_);
  set_property ("file", file_->assign_id(context)); 
  if (get_xml_dirty ()) set_property ("line", line_);
  if (get_xml_dirty ()) set_property ("len", len_);
  ACM_Node::to_xml (context);
}

void ACM_MemberIntro::to_xml (SaveContext &context) {
  set_property ("intro", intro_->assign_id(context)); 
  if (members_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Name, false> ::iterator i = members_.begin (); i != members_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("members", idrefs.str ());
  }
  else {
    remove_property ("members");
  }
  ACM_Node::to_xml (context);
}

void ACM_BaseIntro::to_xml (SaveContext &context) {
  set_property ("intro", intro_->assign_id(context)); 
  if (bases_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Class, false> ::iterator i = bases_.begin (); i != bases_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("bases", idrefs.str ());
  }
  else {
    remove_property ("bases");
  }
  ACM_Node::to_xml (context);
}

void ACM_ClassPlan::to_xml (SaveContext &context) {
  if (base_intros_.size () > 0) {
    if (!base_intros_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"base_intros", NULL);
      base_intros_.set_xml_node (node);
    }
    base_intros_.to_xml (context);
    base_intros_.set_xml_dirty (false);
  }
  else {
    if (base_intros_.get_xml_node ()) {
      xmlUnlinkNode (base_intros_.get_xml_node ());
      xmlFreeNode (base_intros_.get_xml_node ());
      base_intros_.set_xml_node (0);
      base_intros_.set_xml_dirty ();
    }
  }
  
  if (member_intros_.size () > 0) {
    if (!member_intros_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"member_intros", NULL);
      member_intros_.set_xml_node (node);
    }
    member_intros_.to_xml (context);
    member_intros_.set_xml_dirty (false);
  }
  else {
    if (member_intros_.get_xml_node ()) {
      xmlUnlinkNode (member_intros_.get_xml_node ());
      xmlFreeNode (member_intros_.get_xml_node ());
      member_intros_.set_xml_node (0);
      member_intros_.set_xml_dirty ();
    }
  }
  
  ACM_Node::to_xml (context);
}

void ACM_CodeAdvice::to_xml (SaveContext &context) {
  set_property ("advice", advice_->assign_id(context)); 
  if (get_xml_dirty ()) set_property ("conditional", conditional_);
  ACM_Node::to_xml (context);
}

void ACM_CodePlan::to_xml (SaveContext &context) {
  if (before_.size () > 0) {
    if (!before_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"before", NULL);
      before_.set_xml_node (node);
    }
    before_.to_xml (context);
    before_.set_xml_dirty (false);
  }
  else {
    if (before_.get_xml_node ()) {
      xmlUnlinkNode (before_.get_xml_node ());
      xmlFreeNode (before_.get_xml_node ());
      before_.set_xml_node (0);
      before_.set_xml_dirty ();
    }
  }
  
  if (has_around ()) {
    if (!around_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"around", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)around_->type_str (), NULL);
      around_->set_xml_node (node);
    }
    around_->to_xml (context);
    around_->set_xml_dirty (false);
  }
  if (after_.size () > 0) {
    if (!after_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"after", NULL);
      after_.set_xml_node (node);
    }
    after_.to_xml (context);
    after_.set_xml_dirty (false);
  }
  else {
    if (after_.get_xml_node ()) {
      xmlUnlinkNode (after_.get_xml_node ());
      xmlFreeNode (after_.get_xml_node ());
      after_.set_xml_node (0);
      after_.set_xml_dirty ();
    }
  }
  
  if (has_next_level ()) {
    if (!next_level_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"next_level", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)next_level_->type_str (), NULL);
      next_level_->set_xml_node (node);
    }
    next_level_->to_xml (context);
    next_level_->set_xml_dirty (false);
  }
  ACM_Node::to_xml (context);
}

void ACM_Any::to_xml (SaveContext &context) {
  if (has_jpid ())  
  if (get_xml_dirty ()) set_property ("jpid", jpid_);
  if (source_.size () > 0) {
    if (!source_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"source", NULL);
      source_.set_xml_node (node);
    }
    source_.to_xml (context);
    source_.set_xml_dirty (false);
  }
  else {
    if (source_.get_xml_node ()) {
      xmlUnlinkNode (source_.get_xml_node ());
      xmlFreeNode (source_.get_xml_node ());
      source_.set_xml_node (0);
      source_.set_xml_dirty ();
    }
  }
  
  if (attributes_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Attribute, false> ::iterator i = attributes_.begin (); i != attributes_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("attributes", idrefs.str ());
  }
  else {
    remove_property ("attributes");
  }
  ACM_Node::to_xml (context);
}

void ACM_Type::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("signature", signature_);
  ACM_Any::to_xml (context);
}

void ACM_Arg::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("type", type_);
  if (get_xml_dirty ()) set_property ("name", name_);
  ACM_Any::to_xml (context);
}

void ACM_Name::to_xml (SaveContext &context) {
  if (children_.size () > 0) {
    if (!children_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"children", NULL);
      children_.set_xml_node (node);
    }
    children_.to_xml (context);
    children_.set_xml_dirty (false);
  }
  else {
    if (children_.get_xml_node ()) {
      xmlUnlinkNode (children_.get_xml_node ());
      xmlFreeNode (children_.get_xml_node ());
      children_.set_xml_node (0);
      children_.set_xml_dirty ();
    }
  }
  
  if (get_xml_dirty ()) set_property ("name", name_);
  if (get_xml_dirty ()) set_property ("builtin", builtin_);
  if (tunits_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_TUnit, false> ::iterator i = tunits_.begin (); i != tunits_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("tunits", idrefs.str ());
  }
  else {
    remove_property ("tunits");
  }
  ACM_Any::to_xml (context);
}

void ACM_Namespace::to_xml (SaveContext &context) {
  ACM_Name::to_xml (context);
}

void ACM_Function::to_xml (SaveContext &context) {
  if (has_result_type ()) {
    if (!result_type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"result_type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)result_type_->type_str (), NULL);
      result_type_->set_xml_node (node);
    }
    result_type_->to_xml (context);
    result_type_->set_xml_dirty (false);
  }
  if (arg_types_.size () > 0) {
    if (!arg_types_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"arg_types", NULL);
      arg_types_.set_xml_node (node);
    }
    arg_types_.to_xml (context);
    arg_types_.set_xml_dirty (false);
  }
  else {
    if (arg_types_.get_xml_node ()) {
      xmlUnlinkNode (arg_types_.get_xml_node ());
      xmlFreeNode (arg_types_.get_xml_node ());
      arg_types_.set_xml_node (0);
      arg_types_.set_xml_dirty ();
    }
  }
  
  if (get_xml_dirty ()) set_property ("kind", kind_);
  if (get_xml_dirty ()) set_property ("variadic_args", variadic_args_);
  if (get_xml_dirty ()) set_property ("cv_qualifiers", cv_qualifiers_);
  if (has_static_in ())
  set_property ("static_in", static_in_->assign_id(context)); 
  ACM_Name::to_xml (context);
}

void ACM_Variable::to_xml (SaveContext &context) {
    if (!type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)type_->type_str (), NULL);
      type_->set_xml_node (node);
    }
    type_->to_xml (context);
    type_->set_xml_dirty (false);
  if (get_xml_dirty ()) set_property ("kind", kind_);
  ACM_Name::to_xml (context);
}

void ACM_Class::to_xml (SaveContext &context) {
  if (bases_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Class, false> ::iterator i = bases_.begin (); i != bases_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("bases", idrefs.str ());
  }
  else {
    remove_property ("bases");
  }
  if (has_plan ()) {
    if (!plan_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"plan", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)plan_->type_str (), NULL);
      plan_->set_xml_node (node);
    }
    plan_->to_xml (context);
    plan_->set_xml_dirty (false);
  }
  ACM_Name::to_xml (context);
}

void ACM_ClassSlice::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("is_struct", is_struct_);
  ACM_Name::to_xml (context);
}

void ACM_Code::to_xml (SaveContext &context) {
  if (has_plan ()) {
    if (!plan_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"plan", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)plan_->type_str (), NULL);
      plan_->set_xml_node (node);
    }
    plan_->to_xml (context);
    plan_->set_xml_dirty (false);
  }
  ACM_Any::to_xml (context);
}

void ACM_Access::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("lid", lid_);
  if (has_target_object_lid ())  
  if (get_xml_dirty ()) set_property ("target_object_lid", target_object_lid_);
  if (has_cfg_block_lid ())  
  if (get_xml_dirty ()) set_property ("cfg_block_lid", cfg_block_lid_);
  if (has_target_class ())
  set_property ("target_class", target_class_->assign_id(context)); 
  ACM_Code::to_xml (context);
}

void ACM_Get::to_xml (SaveContext &context) {
  if (has_variable ())
  set_property ("variable", variable_->assign_id(context)); 
  ACM_Access::to_xml (context);
}

void ACM_Set::to_xml (SaveContext &context) {
  if (has_variable ())
  set_property ("variable", variable_->assign_id(context)); 
  ACM_Access::to_xml (context);
}

void ACM_Call::to_xml (SaveContext &context) {
  set_property ("target", target_->assign_id(context)); 
  if (variadic_arg_types_.size () > 0) {
    if (!variadic_arg_types_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"variadic_arg_types", NULL);
      variadic_arg_types_.set_xml_node (node);
    }
    variadic_arg_types_.to_xml (context);
    variadic_arg_types_.set_xml_dirty (false);
  }
  else {
    if (variadic_arg_types_.get_xml_node ()) {
      xmlUnlinkNode (variadic_arg_types_.get_xml_node ());
      xmlFreeNode (variadic_arg_types_.get_xml_node ());
      variadic_arg_types_.set_xml_node (0);
      variadic_arg_types_.set_xml_dirty ();
    }
  }
  
  if (has_default_args ())  
  if (get_xml_dirty ()) set_property ("default_args", default_args_);
  ACM_Access::to_xml (context);
}

void ACM_Builtin::to_xml (SaveContext &context) {
  set_property ("target", target_->assign_id(context)); 
  if (implicit_access_.size () > 0) {
    if (!implicit_access_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"implicit_access", NULL);
      implicit_access_.set_xml_node (node);
    }
    implicit_access_.to_xml (context);
    implicit_access_.set_xml_dirty (false);
  }
  else {
    if (implicit_access_.get_xml_node ()) {
      xmlUnlinkNode (implicit_access_.get_xml_node ());
      xmlFreeNode (implicit_access_.get_xml_node ());
      implicit_access_.set_xml_node (0);
      implicit_access_.set_xml_dirty ();
    }
  }
  
  ACM_Access::to_xml (context);
}

void ACM_Ref::to_xml (SaveContext &context) {
  set_property ("variable", variable_->assign_id(context)); 
    if (!type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)type_->type_str (), NULL);
      type_->set_xml_node (node);
    }
    type_->to_xml (context);
    type_->set_xml_dirty (false);
  ACM_Access::to_xml (context);
}

void ACM_GetRef::to_xml (SaveContext &context) {
    if (!type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)type_->type_str (), NULL);
      type_->set_xml_node (node);
    }
    type_->to_xml (context);
    type_->set_xml_dirty (false);
  ACM_Access::to_xml (context);
}

void ACM_SetRef::to_xml (SaveContext &context) {
    if (!type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)type_->type_str (), NULL);
      type_->set_xml_node (node);
    }
    type_->to_xml (context);
    type_->set_xml_dirty (false);
  ACM_Access::to_xml (context);
}

void ACM_CallRef::to_xml (SaveContext &context) {
    if (!type_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"type", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)type_->type_str (), NULL);
      type_->set_xml_node (node);
    }
    type_->to_xml (context);
    type_->set_xml_dirty (false);
  ACM_Access::to_xml (context);
}

void ACM_Execution::to_xml (SaveContext &context) {
  ACM_Code::to_xml (context);
}

void ACM_Construction::to_xml (SaveContext &context) {
  ACM_Code::to_xml (context);
}

void ACM_Destruction::to_xml (SaveContext &context) {
  ACM_Code::to_xml (context);
}

void ACM_Pointcut::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("expr", expr_);
  if (args_.size () > 0) {
    if (!args_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"args", NULL);
      args_.set_xml_node (node);
    }
    args_.to_xml (context);
    args_.set_xml_dirty (false);
  }
  else {
    if (args_.get_xml_node ()) {
      xmlUnlinkNode (args_.get_xml_node ());
      xmlFreeNode (args_.get_xml_node ());
      args_.set_xml_node (0);
      args_.set_xml_dirty ();
    }
  }
  
  if (get_xml_dirty ()) set_property ("kind", kind_);
  ACM_Name::to_xml (context);
}

void ACM_Attribute::to_xml (SaveContext &context) {
  if (args_.size () > 0) {
    if (!args_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"args", NULL);
      args_.set_xml_node (node);
    }
    args_.to_xml (context);
    args_.set_xml_dirty (false);
  }
  else {
    if (args_.get_xml_node ()) {
      xmlUnlinkNode (args_.get_xml_node ());
      xmlFreeNode (args_.get_xml_node ());
      args_.set_xml_node (0);
      args_.set_xml_dirty ();
    }
  }
  
  ACM_Name::to_xml (context);
}

void ACM_Advice::to_xml (SaveContext &context) {
    if (!pointcut_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"pointcut", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)pointcut_->type_str (), NULL);
      pointcut_->set_xml_node (node);
    }
    pointcut_->to_xml (context);
    pointcut_->set_xml_dirty (false);
  if (get_xml_dirty ()) set_property ("lid", lid_);
  ACM_Any::to_xml (context);
}

void ACM_AdviceCode::to_xml (SaveContext &context) {
  if (get_xml_dirty ()) set_property ("kind", kind_);
  if (get_xml_dirty ()) set_property ("context", context_);
  ACM_Advice::to_xml (context);
}

void ACM_Introduction::to_xml (SaveContext &context) {
  if (has_named_slice ())
  set_property ("named_slice", named_slice_->assign_id(context)); 
  if (has_anon_slice ()) {
    if (!anon_slice_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"anon_slice", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)anon_slice_->type_str (), NULL);
      anon_slice_->set_xml_node (node);
    }
    anon_slice_->to_xml (context);
    anon_slice_->set_xml_dirty (false);
  }
  ACM_Advice::to_xml (context);
}

void ACM_Order::to_xml (SaveContext &context) {
  if (aspect_exprs_.size () > 0) {
    if (!aspect_exprs_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"aspect_exprs", NULL);
      aspect_exprs_.set_xml_node (node);
    }
    aspect_exprs_.to_xml (context);
    aspect_exprs_.set_xml_dirty (false);
  }
  else {
    if (aspect_exprs_.get_xml_node ()) {
      xmlUnlinkNode (aspect_exprs_.get_xml_node ());
      xmlFreeNode (aspect_exprs_.get_xml_node ());
      aspect_exprs_.set_xml_node (0);
      aspect_exprs_.set_xml_dirty ();
    }
  }
  
  ACM_Advice::to_xml (context);
}

void ACM_Aspect::to_xml (SaveContext &context) {
  if (intros_.size () > 0) {
    if (!intros_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"intros", NULL);
      intros_.set_xml_node (node);
    }
    intros_.to_xml (context);
    intros_.set_xml_dirty (false);
  }
  else {
    if (intros_.get_xml_node ()) {
      xmlUnlinkNode (intros_.get_xml_node ());
      xmlFreeNode (intros_.get_xml_node ());
      intros_.set_xml_node (0);
      intros_.set_xml_dirty ();
    }
  }
  
  if (advices_.size () > 0) {
    if (!advices_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"advices", NULL);
      advices_.set_xml_node (node);
    }
    advices_.to_xml (context);
    advices_.set_xml_dirty (false);
  }
  else {
    if (advices_.get_xml_node ()) {
      xmlUnlinkNode (advices_.get_xml_node ());
      xmlFreeNode (advices_.get_xml_node ());
      advices_.set_xml_node (0);
      advices_.set_xml_dirty ();
    }
  }
  
  if (orders_.size () > 0) {
    if (!orders_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"orders", NULL);
      orders_.set_xml_node (node);
    }
    orders_.to_xml (context);
    orders_.set_xml_dirty (false);
  }
  else {
    if (orders_.get_xml_node ()) {
      xmlUnlinkNode (orders_.get_xml_node ());
      xmlFreeNode (orders_.get_xml_node ());
      orders_.set_xml_node (0);
      orders_.set_xml_dirty ();
    }
  }
  
  ACM_Class::to_xml (context);
}

void ACM_Speculation::to_xml (SaveContext &context) {
  set_property ("advice", advice_->assign_id(context)); 
  set_property ("tunit", tunit_->assign_id(context)); 
  if (true_jps_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Any, false> ::iterator i = true_jps_.begin (); i != true_jps_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("true_jps", idrefs.str ());
  }
  else {
    remove_property ("true_jps");
  }
  if (false_jps_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Any, false> ::iterator i = false_jps_.begin (); i != false_jps_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("false_jps", idrefs.str ());
  }
  else {
    remove_property ("false_jps");
  }
  if (conditional_jps_.size () > 0) {
    ostringstream idrefs; bool first = true;
    for (ACM_Container<ACM_Any, false> ::iterator i = conditional_jps_.begin (); i != conditional_jps_.end (); ++i) {
      (*i)->assign_id (context);
      if (!first) idrefs << " "; else first = false;
      idrefs << (*i)->get_id ();
    }
    set_property ("conditional_jps", idrefs.str ());
  }
  else {
    remove_property ("conditional_jps");
  }
  ACM_Node::to_xml (context);
}

void ACM_Model::to_xml (SaveContext &context) {
  if (files_.size () > 0) {
    if (!files_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"files", NULL);
      files_.set_xml_node (node);
    }
    files_.to_xml (context);
    files_.set_xml_dirty (false);
  }
  else {
    if (files_.get_xml_node ()) {
      xmlUnlinkNode (files_.get_xml_node ());
      xmlFreeNode (files_.get_xml_node ());
      files_.set_xml_node (0);
      files_.set_xml_dirty ();
    }
  }
  
    if (!root_->get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"root", NULL);
      node = xmlNewChild(node, NULL, (xmlChar*)root_->type_str (), NULL);
      root_->set_xml_node (node);
    }
    root_->to_xml (context);
    root_->set_xml_dirty (false);
  if (speculations_.size () > 0) {
    if (!speculations_.get_xml_node ()) {
      xmlNodePtr node = xmlNewChild(get_xml_node (), NULL, (xmlChar*)"speculations", NULL);
      speculations_.set_xml_node (node);
    }
    speculations_.to_xml (context);
    speculations_.set_xml_dirty (false);
  }
  else {
    if (speculations_.get_xml_node ()) {
      xmlUnlinkNode (speculations_.get_xml_node ());
      xmlFreeNode (speculations_.get_xml_node ());
      speculations_.set_xml_node (0);
      speculations_.set_xml_dirty ();
    }
  }
  
  if (get_xml_dirty ()) set_property ("version", version_);
  ACM_Node::to_xml (context);
}

// model facade class
// delete all model elements from a Project Model
void ProjectModel::clear () {
  for (int l = 0; l < MAX_JOIN_POINT_TYPE_INDEX; l++) {
    for (NodeList::iterator i = _join_points[l].begin ();
      i != _join_points[l].end (); ++i)
      delete *i;
    _join_points[l].clear ();
  }
  set_root (0);
  get_files ().clear ();
  if (_xml_doc) {
    xmlFreeDoc (_xml_doc); // delete the DOM
    _xml_doc = 0;
  }
}

// allocated a new joinpoint ID
unsigned ProjectModel::alloc_jpid () {
  unsigned result;
  if (_jpids_freed.empty ()) {
    result = _jpids_in_use;
    _jpids_in_use++;
  }
  else {
    result = *_jpids_freed.begin ();
    _jpids_freed.erase (_jpids_freed.begin ());
  }
  return result;
}

// mark a join point ID as allocated (needed for loading models)
void ProjectModel::alloc_jpid (unsigned jpid) {
  if (jpid >= _jpids_in_use) {
    for (unsigned id = _jpids_in_use; id < jpid; id++)
      _jpids_freed.insert (id);
    _jpids_in_use = jpid + 1;
  }
  else {
    _jpids_freed.erase (jpid);
  }
}

// free a joinpoint ID
void ProjectModel::free_jpid (unsigned jpid) {
  _jpids_freed.insert (jpid);
}

// select and return all join points of a given type (any of them)
void ProjectModel::select (JoinPointType jpt, Selection &result, bool prj_only) {
  for (int l = 0; l < MAX_JOIN_POINT_TYPE_INDEX; l++) {
    if (jpt & (1<<l)) {
      for (NodeList::iterator i = _join_points[l].begin ();
        i != _join_points[l].end (); ++i) {
        if (prj_only && ((1 << l) & (JPT_Class | JPT_Aspect))) {
          ACM_Class *jpl = (ACM_Class*)*i;
          if (!jpl->get_tunits().empty()) // external
            continue;
        }
        result.push_back (*i);
      }
    }
  }
}

// merge the elements of another model into this one
void ProjectModel::merge (ProjectModel &that) {
  _merge_id++;
  init_change_set (that);
  sourceCleanupModel (*this);
  merge_node (this, &that);
  fix_refs (this);
  clear_change_set ();
}

// before merging two models find out which files have been changed
// (file in 'that' newer as in 'this')
void ProjectModel::init_change_set(ProjectModel &that) {
  typedef ACM_Container<ACM_File, true> Container;
  Container::Set &me  = get_files().get_sorted ();
  Container::Set &you = that.get_files().get_sorted ();
  // iterate over both sorted file lists
  Container::Set::iterator curr_me = me.begin ();
  Container::Set::iterator curr_you = you.begin ();
  while (curr_me != me.end () && curr_you != you.end ()) {
    if (**curr_me < ** curr_you) // an element in 'me', which is not present
      ++curr_me;                 // in 'you' => simply skip
    else if (**curr_you < ** curr_me) { // a new element (not in 'me' yet
      ++curr_you;
    }
    else { // two equal elements according to their key
      if ((*curr_me)->get_time() != (*curr_you)->get_time()) {
        _modified_files.insert (*curr_me);
        (*curr_me)->set_time((*curr_you)->get_time());
      }
      ++curr_you;
      ++curr_me;
    }
  }
}

// cleanup list of modified files (at the end of merging)
void ProjectModel::clear_change_set() {
  _modified_files.clear ();
}

// handle attribute variations when merging models
// returns 'true' if the attribute in the project model ("me") should be
// overwritten by the attribute in the translation unit model ("that")
bool ProjectModel::merge_attr (ACM_Node &me, ACM_Node &that, const std::string &attr_name) {
  // make sure that the joinpoint IDs from the translation unit model are
  // overwritten by the IDs from the stored project model
  if (attr_name == "jpid") {
    ((ACM_Any&)that).set_jpid (((ACM_Any&)me).get_jpid ());
    return false;
  }
  // cout << "warning: attr " << attr_name << " differs" << endl;
  return true;
}

void ProjectModel::merge_attr_only_in_me (ACM_Node &me, ACM_Node &that, const std::string &attr_name) {
  cout << "warning: attr " << attr_name << " only in project model" << endl;
}

void ProjectModel::merge_attr_only_in_that (ACM_Node &me, ACM_Node &that, const std::string &attr_name) {
  cout << "warning: attr " << attr_name << " only in translation unit model" << endl;
}


// delete all source entries that are referencing a modified file
// true: *all* entries (>0) have been deleted
bool ProjectModel::purgeSources(ACM_Any &me) {
  ACM_Container<ACM_Source, true> &source_container = me.get_source();
  if (source_container.size () == 0)
    return false;
  bool needs_decl = (me.get_parent() &&
      (((ACM_Any*)me.get_parent())->type_val() & (JPT_Class|JPT_Aspect)));
  bool has_decl = false;
  bool removed_decl = false;
  bool has_def  = false;
  typedef ACM_Container<ACM_Source, true> Container;
  list<ACM_Source*> found;
  for (Container::iterator i = source_container.begin ();
      i != source_container.end (); ++i) {
    ACM_Source *src = *i;
    if (_modified_files.find (src->get_file ()) != _modified_files.end ()) {
      found.push_back (*i);
      if (src->get_kind() == SLK_DECL)
        removed_decl = true;
    }
    else {
      if (src->get_kind() == SLK_DECL)
        has_decl = true;
      else if (src->get_kind() == SLK_DEF)
        has_def = true;
    }
  }
  // For members of classes or aspects (member functions, nested classes,
  // member slices the following rule is checked:
  // If there is a definition (in a non-modified file), there also has to
  // be a declaration (within the parent class or aspect). Otherwise the
  // model element has to be deleted.
  if (has_def && removed_decl && needs_decl && !has_decl)
    return true;
  if (source_container.size () != found.size ()) {
    for (list<ACM_Source*>::iterator i = found.begin (); i != found.end (); ++i) {
      ACM_Source *src = *i;
      source_container.remove (src);
      unlink_node (src);
    }
    return false;
  }
  return true;
}

ACM_Node *ProjectModel::copyNode(ACM_Node *that_node) {
  ACM_Node *result;
  switch (that_node->type_val ()) {
    case JPT_TUnit:
      result = newTUnit();
      copyTUnit ((ACM_TUnit&)*result, (ACM_TUnit&)*that_node);
      break;
    case JPT_Header:
      result = newHeader();
      copyHeader ((ACM_Header&)*result, (ACM_Header&)*that_node);
      break;
    case JPT_Source:
      result = newSource();
      copySource ((ACM_Source&)*result, (ACM_Source&)*that_node);
      break;
    case JPT_MemberIntro:
      result = newMemberIntro();
      copyMemberIntro ((ACM_MemberIntro&)*result, (ACM_MemberIntro&)*that_node);
      break;
    case JPT_BaseIntro:
      result = newBaseIntro();
      copyBaseIntro ((ACM_BaseIntro&)*result, (ACM_BaseIntro&)*that_node);
      break;
    case JPT_ClassPlan:
      result = newClassPlan();
      copyClassPlan ((ACM_ClassPlan&)*result, (ACM_ClassPlan&)*that_node);
      break;
    case JPT_CodeAdvice:
      result = newCodeAdvice();
      copyCodeAdvice ((ACM_CodeAdvice&)*result, (ACM_CodeAdvice&)*that_node);
      break;
    case JPT_CodePlan:
      result = newCodePlan();
      copyCodePlan ((ACM_CodePlan&)*result, (ACM_CodePlan&)*that_node);
      break;
    case JPT_Type:
      result = newType();
      copyType ((ACM_Type&)*result, (ACM_Type&)*that_node);
      break;
    case JPT_Arg:
      result = newArg();
      copyArg ((ACM_Arg&)*result, (ACM_Arg&)*that_node);
      break;
    case JPT_Namespace:
      result = newNamespace();
      copyNamespace ((ACM_Namespace&)*result, (ACM_Namespace&)*that_node);
      break;
    case JPT_Function:
      result = newFunction();
      copyFunction ((ACM_Function&)*result, (ACM_Function&)*that_node);
      break;
    case JPT_Variable:
      result = newVariable();
      copyVariable ((ACM_Variable&)*result, (ACM_Variable&)*that_node);
      break;
    case JPT_Class:
      result = newClass();
      copyClass ((ACM_Class&)*result, (ACM_Class&)*that_node);
      break;
    case JPT_ClassSlice:
      result = newClassSlice();
      copyClassSlice ((ACM_ClassSlice&)*result, (ACM_ClassSlice&)*that_node);
      break;
    case JPT_Get:
      result = newGet();
      copyGet ((ACM_Get&)*result, (ACM_Get&)*that_node);
      break;
    case JPT_Set:
      result = newSet();
      copySet ((ACM_Set&)*result, (ACM_Set&)*that_node);
      break;
    case JPT_Call:
      result = newCall();
      copyCall ((ACM_Call&)*result, (ACM_Call&)*that_node);
      break;
    case JPT_Builtin:
      result = newBuiltin();
      copyBuiltin ((ACM_Builtin&)*result, (ACM_Builtin&)*that_node);
      break;
    case JPT_Ref:
      result = newRef();
      copyRef ((ACM_Ref&)*result, (ACM_Ref&)*that_node);
      break;
    case JPT_GetRef:
      result = newGetRef();
      copyGetRef ((ACM_GetRef&)*result, (ACM_GetRef&)*that_node);
      break;
    case JPT_SetRef:
      result = newSetRef();
      copySetRef ((ACM_SetRef&)*result, (ACM_SetRef&)*that_node);
      break;
    case JPT_CallRef:
      result = newCallRef();
      copyCallRef ((ACM_CallRef&)*result, (ACM_CallRef&)*that_node);
      break;
    case JPT_Execution:
      result = newExecution();
      copyExecution ((ACM_Execution&)*result, (ACM_Execution&)*that_node);
      break;
    case JPT_Construction:
      result = newConstruction();
      copyConstruction ((ACM_Construction&)*result, (ACM_Construction&)*that_node);
      break;
    case JPT_Destruction:
      result = newDestruction();
      copyDestruction ((ACM_Destruction&)*result, (ACM_Destruction&)*that_node);
      break;
    case JPT_Pointcut:
      result = newPointcut();
      copyPointcut ((ACM_Pointcut&)*result, (ACM_Pointcut&)*that_node);
      break;
    case JPT_Attribute:
      result = newAttribute();
      copyAttribute ((ACM_Attribute&)*result, (ACM_Attribute&)*that_node);
      break;
    case JPT_AdviceCode:
      result = newAdviceCode();
      copyAdviceCode ((ACM_AdviceCode&)*result, (ACM_AdviceCode&)*that_node);
      break;
    case JPT_Introduction:
      result = newIntroduction();
      copyIntroduction ((ACM_Introduction&)*result, (ACM_Introduction&)*that_node);
      break;
    case JPT_Order:
      result = newOrder();
      copyOrder ((ACM_Order&)*result, (ACM_Order&)*that_node);
      break;
    case JPT_Aspect:
      result = newAspect();
      copyAspect ((ACM_Aspect&)*result, (ACM_Aspect&)*that_node);
      break;
    case JPT_Speculation:
      result = newSpeculation();
      copySpeculation ((ACM_Speculation&)*result, (ACM_Speculation&)*that_node);
      break;
    case JPT_Model:
      result = newModel();
      copyModel ((ACM_Model&)*result, (ACM_Model&)*that_node);
      break;
    default:
      result = 0;
  }
  if (result) that_node->set_partner (result);
  return result;
}

void ProjectModel::merge_node(ACM_Node *prj_node, ACM_Node *tu_node) {
  switch (prj_node->type_val ()) {
    case JPT_TUnit: mergeTUnit ((ACM_TUnit&)*prj_node, (ACM_TUnit&)*tu_node); break;
    case JPT_Header: mergeHeader ((ACM_Header&)*prj_node, (ACM_Header&)*tu_node); break;
    case JPT_Source: mergeSource ((ACM_Source&)*prj_node, (ACM_Source&)*tu_node); break;
    case JPT_MemberIntro: mergeMemberIntro ((ACM_MemberIntro&)*prj_node, (ACM_MemberIntro&)*tu_node); break;
    case JPT_BaseIntro: mergeBaseIntro ((ACM_BaseIntro&)*prj_node, (ACM_BaseIntro&)*tu_node); break;
    case JPT_ClassPlan: mergeClassPlan ((ACM_ClassPlan&)*prj_node, (ACM_ClassPlan&)*tu_node); break;
    case JPT_CodeAdvice: mergeCodeAdvice ((ACM_CodeAdvice&)*prj_node, (ACM_CodeAdvice&)*tu_node); break;
    case JPT_CodePlan: mergeCodePlan ((ACM_CodePlan&)*prj_node, (ACM_CodePlan&)*tu_node); break;
    case JPT_Type: mergeType ((ACM_Type&)*prj_node, (ACM_Type&)*tu_node); break;
    case JPT_Arg: mergeArg ((ACM_Arg&)*prj_node, (ACM_Arg&)*tu_node); break;
    case JPT_Namespace: mergeNamespace ((ACM_Namespace&)*prj_node, (ACM_Namespace&)*tu_node); break;
    case JPT_Function: mergeFunction ((ACM_Function&)*prj_node, (ACM_Function&)*tu_node); break;
    case JPT_Variable: mergeVariable ((ACM_Variable&)*prj_node, (ACM_Variable&)*tu_node); break;
    case JPT_Class: mergeClass ((ACM_Class&)*prj_node, (ACM_Class&)*tu_node); break;
    case JPT_ClassSlice: mergeClassSlice ((ACM_ClassSlice&)*prj_node, (ACM_ClassSlice&)*tu_node); break;
    case JPT_Get: mergeGet ((ACM_Get&)*prj_node, (ACM_Get&)*tu_node); break;
    case JPT_Set: mergeSet ((ACM_Set&)*prj_node, (ACM_Set&)*tu_node); break;
    case JPT_Call: mergeCall ((ACM_Call&)*prj_node, (ACM_Call&)*tu_node); break;
    case JPT_Builtin: mergeBuiltin ((ACM_Builtin&)*prj_node, (ACM_Builtin&)*tu_node); break;
    case JPT_Ref: mergeRef ((ACM_Ref&)*prj_node, (ACM_Ref&)*tu_node); break;
    case JPT_GetRef: mergeGetRef ((ACM_GetRef&)*prj_node, (ACM_GetRef&)*tu_node); break;
    case JPT_SetRef: mergeSetRef ((ACM_SetRef&)*prj_node, (ACM_SetRef&)*tu_node); break;
    case JPT_CallRef: mergeCallRef ((ACM_CallRef&)*prj_node, (ACM_CallRef&)*tu_node); break;
    case JPT_Execution: mergeExecution ((ACM_Execution&)*prj_node, (ACM_Execution&)*tu_node); break;
    case JPT_Construction: mergeConstruction ((ACM_Construction&)*prj_node, (ACM_Construction&)*tu_node); break;
    case JPT_Destruction: mergeDestruction ((ACM_Destruction&)*prj_node, (ACM_Destruction&)*tu_node); break;
    case JPT_Pointcut: mergePointcut ((ACM_Pointcut&)*prj_node, (ACM_Pointcut&)*tu_node); break;
    case JPT_Attribute: mergeAttribute ((ACM_Attribute&)*prj_node, (ACM_Attribute&)*tu_node); break;
    case JPT_AdviceCode: mergeAdviceCode ((ACM_AdviceCode&)*prj_node, (ACM_AdviceCode&)*tu_node); break;
    case JPT_Introduction: mergeIntroduction ((ACM_Introduction&)*prj_node, (ACM_Introduction&)*tu_node); break;
    case JPT_Order: mergeOrder ((ACM_Order&)*prj_node, (ACM_Order&)*tu_node); break;
    case JPT_Aspect: mergeAspect ((ACM_Aspect&)*prj_node, (ACM_Aspect&)*tu_node); break;
    case JPT_Speculation: mergeSpeculation ((ACM_Speculation&)*prj_node, (ACM_Speculation&)*tu_node); break;
    case JPT_Model: mergeModel ((ACM_Model&)*prj_node, (ACM_Model&)*tu_node); break;
    default:
      break;
  }
  tu_node->set_partner (prj_node);
}

bool ProjectModel::source_cleanup(ACM_Node &node) {
  switch (node.type_val ()) {
    case JPT_TUnit: return sourceCleanupTUnit ((ACM_TUnit&)node);
    case JPT_Header: return sourceCleanupHeader ((ACM_Header&)node);
    case JPT_Source: return sourceCleanupSource ((ACM_Source&)node);
    case JPT_MemberIntro: return sourceCleanupMemberIntro ((ACM_MemberIntro&)node);
    case JPT_BaseIntro: return sourceCleanupBaseIntro ((ACM_BaseIntro&)node);
    case JPT_ClassPlan: return sourceCleanupClassPlan ((ACM_ClassPlan&)node);
    case JPT_CodeAdvice: return sourceCleanupCodeAdvice ((ACM_CodeAdvice&)node);
    case JPT_CodePlan: return sourceCleanupCodePlan ((ACM_CodePlan&)node);
    case JPT_Type: return sourceCleanupType ((ACM_Type&)node);
    case JPT_Arg: return sourceCleanupArg ((ACM_Arg&)node);
    case JPT_Namespace: return sourceCleanupNamespace ((ACM_Namespace&)node);
    case JPT_Function: return sourceCleanupFunction ((ACM_Function&)node);
    case JPT_Variable: return sourceCleanupVariable ((ACM_Variable&)node);
    case JPT_Class: return sourceCleanupClass ((ACM_Class&)node);
    case JPT_ClassSlice: return sourceCleanupClassSlice ((ACM_ClassSlice&)node);
    case JPT_Get: return sourceCleanupGet ((ACM_Get&)node);
    case JPT_Set: return sourceCleanupSet ((ACM_Set&)node);
    case JPT_Call: return sourceCleanupCall ((ACM_Call&)node);
    case JPT_Builtin: return sourceCleanupBuiltin ((ACM_Builtin&)node);
    case JPT_Ref: return sourceCleanupRef ((ACM_Ref&)node);
    case JPT_GetRef: return sourceCleanupGetRef ((ACM_GetRef&)node);
    case JPT_SetRef: return sourceCleanupSetRef ((ACM_SetRef&)node);
    case JPT_CallRef: return sourceCleanupCallRef ((ACM_CallRef&)node);
    case JPT_Execution: return sourceCleanupExecution ((ACM_Execution&)node);
    case JPT_Construction: return sourceCleanupConstruction ((ACM_Construction&)node);
    case JPT_Destruction: return sourceCleanupDestruction ((ACM_Destruction&)node);
    case JPT_Pointcut: return sourceCleanupPointcut ((ACM_Pointcut&)node);
    case JPT_Attribute: return sourceCleanupAttribute ((ACM_Attribute&)node);
    case JPT_AdviceCode: return sourceCleanupAdviceCode ((ACM_AdviceCode&)node);
    case JPT_Introduction: return sourceCleanupIntroduction ((ACM_Introduction&)node);
    case JPT_Order: return sourceCleanupOrder ((ACM_Order&)node);
    case JPT_Aspect: return sourceCleanupAspect ((ACM_Aspect&)node);
    case JPT_Speculation: return sourceCleanupSpeculation ((ACM_Speculation&)node);
    case JPT_Model: return sourceCleanupModel ((ACM_Model&)node);
    default:
      return false;
  }
}

// fix all node references; this is the second phase of merging a tunit
// model into a project model. Reference still address the tunit model
// nodes. Now they have to be replaced with the corresponding project
// model node references
void ProjectModel::fix_refs(ACM_Node *me) {
  switch (me->type_val ()) {
    case JPT_TUnit: fix_refs_TUnit ((ACM_TUnit&)*me); break;
    case JPT_Header: fix_refs_Header ((ACM_Header&)*me); break;
    case JPT_Source: fix_refs_Source ((ACM_Source&)*me); break;
    case JPT_MemberIntro: fix_refs_MemberIntro ((ACM_MemberIntro&)*me); break;
    case JPT_BaseIntro: fix_refs_BaseIntro ((ACM_BaseIntro&)*me); break;
    case JPT_ClassPlan: fix_refs_ClassPlan ((ACM_ClassPlan&)*me); break;
    case JPT_CodeAdvice: fix_refs_CodeAdvice ((ACM_CodeAdvice&)*me); break;
    case JPT_CodePlan: fix_refs_CodePlan ((ACM_CodePlan&)*me); break;
    case JPT_Type: fix_refs_Type ((ACM_Type&)*me); break;
    case JPT_Arg: fix_refs_Arg ((ACM_Arg&)*me); break;
    case JPT_Namespace: fix_refs_Namespace ((ACM_Namespace&)*me); break;
    case JPT_Function: fix_refs_Function ((ACM_Function&)*me); break;
    case JPT_Variable: fix_refs_Variable ((ACM_Variable&)*me); break;
    case JPT_Class: fix_refs_Class ((ACM_Class&)*me); break;
    case JPT_ClassSlice: fix_refs_ClassSlice ((ACM_ClassSlice&)*me); break;
    case JPT_Get: fix_refs_Get ((ACM_Get&)*me); break;
    case JPT_Set: fix_refs_Set ((ACM_Set&)*me); break;
    case JPT_Call: fix_refs_Call ((ACM_Call&)*me); break;
    case JPT_Builtin: fix_refs_Builtin ((ACM_Builtin&)*me); break;
    case JPT_Ref: fix_refs_Ref ((ACM_Ref&)*me); break;
    case JPT_GetRef: fix_refs_GetRef ((ACM_GetRef&)*me); break;
    case JPT_SetRef: fix_refs_SetRef ((ACM_SetRef&)*me); break;
    case JPT_CallRef: fix_refs_CallRef ((ACM_CallRef&)*me); break;
    case JPT_Execution: fix_refs_Execution ((ACM_Execution&)*me); break;
    case JPT_Construction: fix_refs_Construction ((ACM_Construction&)*me); break;
    case JPT_Destruction: fix_refs_Destruction ((ACM_Destruction&)*me); break;
    case JPT_Pointcut: fix_refs_Pointcut ((ACM_Pointcut&)*me); break;
    case JPT_Attribute: fix_refs_Attribute ((ACM_Attribute&)*me); break;
    case JPT_AdviceCode: fix_refs_AdviceCode ((ACM_AdviceCode&)*me); break;
    case JPT_Introduction: fix_refs_Introduction ((ACM_Introduction&)*me); break;
    case JPT_Order: fix_refs_Order ((ACM_Order&)*me); break;
    case JPT_Aspect: fix_refs_Aspect ((ACM_Aspect&)*me); break;
    case JPT_Speculation: fix_refs_Speculation ((ACM_Speculation&)*me); break;
    case JPT_Model: fix_refs_Model ((ACM_Model&)*me); break;
    default:
      break;
  }
  me->set_partner (0);
}


// copy methods that copy elements from other models into this one
// NOTE: the elements are only copied and registerd, but no link is set!
void ProjectModel::copyNode(ACM_Node &me, ACM_Node &that) {
}
void ProjectModel::copyFile(ACM_File &me, ACM_File &that) {
  copyNode(me, that);
  me.set_filename(that.get_filename());
  me.set_len(that.get_len());
  me.set_time(that.get_time());
}
void ProjectModel::copyTUnit(ACM_TUnit &me, ACM_TUnit &that) {
  copyFile(me, that);
  me.set_dirty_flag(that.get_dirty_flag());
}
void ProjectModel::copyHeader(ACM_Header &me, ACM_Header &that) {
  copyFile(me, that);
  
  // reference attribute 'in' copied, address(es) must be adapted later
  for (ACM_Container<ACM_TUnit, false> ::iterator i = that.get_in ().begin (); i != that.get_in ().end (); ++i)
    me.get_in ().insert ((ACM_TUnit*)*i);
}
void ProjectModel::copySource(ACM_Source &me, ACM_Source &that) {
  copyNode(me, that);
  me.set_kind(that.get_kind());
  
  // reference attribute 'file' copied, address(es) must be adapted later
  me.set_file(that.get_file ());
  me.set_line(that.get_line());
  me.set_len(that.get_len());
}
void ProjectModel::copyMemberIntro(ACM_MemberIntro &me, ACM_MemberIntro &that) {
  copyNode(me, that);
  
  // reference attribute 'intro' copied, address(es) must be adapted later
  me.set_intro(that.get_intro ());
  
  // reference attribute 'members' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Name, false> ::iterator i = that.get_members ().begin (); i != that.get_members ().end (); ++i)
    me.get_members ().insert ((ACM_Name*)*i);
}
void ProjectModel::copyBaseIntro(ACM_BaseIntro &me, ACM_BaseIntro &that) {
  copyNode(me, that);
  
  // reference attribute 'intro' copied, address(es) must be adapted later
  me.set_intro(that.get_intro ());
  
  // reference attribute 'bases' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Class, false> ::iterator i = that.get_bases ().begin (); i != that.get_bases ().end (); ++i)
    me.get_bases ().insert ((ACM_Class*)*i);
}
void ProjectModel::copyClassPlan(ACM_ClassPlan &me, ACM_ClassPlan &that) {
  copyNode(me, that);
  for (ACM_Container<ACM_BaseIntro, true> ::iterator i = that.get_base_intros ().begin (); i != that.get_base_intros ().end (); ++i)
    me.get_base_intros ().insert ((ACM_BaseIntro*)copyNode(*i));
  for (ACM_Container<ACM_MemberIntro, true> ::iterator i = that.get_member_intros ().begin (); i != that.get_member_intros ().end (); ++i)
    me.get_member_intros ().insert ((ACM_MemberIntro*)copyNode(*i));
}
void ProjectModel::copyCodeAdvice(ACM_CodeAdvice &me, ACM_CodeAdvice &that) {
  copyNode(me, that);
  
  // reference attribute 'advice' copied, address(es) must be adapted later
  me.set_advice(that.get_advice ());
  me.set_conditional(that.get_conditional());
}
void ProjectModel::copyCodePlan(ACM_CodePlan &me, ACM_CodePlan &that) {
  copyNode(me, that);
  for (ACM_Container<ACM_CodeAdvice, true> ::iterator i = that.get_before ().begin (); i != that.get_before ().end (); ++i)
    me.get_before ().insert ((ACM_CodeAdvice*)copyNode(*i));
  if (that.has_around()) me.set_around((ACM_CodeAdvice*)copyNode(that.get_around ()));
  for (ACM_Container<ACM_CodeAdvice, true> ::iterator i = that.get_after ().begin (); i != that.get_after ().end (); ++i)
    me.get_after ().insert ((ACM_CodeAdvice*)copyNode(*i));
  if (that.has_next_level()) me.set_next_level((ACM_CodePlan*)copyNode(that.get_next_level ()));
}
void ProjectModel::copyAny(ACM_Any &me, ACM_Any &that) {
  copyNode(me, that);
  if (that.has_jpid()) {
    // unsigned old_jpid = that.get_jpid (); 
    unsigned new_jpid = alloc_jpid ();
    me.set_jpid (new_jpid);
    that.set_jpid (new_jpid);
  }
  for (ACM_Container<ACM_Source, true> ::iterator i = that.get_source ().begin (); i != that.get_source ().end (); ++i)
    me.get_source ().insert ((ACM_Source*)copyNode(*i));
  
  // reference attribute 'attributes' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Attribute, false> ::iterator i = that.get_attributes ().begin (); i != that.get_attributes ().end (); ++i)
    me.get_attributes ().insert ((ACM_Attribute*)*i);
}
void ProjectModel::copyType(ACM_Type &me, ACM_Type &that) {
  copyAny(me, that);
  me.set_signature(that.get_signature());
  // TODO: copy complex object match_sig here
}
void ProjectModel::copyArg(ACM_Arg &me, ACM_Arg &that) {
  copyAny(me, that);
  me.set_type(that.get_type());
  me.set_name(that.get_name());
}
void ProjectModel::copyName(ACM_Name &me, ACM_Name &that) {
  copyAny(me, that);
  for (ACM_Container<ACM_Any, true> ::iterator i = that.get_children ().begin (); i != that.get_children ().end (); ++i)
    me.get_children ().insert ((ACM_Any*)copyNode(*i));
  me.set_name(that.get_name());
  // TODO: copy complex object match_sig here
  // TODO: copy complex object name_map here
  me.set_builtin(that.get_builtin());
  
  // reference attribute 'tunits' copied, address(es) must be adapted later
  for (ACM_Container<ACM_TUnit, false> ::iterator i = that.get_tunits ().begin (); i != that.get_tunits ().end (); ++i)
    me.get_tunits ().insert ((ACM_TUnit*)*i);
}
void ProjectModel::copyNamespace(ACM_Namespace &me, ACM_Namespace &that) {
  copyName(me, that);
}
void ProjectModel::copyFunction(ACM_Function &me, ACM_Function &that) {
  copyName(me, that);
  if (that.has_result_type()) me.set_result_type((ACM_Type*)copyNode(that.get_result_type ()));
  for (ACM_Container<ACM_Type, true> ::iterator i = that.get_arg_types ().begin (); i != that.get_arg_types ().end (); ++i)
    me.get_arg_types ().insert ((ACM_Type*)copyNode(*i));
  me.set_kind(that.get_kind());
  me.set_variadic_args(that.get_variadic_args());
  me.set_cv_qualifiers(that.get_cv_qualifiers());
  
  // reference attribute 'static_in' copied, address(es) must be adapted later
  if (that.has_static_in()) me.set_static_in(that.get_static_in ());
  
  // reference attribute 'calls' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Call, false> ::iterator i = that.get_calls ().begin (); i != that.get_calls ().end (); ++i)
    me.get_calls ().insert ((ACM_Call*)*i);
}
void ProjectModel::copyVariable(ACM_Variable &me, ACM_Variable &that) {
  copyName(me, that);
  me.set_type((ACM_Type*)copyNode(that.get_type ()));
  me.set_kind(that.get_kind());
}
void ProjectModel::copyClass(ACM_Class &me, ACM_Class &that) {
  copyName(me, that);
  
  // reference attribute 'bases' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Class, false> ::iterator i = that.get_bases ().begin (); i != that.get_bases ().end (); ++i)
    me.get_bases ().insert ((ACM_Class*)*i);
  
  // reference attribute 'derived' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Class, false> ::iterator i = that.get_derived ().begin (); i != that.get_derived ().end (); ++i)
    me.get_derived ().insert ((ACM_Class*)*i);
  me.set_intro_target(that.get_intro_target());
  if (that.has_plan()) me.set_plan((ACM_ClassPlan*)copyNode(that.get_plan ()));
}
void ProjectModel::copyClassSlice(ACM_ClassSlice &me, ACM_ClassSlice &that) {
  copyName(me, that);
  me.set_is_struct(that.get_is_struct());
}
void ProjectModel::copyCode(ACM_Code &me, ACM_Code &that) {
  copyAny(me, that);
  if (that.has_plan()) me.set_plan((ACM_CodePlan*)copyNode(that.get_plan ()));
}
void ProjectModel::copyAccess(ACM_Access &me, ACM_Access &that) {
  copyCode(me, that);
  me.set_lid(that.get_lid());
  if (that.has_target_object_lid()) me.set_target_object_lid(that.get_target_object_lid());
  if (that.has_cfg_block_lid()) me.set_cfg_block_lid(that.get_cfg_block_lid());
  
  // reference attribute 'target_class' copied, address(es) must be adapted later
  if (that.has_target_class()) me.set_target_class(that.get_target_class ());
}
void ProjectModel::copyGet(ACM_Get &me, ACM_Get &that) {
  copyAccess(me, that);
  
  // reference attribute 'variable' copied, address(es) must be adapted later
  if (that.has_variable()) me.set_variable(that.get_variable ());
}
void ProjectModel::copySet(ACM_Set &me, ACM_Set &that) {
  copyAccess(me, that);
  
  // reference attribute 'variable' copied, address(es) must be adapted later
  if (that.has_variable()) me.set_variable(that.get_variable ());
}
void ProjectModel::copyCall(ACM_Call &me, ACM_Call &that) {
  copyAccess(me, that);
  
  // reference attribute 'target' copied, address(es) must be adapted later
  me.set_target(that.get_target ());
  for (ACM_Container<ACM_Type, true> ::iterator i = that.get_variadic_arg_types ().begin (); i != that.get_variadic_arg_types ().end (); ++i)
    me.get_variadic_arg_types ().insert ((ACM_Type*)copyNode(*i));
  if (that.has_default_args()) me.set_default_args(that.get_default_args());
}
void ProjectModel::copyBuiltin(ACM_Builtin &me, ACM_Builtin &that) {
  copyAccess(me, that);
  
  // reference attribute 'target' copied, address(es) must be adapted later
  me.set_target(that.get_target ());
  for (ACM_Container<ACM_Access, true> ::iterator i = that.get_implicit_access ().begin (); i != that.get_implicit_access ().end (); ++i)
    me.get_implicit_access ().insert ((ACM_Access*)copyNode(*i));
}
void ProjectModel::copyRef(ACM_Ref &me, ACM_Ref &that) {
  copyAccess(me, that);
  
  // reference attribute 'variable' copied, address(es) must be adapted later
  me.set_variable(that.get_variable ());
  me.set_type((ACM_Type*)copyNode(that.get_type ()));
}
void ProjectModel::copyGetRef(ACM_GetRef &me, ACM_GetRef &that) {
  copyAccess(me, that);
  me.set_type((ACM_Type*)copyNode(that.get_type ()));
}
void ProjectModel::copySetRef(ACM_SetRef &me, ACM_SetRef &that) {
  copyAccess(me, that);
  me.set_type((ACM_Type*)copyNode(that.get_type ()));
}
void ProjectModel::copyCallRef(ACM_CallRef &me, ACM_CallRef &that) {
  copyAccess(me, that);
  me.set_type((ACM_Type*)copyNode(that.get_type ()));
}
void ProjectModel::copyExecution(ACM_Execution &me, ACM_Execution &that) {
  copyCode(me, that);
}
void ProjectModel::copyConstruction(ACM_Construction &me, ACM_Construction &that) {
  copyCode(me, that);
}
void ProjectModel::copyDestruction(ACM_Destruction &me, ACM_Destruction &that) {
  copyCode(me, that);
}
void ProjectModel::copyPointcut(ACM_Pointcut &me, ACM_Pointcut &that) {
  copyName(me, that);
  me.set_expr(that.get_expr());
  for (ACM_Container<ACM_Arg, true> ::iterator i = that.get_args ().begin (); i != that.get_args ().end (); ++i)
    me.get_args ().insert ((ACM_Arg*)copyNode(*i));
  me.set_kind(that.get_kind());
  // TODO: copy complex object parsed_expr here
}
void ProjectModel::copyAttribute(ACM_Attribute &me, ACM_Attribute &that) {
  copyName(me, that);
  for (ACM_Container<ACM_Arg, true> ::iterator i = that.get_args ().begin (); i != that.get_args ().end (); ++i)
    me.get_args ().insert ((ACM_Arg*)copyNode(*i));
}
void ProjectModel::copyAdvice(ACM_Advice &me, ACM_Advice &that) {
  copyAny(me, that);
  me.set_pointcut((ACM_Pointcut*)copyNode(that.get_pointcut ()));
  me.set_lid(that.get_lid());
}
void ProjectModel::copyAdviceCode(ACM_AdviceCode &me, ACM_AdviceCode &that) {
  copyAdvice(me, that);
  me.set_kind(that.get_kind());
  me.set_context(that.get_context());
}
void ProjectModel::copyIntroduction(ACM_Introduction &me, ACM_Introduction &that) {
  copyAdvice(me, that);
  
  // reference attribute 'named_slice' copied, address(es) must be adapted later
  if (that.has_named_slice()) me.set_named_slice(that.get_named_slice ());
  if (that.has_anon_slice()) me.set_anon_slice((ACM_ClassSlice*)copyNode(that.get_anon_slice ()));
}
void ProjectModel::copyOrder(ACM_Order &me, ACM_Order &that) {
  copyAdvice(me, that);
  for (ACM_Container<ACM_Pointcut, true> ::iterator i = that.get_aspect_exprs ().begin (); i != that.get_aspect_exprs ().end (); ++i)
    me.get_aspect_exprs ().insert ((ACM_Pointcut*)copyNode(*i));
}
void ProjectModel::copyAspect(ACM_Aspect &me, ACM_Aspect &that) {
  copyClass(me, that);
  for (ACM_Container<ACM_Introduction, true> ::iterator i = that.get_intros ().begin (); i != that.get_intros ().end (); ++i)
    me.get_intros ().insert ((ACM_Introduction*)copyNode(*i));
  for (ACM_Container<ACM_AdviceCode, true> ::iterator i = that.get_advices ().begin (); i != that.get_advices ().end (); ++i)
    me.get_advices ().insert ((ACM_AdviceCode*)copyNode(*i));
  for (ACM_Container<ACM_Order, true> ::iterator i = that.get_orders ().begin (); i != that.get_orders ().end (); ++i)
    me.get_orders ().insert ((ACM_Order*)copyNode(*i));
}
void ProjectModel::copySpeculation(ACM_Speculation &me, ACM_Speculation &that) {
  copyNode(me, that);
  
  // reference attribute 'advice' copied, address(es) must be adapted later
  me.set_advice(that.get_advice ());
  
  // reference attribute 'tunit' copied, address(es) must be adapted later
  me.set_tunit(that.get_tunit ());
  
  // reference attribute 'true_jps' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Any, false> ::iterator i = that.get_true_jps ().begin (); i != that.get_true_jps ().end (); ++i)
    me.get_true_jps ().insert ((ACM_Any*)*i);
  
  // reference attribute 'false_jps' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Any, false> ::iterator i = that.get_false_jps ().begin (); i != that.get_false_jps ().end (); ++i)
    me.get_false_jps ().insert ((ACM_Any*)*i);
  
  // reference attribute 'conditional_jps' copied, address(es) must be adapted later
  for (ACM_Container<ACM_Any, false> ::iterator i = that.get_conditional_jps ().begin (); i != that.get_conditional_jps ().end (); ++i)
    me.get_conditional_jps ().insert ((ACM_Any*)*i);
}
void ProjectModel::copyModel(ACM_Model &me, ACM_Model &that) {
  copyNode(me, that);
  for (ACM_Container<ACM_File, true> ::iterator i = that.get_files ().begin (); i != that.get_files ().end (); ++i)
    me.get_files ().insert ((ACM_File*)copyNode(*i));
  me.set_root((ACM_Namespace*)copyNode(that.get_root ()));
  for (ACM_Container<ACM_Speculation, true> ::iterator i = that.get_speculations ().begin (); i != that.get_speculations ().end (); ++i)
    me.get_speculations ().insert ((ACM_Speculation*)copyNode(*i));
  me.set_version(that.get_version());
}

// remove all out-dated model elements
bool ProjectModel::sourceCleanupNode(ACM_Node &me) {
  return false;
}
bool ProjectModel::sourceCleanupFile(ACM_File &me) {
  if (sourceCleanupNode(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupTUnit(ACM_TUnit &me) {
  if (sourceCleanupFile(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupHeader(ACM_Header &me) {
  if (sourceCleanupFile(me))
    return true;
  sourceCleanupContainer(me.get_in ());
  return false;
}
bool ProjectModel::sourceCleanupSource(ACM_Source &me) {
  if (sourceCleanupNode(me))
    return true;
  int merge_count_file = me.get_file ()->get_merge_count ();
  me.get_file ()->set_merge_count (_merge_id);
  if ((merge_count_file == _merge_id) ? me.get_file ()->is_deleted () : sourceCleanupFile(*me.get_file ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupMemberIntro(ACM_MemberIntro &me) {
  if (sourceCleanupNode(me))
    return true;
  int merge_count_intro = me.get_intro ()->get_merge_count ();
  me.get_intro ()->set_merge_count (_merge_id);
  if ((merge_count_intro == _merge_id) ? me.get_intro ()->is_deleted () : sourceCleanupIntroduction(*me.get_intro ()))
    return true;
  sourceCleanupContainer(me.get_members ());
  return false;
}
bool ProjectModel::sourceCleanupBaseIntro(ACM_BaseIntro &me) {
  if (sourceCleanupNode(me))
    return true;
  int merge_count_intro = me.get_intro ()->get_merge_count ();
  me.get_intro ()->set_merge_count (_merge_id);
  if ((merge_count_intro == _merge_id) ? me.get_intro ()->is_deleted () : sourceCleanupIntroduction(*me.get_intro ()))
    return true;
  sourceCleanupContainer(me.get_bases ());
  return false;
}
bool ProjectModel::sourceCleanupClassPlan(ACM_ClassPlan &me) {
  if (sourceCleanupNode(me))
    return true;
  sourceCleanupContainer(me.get_base_intros ());
  sourceCleanupContainer(me.get_member_intros ());
  return false;
}
bool ProjectModel::sourceCleanupCodeAdvice(ACM_CodeAdvice &me) {
  if (sourceCleanupNode(me))
    return true;
  int merge_count_advice = me.get_advice ()->get_merge_count ();
  me.get_advice ()->set_merge_count (_merge_id);
  if ((merge_count_advice == _merge_id) ? me.get_advice ()->is_deleted () : sourceCleanupAdviceCode(*me.get_advice ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupCodePlan(ACM_CodePlan &me) {
  if (sourceCleanupNode(me))
    return true;
  sourceCleanupContainer(me.get_before ());
  if (me.has_around ()) {
  int merge_count_around = me.get_around ()->get_merge_count ();
  me.get_around ()->set_merge_count (_merge_id);
  if ((merge_count_around == _merge_id) ? me.get_around ()->is_deleted () : sourceCleanupCodeAdvice(*me.get_around ()))
    return true;
  }
  sourceCleanupContainer(me.get_after ());
  if (me.has_next_level ()) {
  int merge_count_next_level = me.get_next_level ()->get_merge_count ();
  me.get_next_level ()->set_merge_count (_merge_id);
  if ((merge_count_next_level == _merge_id) ? me.get_next_level ()->is_deleted () : sourceCleanupCodePlan(*me.get_next_level ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupAny(ACM_Any &me) {
  if (sourceCleanupNode(me))
    return true;
  if (purgeSources (me)) {
    unlink_node(&me);
    return true;
  }
  sourceCleanupContainer(me.get_attributes ());
  return false;
}
bool ProjectModel::sourceCleanupType(ACM_Type &me) {
  if (sourceCleanupAny(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupArg(ACM_Arg &me) {
  if (sourceCleanupAny(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupName(ACM_Name &me) {
  if (sourceCleanupAny(me))
    return true;
  sourceCleanupContainer(me.get_children ());
  sourceCleanupContainer(me.get_tunits ());
  return false;
}
bool ProjectModel::sourceCleanupNamespace(ACM_Namespace &me) {
  if (sourceCleanupName(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupFunction(ACM_Function &me) {
  if (sourceCleanupName(me))
    return true;
  if (me.has_result_type ()) {
  int merge_count_result_type = me.get_result_type ()->get_merge_count ();
  me.get_result_type ()->set_merge_count (_merge_id);
  if ((merge_count_result_type == _merge_id) ? me.get_result_type ()->is_deleted () : sourceCleanupType(*me.get_result_type ()))
    return true;
  }
  sourceCleanupContainer(me.get_arg_types ());
  if (me.has_static_in ()) {
  int merge_count_static_in = me.get_static_in ()->get_merge_count ();
  me.get_static_in ()->set_merge_count (_merge_id);
  if ((merge_count_static_in == _merge_id) ? me.get_static_in ()->is_deleted () : sourceCleanupTUnit(*me.get_static_in ()))
    return true;
  }
  sourceCleanupContainer(me.get_calls ());
  return false;
}
bool ProjectModel::sourceCleanupVariable(ACM_Variable &me) {
  if (sourceCleanupName(me))
    return true;
  int merge_count_type = me.get_type ()->get_merge_count ();
  me.get_type ()->set_merge_count (_merge_id);
  if ((merge_count_type == _merge_id) ? me.get_type ()->is_deleted () : sourceCleanupType(*me.get_type ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupClass(ACM_Class &me) {
  if (sourceCleanupName(me))
    return true;
  sourceCleanupContainer(me.get_bases ());
  sourceCleanupContainer(me.get_derived ());
  if (me.has_plan ()) {
  int merge_count_plan = me.get_plan ()->get_merge_count ();
  me.get_plan ()->set_merge_count (_merge_id);
  if ((merge_count_plan == _merge_id) ? me.get_plan ()->is_deleted () : sourceCleanupClassPlan(*me.get_plan ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupClassSlice(ACM_ClassSlice &me) {
  if (sourceCleanupName(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupCode(ACM_Code &me) {
  if (sourceCleanupAny(me))
    return true;
  if (me.has_plan ()) {
  int merge_count_plan = me.get_plan ()->get_merge_count ();
  me.get_plan ()->set_merge_count (_merge_id);
  if ((merge_count_plan == _merge_id) ? me.get_plan ()->is_deleted () : sourceCleanupCodePlan(*me.get_plan ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupAccess(ACM_Access &me) {
  if (sourceCleanupCode(me))
    return true;
  if (me.has_target_class ()) {
  int merge_count_target_class = me.get_target_class ()->get_merge_count ();
  me.get_target_class ()->set_merge_count (_merge_id);
  if ((merge_count_target_class == _merge_id) ? me.get_target_class ()->is_deleted () : sourceCleanupClass(*me.get_target_class ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupGet(ACM_Get &me) {
  if (sourceCleanupAccess(me))
    return true;
  if (me.has_variable ()) {
  int merge_count_variable = me.get_variable ()->get_merge_count ();
  me.get_variable ()->set_merge_count (_merge_id);
  if ((merge_count_variable == _merge_id) ? me.get_variable ()->is_deleted () : sourceCleanupVariable(*me.get_variable ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupSet(ACM_Set &me) {
  if (sourceCleanupAccess(me))
    return true;
  if (me.has_variable ()) {
  int merge_count_variable = me.get_variable ()->get_merge_count ();
  me.get_variable ()->set_merge_count (_merge_id);
  if ((merge_count_variable == _merge_id) ? me.get_variable ()->is_deleted () : sourceCleanupVariable(*me.get_variable ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupCall(ACM_Call &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_target = me.get_target ()->get_merge_count ();
  me.get_target ()->set_merge_count (_merge_id);
  if ((merge_count_target == _merge_id) ? me.get_target ()->is_deleted () : sourceCleanupFunction(*me.get_target ()))
    return true;
  sourceCleanupContainer(me.get_variadic_arg_types ());
  return false;
}
bool ProjectModel::sourceCleanupBuiltin(ACM_Builtin &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_target = me.get_target ()->get_merge_count ();
  me.get_target ()->set_merge_count (_merge_id);
  if ((merge_count_target == _merge_id) ? me.get_target ()->is_deleted () : sourceCleanupFunction(*me.get_target ()))
    return true;
  sourceCleanupContainer(me.get_implicit_access ());
  return false;
}
bool ProjectModel::sourceCleanupRef(ACM_Ref &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_variable = me.get_variable ()->get_merge_count ();
  me.get_variable ()->set_merge_count (_merge_id);
  if ((merge_count_variable == _merge_id) ? me.get_variable ()->is_deleted () : sourceCleanupVariable(*me.get_variable ()))
    return true;
  int merge_count_type = me.get_type ()->get_merge_count ();
  me.get_type ()->set_merge_count (_merge_id);
  if ((merge_count_type == _merge_id) ? me.get_type ()->is_deleted () : sourceCleanupType(*me.get_type ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupGetRef(ACM_GetRef &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_type = me.get_type ()->get_merge_count ();
  me.get_type ()->set_merge_count (_merge_id);
  if ((merge_count_type == _merge_id) ? me.get_type ()->is_deleted () : sourceCleanupType(*me.get_type ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupSetRef(ACM_SetRef &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_type = me.get_type ()->get_merge_count ();
  me.get_type ()->set_merge_count (_merge_id);
  if ((merge_count_type == _merge_id) ? me.get_type ()->is_deleted () : sourceCleanupType(*me.get_type ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupCallRef(ACM_CallRef &me) {
  if (sourceCleanupAccess(me))
    return true;
  int merge_count_type = me.get_type ()->get_merge_count ();
  me.get_type ()->set_merge_count (_merge_id);
  if ((merge_count_type == _merge_id) ? me.get_type ()->is_deleted () : sourceCleanupType(*me.get_type ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupExecution(ACM_Execution &me) {
  if (sourceCleanupCode(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupConstruction(ACM_Construction &me) {
  if (sourceCleanupCode(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupDestruction(ACM_Destruction &me) {
  if (sourceCleanupCode(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupPointcut(ACM_Pointcut &me) {
  if (sourceCleanupName(me))
    return true;
  sourceCleanupContainer(me.get_args ());
  return false;
}
bool ProjectModel::sourceCleanupAttribute(ACM_Attribute &me) {
  if (sourceCleanupName(me))
    return true;
  sourceCleanupContainer(me.get_args ());
  return false;
}
bool ProjectModel::sourceCleanupAdvice(ACM_Advice &me) {
  if (sourceCleanupAny(me))
    return true;
  int merge_count_pointcut = me.get_pointcut ()->get_merge_count ();
  me.get_pointcut ()->set_merge_count (_merge_id);
  if ((merge_count_pointcut == _merge_id) ? me.get_pointcut ()->is_deleted () : sourceCleanupPointcut(*me.get_pointcut ()))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupAdviceCode(ACM_AdviceCode &me) {
  if (sourceCleanupAdvice(me))
    return true;
  return false;
}
bool ProjectModel::sourceCleanupIntroduction(ACM_Introduction &me) {
  if (sourceCleanupAdvice(me))
    return true;
  if (me.has_named_slice ()) {
  int merge_count_named_slice = me.get_named_slice ()->get_merge_count ();
  me.get_named_slice ()->set_merge_count (_merge_id);
  if ((merge_count_named_slice == _merge_id) ? me.get_named_slice ()->is_deleted () : sourceCleanupClassSlice(*me.get_named_slice ()))
    return true;
  }
  if (me.has_anon_slice ()) {
  int merge_count_anon_slice = me.get_anon_slice ()->get_merge_count ();
  me.get_anon_slice ()->set_merge_count (_merge_id);
  if ((merge_count_anon_slice == _merge_id) ? me.get_anon_slice ()->is_deleted () : sourceCleanupClassSlice(*me.get_anon_slice ()))
    return true;
  }
  return false;
}
bool ProjectModel::sourceCleanupOrder(ACM_Order &me) {
  if (sourceCleanupAdvice(me))
    return true;
  sourceCleanupContainer(me.get_aspect_exprs ());
  return false;
}
bool ProjectModel::sourceCleanupAspect(ACM_Aspect &me) {
  if (sourceCleanupClass(me))
    return true;
  sourceCleanupContainer(me.get_intros ());
  sourceCleanupContainer(me.get_advices ());
  sourceCleanupContainer(me.get_orders ());
  return false;
}
bool ProjectModel::sourceCleanupSpeculation(ACM_Speculation &me) {
  if (sourceCleanupNode(me))
    return true;
  int merge_count_advice = me.get_advice ()->get_merge_count ();
  me.get_advice ()->set_merge_count (_merge_id);
  if ((merge_count_advice == _merge_id) ? me.get_advice ()->is_deleted () : sourceCleanupAdvice(*me.get_advice ()))
    return true;
  int merge_count_tunit = me.get_tunit ()->get_merge_count ();
  me.get_tunit ()->set_merge_count (_merge_id);
  if ((merge_count_tunit == _merge_id) ? me.get_tunit ()->is_deleted () : sourceCleanupTUnit(*me.get_tunit ()))
    return true;
  sourceCleanupContainer(me.get_true_jps ());
  sourceCleanupContainer(me.get_false_jps ());
  sourceCleanupContainer(me.get_conditional_jps ());
  return false;
}
bool ProjectModel::sourceCleanupModel(ACM_Model &me) {
  if (sourceCleanupNode(me))
    return true;
  sourceCleanupContainer(me.get_files ());
  int merge_count_root = me.get_root ()->get_merge_count ();
  me.get_root ()->set_merge_count (_merge_id);
  if ((merge_count_root == _merge_id) ? me.get_root ()->is_deleted () : sourceCleanupNamespace(*me.get_root ()))
    return true;
  sourceCleanupContainer(me.get_speculations ());
  return false;
}

// merge methods that are called for corresponding translation unit model
// and project model nodes. The functions call merge or copy functions on
// their aggregated sub-nodes.
void ProjectModel::mergeNode(ACM_Node &me, ACM_Node &that) {
}
void ProjectModel::mergeFile(ACM_File &me, ACM_File &that) {
  mergeNode(me, that);
  if (that.get_len () != me.get_len ()) {
    if (merge_attr (me, that, "len"))
      me.set_len (that.get_len ());
  }
  if (that.get_time () != me.get_time ()) {
    if (merge_attr (me, that, "time"))
      me.set_time (that.get_time ());
  }
}
void ProjectModel::mergeTUnit(ACM_TUnit &me, ACM_TUnit &that) {
  mergeFile(me, that);
  if (that.get_dirty_flag () != me.get_dirty_flag ()) {
    if (merge_attr (me, that, "dirty_flag"))
      me.set_dirty_flag (that.get_dirty_flag ());
  }
}
void ProjectModel::mergeHeader(ACM_Header &me, ACM_Header &that) {
  mergeFile(me, that);
  merge_container (that.get_in (), me.get_in (), true);
}
void ProjectModel::mergeSource(ACM_Source &me, ACM_Source &that) {
  mergeNode(me, that);
  // TODO: implement merging for reference member 'file' -> is that all?
  me.set_file(that.get_file ());
  if (that.get_len () != me.get_len ()) {
    if (merge_attr (me, that, "len"))
      me.set_len (that.get_len ());
  }
}
void ProjectModel::mergeMemberIntro(ACM_MemberIntro &me, ACM_MemberIntro &that) {
  mergeNode(me, that);
  // TODO: implement merging for reference member 'intro' -> is that all?
  me.set_intro(that.get_intro ());
  merge_container (that.get_members (), me.get_members (), true);
}
void ProjectModel::mergeBaseIntro(ACM_BaseIntro &me, ACM_BaseIntro &that) {
  mergeNode(me, that);
  // TODO: implement merging for reference member 'intro' -> is that all?
  me.set_intro(that.get_intro ());
  merge_container (that.get_bases (), me.get_bases (), true);
}
void ProjectModel::mergeClassPlan(ACM_ClassPlan &me, ACM_ClassPlan &that) {
  mergeNode(me, that);
  merge_container (that.get_base_intros (), me.get_base_intros ());
  merge_container (that.get_member_intros (), me.get_member_intros ());
}
void ProjectModel::mergeCodeAdvice(ACM_CodeAdvice &me, ACM_CodeAdvice &that) {
  mergeNode(me, that);
  // TODO: implement merging for reference member 'advice' -> is that all?
  me.set_advice(that.get_advice ());
  if (that.get_conditional () != me.get_conditional ()) {
    if (merge_attr (me, that, "conditional"))
      me.set_conditional (that.get_conditional ());
  }
}
void ProjectModel::mergeCodePlan(ACM_CodePlan &me, ACM_CodePlan &that) {
  mergeNode(me, that);
  merge_container (that.get_before (), me.get_before ());
  // TODO: handle merging of optional attribute 'around' -> like this?
  if (me.has_around () && that.has_around ())
    merge_node (me.get_around (), that.get_around ());
  else if (that.has_around ())
    me.set_around((ACM_CodeAdvice*)copyNode (that.get_around ()));
  merge_container (that.get_after (), me.get_after ());
  // TODO: handle merging of optional attribute 'next_level' -> like this?
  if (me.has_next_level () && that.has_next_level ())
    merge_node (me.get_next_level (), that.get_next_level ());
  else if (that.has_next_level ())
    me.set_next_level((ACM_CodePlan*)copyNode (that.get_next_level ()));
}
void ProjectModel::mergeAny(ACM_Any &me, ACM_Any &that) {
  mergeNode(me, that);
  if (that.has_jpid () && !me.has_jpid ())
    merge_attr_only_in_that (me, that, "jpid");
  else if (!that.has_jpid () && me.has_jpid ())
    merge_attr_only_in_me (me, that, "jpid");
  else if (that.has_jpid () && me.has_jpid () &&
      that.get_jpid () != me.get_jpid ()) {
    if (merge_attr (me, that, "jpid"))
      me.set_jpid (that.get_jpid ());
  }
  merge_container (that.get_source (), me.get_source ());
  merge_container (that.get_attributes (), me.get_attributes (), true);
}
void ProjectModel::mergeType(ACM_Type &me, ACM_Type &that) {
  mergeAny(me, that);
}
void ProjectModel::mergeArg(ACM_Arg &me, ACM_Arg &that) {
  mergeAny(me, that);
  if (that.get_type () != me.get_type ()) {
    if (merge_attr (me, that, "type"))
      me.set_type (that.get_type ());
  }
}
void ProjectModel::mergeName(ACM_Name &me, ACM_Name &that) {
  mergeAny(me, that);
  merge_container (that.get_children (), me.get_children ());
  if (that.get_builtin () != me.get_builtin ()) {
    if (merge_attr (me, that, "builtin"))
      me.set_builtin (that.get_builtin ());
  }
  merge_container (that.get_tunits (), me.get_tunits (), true);
}
void ProjectModel::mergeNamespace(ACM_Namespace &me, ACM_Namespace &that) {
  mergeName(me, that);
}
void ProjectModel::mergeFunction(ACM_Function &me, ACM_Function &that) {
  mergeName(me, that);
  // TODO: handle merging of optional attribute 'result_type' -> like this?
  if (me.has_result_type () && that.has_result_type ())
    merge_node (me.get_result_type (), that.get_result_type ());
  else if (that.has_result_type ())
    me.set_result_type((ACM_Type*)copyNode (that.get_result_type ()));
  merge_container (that.get_arg_types (), me.get_arg_types ());
  if (that.get_kind () != me.get_kind ()) {
    if (merge_attr (me, that, "kind"))
      me.set_kind (that.get_kind ());
  }
  if (that.get_variadic_args () != me.get_variadic_args ()) {
    if (merge_attr (me, that, "variadic_args"))
      me.set_variadic_args (that.get_variadic_args ());
  }
  // TODO: implement merging for reference member 'static_in' -> is that all?
  // TODO: What if the reference named slices are different in the two models?
  if (that.has_static_in ())
    me.set_static_in(that.get_static_in ());
  merge_container (that.get_calls (), me.get_calls (), true);
}
void ProjectModel::mergeVariable(ACM_Variable &me, ACM_Variable &that) {
  mergeName(me, that);
  merge_node (me.get_type (), that.get_type ());
  if (that.get_kind () != me.get_kind ()) {
    if (merge_attr (me, that, "kind"))
      me.set_kind (that.get_kind ());
  }
}
void ProjectModel::mergeClass(ACM_Class &me, ACM_Class &that) {
  mergeName(me, that);
  merge_container (that.get_bases (), me.get_bases (), true);
  merge_container (that.get_derived (), me.get_derived (), true);
  // TODO: handle merging of optional attribute 'plan' -> like this?
  if (me.has_plan () && that.has_plan ())
    merge_node (me.get_plan (), that.get_plan ());
  else if (that.has_plan ())
    me.set_plan((ACM_ClassPlan*)copyNode (that.get_plan ()));
}
void ProjectModel::mergeClassSlice(ACM_ClassSlice &me, ACM_ClassSlice &that) {
  mergeName(me, that);
  if (that.get_is_struct () != me.get_is_struct ()) {
    if (merge_attr (me, that, "is_struct"))
      me.set_is_struct (that.get_is_struct ());
  }
}
void ProjectModel::mergeCode(ACM_Code &me, ACM_Code &that) {
  mergeAny(me, that);
  // TODO: handle merging of optional attribute 'plan' -> like this?
  if (me.has_plan () && that.has_plan ())
    merge_node (me.get_plan (), that.get_plan ());
  else if (that.has_plan ())
    me.set_plan((ACM_CodePlan*)copyNode (that.get_plan ()));
}
void ProjectModel::mergeAccess(ACM_Access &me, ACM_Access &that) {
  mergeCode(me, that);
  if (that.has_target_object_lid () && !me.has_target_object_lid ())
    merge_attr_only_in_that (me, that, "target_object_lid");
  else if (!that.has_target_object_lid () && me.has_target_object_lid ())
    merge_attr_only_in_me (me, that, "target_object_lid");
  else if (that.has_target_object_lid () && me.has_target_object_lid () &&
      that.get_target_object_lid () != me.get_target_object_lid ()) {
    if (merge_attr (me, that, "target_object_lid"))
      me.set_target_object_lid (that.get_target_object_lid ());
  }
  if (that.has_cfg_block_lid () && !me.has_cfg_block_lid ())
    merge_attr_only_in_that (me, that, "cfg_block_lid");
  else if (!that.has_cfg_block_lid () && me.has_cfg_block_lid ())
    merge_attr_only_in_me (me, that, "cfg_block_lid");
  else if (that.has_cfg_block_lid () && me.has_cfg_block_lid () &&
      that.get_cfg_block_lid () != me.get_cfg_block_lid ()) {
    if (merge_attr (me, that, "cfg_block_lid"))
      me.set_cfg_block_lid (that.get_cfg_block_lid ());
  }
  // TODO: implement merging for reference member 'target_class' -> is that all?
  // TODO: What if the reference named slices are different in the two models?
  if (that.has_target_class ())
    me.set_target_class(that.get_target_class ());
}
void ProjectModel::mergeGet(ACM_Get &me, ACM_Get &that) {
  mergeAccess(me, that);
  // TODO: implement merging for reference member 'variable' -> is that all?
  // TODO: What if the reference named slices are different in the two models?
  if (that.has_variable ())
    me.set_variable(that.get_variable ());
}
void ProjectModel::mergeSet(ACM_Set &me, ACM_Set &that) {
  mergeAccess(me, that);
  // TODO: implement merging for reference member 'variable' -> is that all?
  // TODO: What if the reference named slices are different in the two models?
  if (that.has_variable ())
    me.set_variable(that.get_variable ());
}
void ProjectModel::mergeCall(ACM_Call &me, ACM_Call &that) {
  mergeAccess(me, that);
  // TODO: implement merging for reference member 'target' -> is that all?
  me.set_target(that.get_target ());
  merge_container (that.get_variadic_arg_types (), me.get_variadic_arg_types ());
  if (that.has_default_args () && !me.has_default_args ())
    merge_attr_only_in_that (me, that, "default_args");
  else if (!that.has_default_args () && me.has_default_args ())
    merge_attr_only_in_me (me, that, "default_args");
  else if (that.has_default_args () && me.has_default_args () &&
      that.get_default_args () != me.get_default_args ()) {
    if (merge_attr (me, that, "default_args"))
      me.set_default_args (that.get_default_args ());
  }
}
void ProjectModel::mergeBuiltin(ACM_Builtin &me, ACM_Builtin &that) {
  mergeAccess(me, that);
  // TODO: implement merging for reference member 'target' -> is that all?
  me.set_target(that.get_target ());
  merge_container (that.get_implicit_access (), me.get_implicit_access ());
}
void ProjectModel::mergeRef(ACM_Ref &me, ACM_Ref &that) {
  mergeAccess(me, that);
  // TODO: implement merging for reference member 'variable' -> is that all?
  me.set_variable(that.get_variable ());
  merge_node (me.get_type (), that.get_type ());
}
void ProjectModel::mergeGetRef(ACM_GetRef &me, ACM_GetRef &that) {
  mergeAccess(me, that);
  merge_node (me.get_type (), that.get_type ());
}
void ProjectModel::mergeSetRef(ACM_SetRef &me, ACM_SetRef &that) {
  mergeAccess(me, that);
  merge_node (me.get_type (), that.get_type ());
}
void ProjectModel::mergeCallRef(ACM_CallRef &me, ACM_CallRef &that) {
  mergeAccess(me, that);
  merge_node (me.get_type (), that.get_type ());
}
void ProjectModel::mergeExecution(ACM_Execution &me, ACM_Execution &that) {
  mergeCode(me, that);
}
void ProjectModel::mergeConstruction(ACM_Construction &me, ACM_Construction &that) {
  mergeCode(me, that);
}
void ProjectModel::mergeDestruction(ACM_Destruction &me, ACM_Destruction &that) {
  mergeCode(me, that);
}
void ProjectModel::mergePointcut(ACM_Pointcut &me, ACM_Pointcut &that) {
  mergeName(me, that);
  if (that.get_expr () != me.get_expr ()) {
    if (merge_attr (me, that, "expr"))
      me.set_expr (that.get_expr ());
  }
  merge_container (that.get_args (), me.get_args ());
  if (that.get_kind () != me.get_kind ()) {
    if (merge_attr (me, that, "kind"))
      me.set_kind (that.get_kind ());
  }
}
void ProjectModel::mergeAttribute(ACM_Attribute &me, ACM_Attribute &that) {
  mergeName(me, that);
  merge_container (that.get_args (), me.get_args ());
}
void ProjectModel::mergeAdvice(ACM_Advice &me, ACM_Advice &that) {
  mergeAny(me, that);
  merge_node (me.get_pointcut (), that.get_pointcut ());
}
void ProjectModel::mergeAdviceCode(ACM_AdviceCode &me, ACM_AdviceCode &that) {
  mergeAdvice(me, that);
  if (that.get_kind () != me.get_kind ()) {
    if (merge_attr (me, that, "kind"))
      me.set_kind (that.get_kind ());
  }
  if (that.get_context () != me.get_context ()) {
    if (merge_attr (me, that, "context"))
      me.set_context (that.get_context ());
  }
}
void ProjectModel::mergeIntroduction(ACM_Introduction &me, ACM_Introduction &that) {
  mergeAdvice(me, that);
  // TODO: implement merging for reference member 'named_slice' -> is that all?
  // TODO: What if the reference named slices are different in the two models?
  if (that.has_named_slice ())
    me.set_named_slice(that.get_named_slice ());
  // TODO: handle merging of optional attribute 'anon_slice' -> like this?
  if (me.has_anon_slice () && that.has_anon_slice ())
    merge_node (me.get_anon_slice (), that.get_anon_slice ());
  else if (that.has_anon_slice ())
    me.set_anon_slice((ACM_ClassSlice*)copyNode (that.get_anon_slice ()));
}
void ProjectModel::mergeOrder(ACM_Order &me, ACM_Order &that) {
  mergeAdvice(me, that);
  merge_container (that.get_aspect_exprs (), me.get_aspect_exprs ());
}
void ProjectModel::mergeAspect(ACM_Aspect &me, ACM_Aspect &that) {
  mergeClass(me, that);
  merge_container (that.get_intros (), me.get_intros ());
  merge_container (that.get_advices (), me.get_advices ());
  merge_container (that.get_orders (), me.get_orders ());
}
void ProjectModel::mergeSpeculation(ACM_Speculation &me, ACM_Speculation &that) {
  mergeNode(me, that);
  // TODO: implement merging for reference member 'advice' -> is that all?
  me.set_advice(that.get_advice ());
  // TODO: implement merging for reference member 'tunit' -> is that all?
  me.set_tunit(that.get_tunit ());
  merge_container (that.get_true_jps (), me.get_true_jps (), true);
  merge_container (that.get_false_jps (), me.get_false_jps (), true);
  merge_container (that.get_conditional_jps (), me.get_conditional_jps (), true);
}
void ProjectModel::mergeModel(ACM_Model &me, ACM_Model &that) {
  mergeNode(me, that);
  merge_container (that.get_files (), me.get_files ());
  merge_node (me.get_root (), that.get_root ());
  merge_container (that.get_speculations (), me.get_speculations ());
  if (that.get_version () != me.get_version ()) {
    if (merge_attr (me, that, "version"))
      me.set_version (that.get_version ());
  }
}

void ProjectModel::fix_refs_Node(ACM_Node &me) {
}
void ProjectModel::fix_refs_File(ACM_File &me) {
  fix_refs_Node(me);
}
void ProjectModel::fix_refs_TUnit(ACM_TUnit &me) {
  fix_refs_File(me);
}
void ProjectModel::fix_refs_Header(ACM_Header &me) {
  fix_refs_File(me);
  ACM_Container<ACM_TUnit, false>  new_in;
  for (ACM_Container<ACM_TUnit, false> ::iterator i = me.get_in ().begin (); i != me.get_in ().end (); ++i)
    if ((*i)->get_partner ())
      new_in.insert ((ACM_TUnit*)(*i)->get_partner ());
    else
      new_in.insert ((ACM_TUnit*)(*i));
  me.get_in () = new_in;
}
void ProjectModel::fix_refs_Source(ACM_Source &me) {
  fix_refs_Node(me);
  if (me.get_file ()->get_partner ())
    me.set_file((ACM_File*)me.get_file ()->get_partner ());
}
void ProjectModel::fix_refs_MemberIntro(ACM_MemberIntro &me) {
  fix_refs_Node(me);
  if (me.get_intro ()->get_partner ())
    me.set_intro((ACM_Introduction*)me.get_intro ()->get_partner ());
  ACM_Container<ACM_Name, false>  new_members;
  for (ACM_Container<ACM_Name, false> ::iterator i = me.get_members ().begin (); i != me.get_members ().end (); ++i)
    if ((*i)->get_partner ())
      new_members.insert ((ACM_Name*)(*i)->get_partner ());
    else
      new_members.insert ((ACM_Name*)(*i));
  me.get_members () = new_members;
}
void ProjectModel::fix_refs_BaseIntro(ACM_BaseIntro &me) {
  fix_refs_Node(me);
  if (me.get_intro ()->get_partner ())
    me.set_intro((ACM_Introduction*)me.get_intro ()->get_partner ());
  ACM_Container<ACM_Class, false>  new_bases;
  for (ACM_Container<ACM_Class, false> ::iterator i = me.get_bases ().begin (); i != me.get_bases ().end (); ++i)
    if ((*i)->get_partner ())
      new_bases.insert ((ACM_Class*)(*i)->get_partner ());
    else
      new_bases.insert ((ACM_Class*)(*i));
  me.get_bases () = new_bases;
}
void ProjectModel::fix_refs_ClassPlan(ACM_ClassPlan &me) {
  fix_refs_Node(me);
  for (ACM_Container<ACM_BaseIntro, true> ::iterator i = me.get_base_intros ().begin (); i != me.get_base_intros ().end (); ++i)
    fix_refs (*i);
  for (ACM_Container<ACM_MemberIntro, true> ::iterator i = me.get_member_intros ().begin (); i != me.get_member_intros ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_CodeAdvice(ACM_CodeAdvice &me) {
  fix_refs_Node(me);
  if (me.get_advice ()->get_partner ())
    me.set_advice((ACM_AdviceCode*)me.get_advice ()->get_partner ());
}
void ProjectModel::fix_refs_CodePlan(ACM_CodePlan &me) {
  fix_refs_Node(me);
  for (ACM_Container<ACM_CodeAdvice, true> ::iterator i = me.get_before ().begin (); i != me.get_before ().end (); ++i)
    fix_refs (*i);
  if (me.has_around()) fix_refs (me.get_around());
  for (ACM_Container<ACM_CodeAdvice, true> ::iterator i = me.get_after ().begin (); i != me.get_after ().end (); ++i)
    fix_refs (*i);
  if (me.has_next_level()) fix_refs (me.get_next_level());
}
void ProjectModel::fix_refs_Any(ACM_Any &me) {
  fix_refs_Node(me);
  for (ACM_Container<ACM_Source, true> ::iterator i = me.get_source ().begin (); i != me.get_source ().end (); ++i)
    fix_refs (*i);
  ACM_Container<ACM_Attribute, false>  new_attributes;
  for (ACM_Container<ACM_Attribute, false> ::iterator i = me.get_attributes ().begin (); i != me.get_attributes ().end (); ++i)
    if ((*i)->get_partner ())
      new_attributes.insert ((ACM_Attribute*)(*i)->get_partner ());
    else
      new_attributes.insert ((ACM_Attribute*)(*i));
  me.get_attributes () = new_attributes;
}
void ProjectModel::fix_refs_Type(ACM_Type &me) {
  fix_refs_Any(me);
}
void ProjectModel::fix_refs_Arg(ACM_Arg &me) {
  fix_refs_Any(me);
}
void ProjectModel::fix_refs_Name(ACM_Name &me) {
  fix_refs_Any(me);
  for (ACM_Container<ACM_Any, true> ::iterator i = me.get_children ().begin (); i != me.get_children ().end (); ++i)
    fix_refs (*i);
  ACM_Container<ACM_TUnit, false>  new_tunits;
  for (ACM_Container<ACM_TUnit, false> ::iterator i = me.get_tunits ().begin (); i != me.get_tunits ().end (); ++i)
    if ((*i)->get_partner ())
      new_tunits.insert ((ACM_TUnit*)(*i)->get_partner ());
    else
      new_tunits.insert ((ACM_TUnit*)(*i));
  me.get_tunits () = new_tunits;
}
void ProjectModel::fix_refs_Namespace(ACM_Namespace &me) {
  fix_refs_Name(me);
}
void ProjectModel::fix_refs_Function(ACM_Function &me) {
  fix_refs_Name(me);
  if (me.has_result_type()) fix_refs (me.get_result_type());
  for (ACM_Container<ACM_Type, true> ::iterator i = me.get_arg_types ().begin (); i != me.get_arg_types ().end (); ++i)
    fix_refs (*i);
  if (me.has_static_in() && me.get_static_in ()->get_partner ())
    me.set_static_in((ACM_TUnit*)me.get_static_in ()->get_partner ());
  ACM_Container<ACM_Call, false>  new_calls;
  for (ACM_Container<ACM_Call, false> ::iterator i = me.get_calls ().begin (); i != me.get_calls ().end (); ++i)
    if ((*i)->get_partner ())
      new_calls.insert ((ACM_Call*)(*i)->get_partner ());
    else
      new_calls.insert ((ACM_Call*)(*i));
  me.get_calls () = new_calls;
}
void ProjectModel::fix_refs_Variable(ACM_Variable &me) {
  fix_refs_Name(me);
  fix_refs (me.get_type());
}
void ProjectModel::fix_refs_Class(ACM_Class &me) {
  fix_refs_Name(me);
  ACM_Container<ACM_Class, false>  new_bases;
  for (ACM_Container<ACM_Class, false> ::iterator i = me.get_bases ().begin (); i != me.get_bases ().end (); ++i)
    if ((*i)->get_partner ())
      new_bases.insert ((ACM_Class*)(*i)->get_partner ());
    else
      new_bases.insert ((ACM_Class*)(*i));
  me.get_bases () = new_bases;
  ACM_Container<ACM_Class, false>  new_derived;
  for (ACM_Container<ACM_Class, false> ::iterator i = me.get_derived ().begin (); i != me.get_derived ().end (); ++i)
    if ((*i)->get_partner ())
      new_derived.insert ((ACM_Class*)(*i)->get_partner ());
    else
      new_derived.insert ((ACM_Class*)(*i));
  me.get_derived () = new_derived;
  if (me.has_plan()) fix_refs (me.get_plan());
}
void ProjectModel::fix_refs_ClassSlice(ACM_ClassSlice &me) {
  fix_refs_Name(me);
}
void ProjectModel::fix_refs_Code(ACM_Code &me) {
  fix_refs_Any(me);
  if (me.has_plan()) fix_refs (me.get_plan());
}
void ProjectModel::fix_refs_Access(ACM_Access &me) {
  fix_refs_Code(me);
  if (me.has_target_class() && me.get_target_class ()->get_partner ())
    me.set_target_class((ACM_Class*)me.get_target_class ()->get_partner ());
}
void ProjectModel::fix_refs_Get(ACM_Get &me) {
  fix_refs_Access(me);
  if (me.has_variable() && me.get_variable ()->get_partner ())
    me.set_variable((ACM_Variable*)me.get_variable ()->get_partner ());
}
void ProjectModel::fix_refs_Set(ACM_Set &me) {
  fix_refs_Access(me);
  if (me.has_variable() && me.get_variable ()->get_partner ())
    me.set_variable((ACM_Variable*)me.get_variable ()->get_partner ());
}
void ProjectModel::fix_refs_Call(ACM_Call &me) {
  fix_refs_Access(me);
  if (me.get_target ()->get_partner ())
    me.set_target((ACM_Function*)me.get_target ()->get_partner ());
  for (ACM_Container<ACM_Type, true> ::iterator i = me.get_variadic_arg_types ().begin (); i != me.get_variadic_arg_types ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Builtin(ACM_Builtin &me) {
  fix_refs_Access(me);
  if (me.get_target ()->get_partner ())
    me.set_target((ACM_Function*)me.get_target ()->get_partner ());
  for (ACM_Container<ACM_Access, true> ::iterator i = me.get_implicit_access ().begin (); i != me.get_implicit_access ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Ref(ACM_Ref &me) {
  fix_refs_Access(me);
  if (me.get_variable ()->get_partner ())
    me.set_variable((ACM_Variable*)me.get_variable ()->get_partner ());
  fix_refs (me.get_type());
}
void ProjectModel::fix_refs_GetRef(ACM_GetRef &me) {
  fix_refs_Access(me);
  fix_refs (me.get_type());
}
void ProjectModel::fix_refs_SetRef(ACM_SetRef &me) {
  fix_refs_Access(me);
  fix_refs (me.get_type());
}
void ProjectModel::fix_refs_CallRef(ACM_CallRef &me) {
  fix_refs_Access(me);
  fix_refs (me.get_type());
}
void ProjectModel::fix_refs_Execution(ACM_Execution &me) {
  fix_refs_Code(me);
}
void ProjectModel::fix_refs_Construction(ACM_Construction &me) {
  fix_refs_Code(me);
}
void ProjectModel::fix_refs_Destruction(ACM_Destruction &me) {
  fix_refs_Code(me);
}
void ProjectModel::fix_refs_Pointcut(ACM_Pointcut &me) {
  fix_refs_Name(me);
  for (ACM_Container<ACM_Arg, true> ::iterator i = me.get_args ().begin (); i != me.get_args ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Attribute(ACM_Attribute &me) {
  fix_refs_Name(me);
  for (ACM_Container<ACM_Arg, true> ::iterator i = me.get_args ().begin (); i != me.get_args ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Advice(ACM_Advice &me) {
  fix_refs_Any(me);
  fix_refs (me.get_pointcut());
}
void ProjectModel::fix_refs_AdviceCode(ACM_AdviceCode &me) {
  fix_refs_Advice(me);
}
void ProjectModel::fix_refs_Introduction(ACM_Introduction &me) {
  fix_refs_Advice(me);
  if (me.has_named_slice() && me.get_named_slice ()->get_partner ())
    me.set_named_slice((ACM_ClassSlice*)me.get_named_slice ()->get_partner ());
  if (me.has_anon_slice()) fix_refs (me.get_anon_slice());
}
void ProjectModel::fix_refs_Order(ACM_Order &me) {
  fix_refs_Advice(me);
  for (ACM_Container<ACM_Pointcut, true> ::iterator i = me.get_aspect_exprs ().begin (); i != me.get_aspect_exprs ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Aspect(ACM_Aspect &me) {
  fix_refs_Class(me);
  for (ACM_Container<ACM_Introduction, true> ::iterator i = me.get_intros ().begin (); i != me.get_intros ().end (); ++i)
    fix_refs (*i);
  for (ACM_Container<ACM_AdviceCode, true> ::iterator i = me.get_advices ().begin (); i != me.get_advices ().end (); ++i)
    fix_refs (*i);
  for (ACM_Container<ACM_Order, true> ::iterator i = me.get_orders ().begin (); i != me.get_orders ().end (); ++i)
    fix_refs (*i);
}
void ProjectModel::fix_refs_Speculation(ACM_Speculation &me) {
  fix_refs_Node(me);
  if (me.get_advice ()->get_partner ())
    me.set_advice((ACM_Advice*)me.get_advice ()->get_partner ());
  if (me.get_tunit ()->get_partner ())
    me.set_tunit((ACM_TUnit*)me.get_tunit ()->get_partner ());
  ACM_Container<ACM_Any, false>  new_true_jps;
  for (ACM_Container<ACM_Any, false> ::iterator i = me.get_true_jps ().begin (); i != me.get_true_jps ().end (); ++i)
    if ((*i)->get_partner ())
      new_true_jps.insert ((ACM_Any*)(*i)->get_partner ());
    else
      new_true_jps.insert ((ACM_Any*)(*i));
  me.get_true_jps () = new_true_jps;
  ACM_Container<ACM_Any, false>  new_false_jps;
  for (ACM_Container<ACM_Any, false> ::iterator i = me.get_false_jps ().begin (); i != me.get_false_jps ().end (); ++i)
    if ((*i)->get_partner ())
      new_false_jps.insert ((ACM_Any*)(*i)->get_partner ());
    else
      new_false_jps.insert ((ACM_Any*)(*i));
  me.get_false_jps () = new_false_jps;
  ACM_Container<ACM_Any, false>  new_conditional_jps;
  for (ACM_Container<ACM_Any, false> ::iterator i = me.get_conditional_jps ().begin (); i != me.get_conditional_jps ().end (); ++i)
    if ((*i)->get_partner ())
      new_conditional_jps.insert ((ACM_Any*)(*i)->get_partner ());
    else
      new_conditional_jps.insert ((ACM_Any*)(*i));
  me.get_conditional_jps () = new_conditional_jps;
}
void ProjectModel::fix_refs_Model(ACM_Model &me) {
  fix_refs_Node(me);
  for (ACM_Container<ACM_File, true> ::iterator i = me.get_files ().begin (); i != me.get_files ().end (); ++i)
    fix_refs (*i);
  fix_refs (me.get_root());
  for (ACM_Container<ACM_Speculation, true> ::iterator i = me.get_speculations ().begin (); i != me.get_speculations ().end (); ++i)
    fix_refs (*i);
}

