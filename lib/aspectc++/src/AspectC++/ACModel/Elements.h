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

#ifndef __Elements_h__
#define __Elements_h__
#include "ModelNode.h"
#include <string>
#include <sstream>

#include <iostream>


// define enums
enum SourceLocKind { SLK_NONE, SLK_DEF, SLK_DECL };
enum FunctionType { FT_UNKNOWN, FT_NON_MEMBER, FT_STATIC_NON_MEMBER, FT_MEMBER, FT_STATIC_MEMBER, FT_VIRTUAL_MEMBER, FT_PURE_VIRTUAL_MEMBER, FT_CONSTRUCTOR, FT_DESTRUCTOR, FT_VIRTUAL_DESTRUCTOR, FT_PURE_VIRTUAL_DESTRUCTOR };
enum PointcutType { PT_NORMAL, PT_VIRTUAL, PT_PURE_VIRTUAL };
enum VariableType { VT_UNKNOWN, VT_NON_MEMBER, VT_STATIC_NON_MEMBER, VT_MEMBER, VT_STATIC_MEMBER };
enum AdviceCodeType { ACT_BEFORE, ACT_AFTER, ACT_AROUND };
enum AdviceCodeContext { ACC_NONE, ACC_TYPE, ACC_OBJ, ACC_TYPE_OBJ, ACC_VARS, ACC_TYPE_VARS, ACC_OBJ_VARS, ACC_TYPE_OBJ_VARS };
enum CVQualifiers { CVQ_NONE, CVQ_CONST, CVQ_VOLATILE, CVQ_CONST_VOLATILE };

// define entity IDs
typedef unsigned long long JoinPointType;
static const JoinPointType JPT_TUnit = 1ULL << 0;
static const int JPTI_TUnit = 0;
static const JoinPointType JPT_Header = 1ULL << 1;
static const int JPTI_Header = 1;
static const JoinPointType JPT_Source = 1ULL << 2;
static const int JPTI_Source = 2;
static const JoinPointType JPT_MemberIntro = 1ULL << 3;
static const int JPTI_MemberIntro = 3;
static const JoinPointType JPT_BaseIntro = 1ULL << 4;
static const int JPTI_BaseIntro = 4;
static const JoinPointType JPT_ClassPlan = 1ULL << 5;
static const int JPTI_ClassPlan = 5;
static const JoinPointType JPT_CodeAdvice = 1ULL << 6;
static const int JPTI_CodeAdvice = 6;
static const JoinPointType JPT_CodePlan = 1ULL << 7;
static const int JPTI_CodePlan = 7;
static const JoinPointType JPT_Type = 1ULL << 8;
static const int JPTI_Type = 8;
static const JoinPointType JPT_Arg = 1ULL << 9;
static const int JPTI_Arg = 9;
static const JoinPointType JPT_Namespace = 1ULL << 10;
static const int JPTI_Namespace = 10;
static const JoinPointType JPT_Function = 1ULL << 11;
static const int JPTI_Function = 11;
static const JoinPointType JPT_Variable = 1ULL << 12;
static const int JPTI_Variable = 12;
static const JoinPointType JPT_Class = 1ULL << 13;
static const int JPTI_Class = 13;
static const JoinPointType JPT_ClassSlice = 1ULL << 14;
static const int JPTI_ClassSlice = 14;
static const JoinPointType JPT_Get = 1ULL << 15;
static const int JPTI_Get = 15;
static const JoinPointType JPT_Set = 1ULL << 16;
static const int JPTI_Set = 16;
static const JoinPointType JPT_Call = 1ULL << 17;
static const int JPTI_Call = 17;
static const JoinPointType JPT_Builtin = 1ULL << 18;
static const int JPTI_Builtin = 18;
static const JoinPointType JPT_Ref = 1ULL << 19;
static const int JPTI_Ref = 19;
static const JoinPointType JPT_GetRef = 1ULL << 20;
static const int JPTI_GetRef = 20;
static const JoinPointType JPT_SetRef = 1ULL << 21;
static const int JPTI_SetRef = 21;
static const JoinPointType JPT_CallRef = 1ULL << 22;
static const int JPTI_CallRef = 22;
static const JoinPointType JPT_Execution = 1ULL << 23;
static const int JPTI_Execution = 23;
static const JoinPointType JPT_Construction = 1ULL << 24;
static const int JPTI_Construction = 24;
static const JoinPointType JPT_Destruction = 1ULL << 25;
static const int JPTI_Destruction = 25;
static const JoinPointType JPT_Pointcut = 1ULL << 26;
static const int JPTI_Pointcut = 26;
static const JoinPointType JPT_Attribute = 1ULL << 27;
static const int JPTI_Attribute = 27;
static const JoinPointType JPT_AdviceCode = 1ULL << 28;
static const int JPTI_AdviceCode = 28;
static const JoinPointType JPT_Introduction = 1ULL << 29;
static const int JPTI_Introduction = 29;
static const JoinPointType JPT_Order = 1ULL << 30;
static const int JPTI_Order = 30;
static const JoinPointType JPT_Aspect = 1ULL << 31;
static const int JPTI_Aspect = 31;
static const JoinPointType JPT_Speculation = 1ULL << 32;
static const int JPTI_Speculation = 32;
static const JoinPointType JPT_Model = 1ULL << 33;
static const int JPTI_Model = 33;
static const int MAX_JOIN_POINT_TYPE_INDEX = 34;
static const JoinPointType JPT_Node = JPT_TUnit|JPT_Header|JPT_Source|JPT_MemberIntro|JPT_BaseIntro|JPT_ClassPlan|JPT_CodeAdvice|JPT_CodePlan|JPT_Type|JPT_Arg|JPT_Namespace|JPT_Function|JPT_Variable|JPT_Class|JPT_ClassSlice|JPT_Get|JPT_Set|JPT_Call|JPT_Builtin|JPT_Ref|JPT_GetRef|JPT_SetRef|JPT_CallRef|JPT_Execution|JPT_Construction|JPT_Destruction|JPT_Pointcut|JPT_Attribute|JPT_AdviceCode|JPT_Introduction|JPT_Order|JPT_Aspect|JPT_Speculation|JPT_Model;
static const JoinPointType JPT_File = JPT_TUnit|JPT_Header;
static const JoinPointType JPT_Any = JPT_Type|JPT_Arg|JPT_Namespace|JPT_Function|JPT_Variable|JPT_Class|JPT_ClassSlice|JPT_Get|JPT_Set|JPT_Call|JPT_Builtin|JPT_Ref|JPT_GetRef|JPT_SetRef|JPT_CallRef|JPT_Execution|JPT_Construction|JPT_Destruction|JPT_Pointcut|JPT_Attribute|JPT_AdviceCode|JPT_Introduction|JPT_Order|JPT_Aspect;
static const JoinPointType JPT_Name = JPT_Namespace|JPT_Function|JPT_Variable|JPT_Class|JPT_ClassSlice|JPT_Pointcut|JPT_Attribute|JPT_Aspect;
static const JoinPointType JPT_Code = JPT_Get|JPT_Set|JPT_Call|JPT_Builtin|JPT_Ref|JPT_GetRef|JPT_SetRef|JPT_CallRef|JPT_Execution|JPT_Construction|JPT_Destruction;
static const JoinPointType JPT_Access = JPT_Get|JPT_Set|JPT_Call|JPT_Builtin|JPT_Ref|JPT_GetRef|JPT_SetRef|JPT_CallRef;
static const JoinPointType JPT_Advice = JPT_AdviceCode|JPT_Introduction|JPT_Order;

// forward declarations of join-point model classes
class ACM_Node;
class ACM_File;
class ACM_TUnit;
class ACM_Header;
class ACM_Source;
class ACM_MemberIntro;
class ACM_BaseIntro;
class ACM_ClassPlan;
class ACM_CodeAdvice;
class ACM_CodePlan;
class ACM_Any;
class ACM_Type;
class ACM_Arg;
class ACM_Name;
class ACM_Namespace;
class ACM_Function;
class ACM_Variable;
class ACM_Class;
class ACM_ClassSlice;
class ACM_Code;
class ACM_Access;
class ACM_Get;
class ACM_Set;
class ACM_Call;
class ACM_Builtin;
class ACM_Ref;
class ACM_GetRef;
class ACM_SetRef;
class ACM_CallRef;
class ACM_Execution;
class ACM_Construction;
class ACM_Destruction;
class ACM_Pointcut;
class ACM_Attribute;
class ACM_Advice;
class ACM_AdviceCode;
class ACM_Introduction;
class ACM_Order;
class ACM_Aspect;
class ACM_Speculation;
class ACM_Model;

// class definitions

class ACM_Node  : public ModelNode {
public:
  ACM_Node () {
  }
  virtual ~ACM_Node () {}
  virtual const char *type_str () const { return "Node"; }
  virtual JoinPointType type_val () const { return JPT_Node; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

};

class ACM_File  : public ACM_Node {
  string filename_;
  int len_;
  int time_;
public:
  ACM_File () {
  }
  virtual ~ACM_File () {}
  virtual const char *type_str () const { return "File"; }
  virtual JoinPointType type_val () const { return JPT_File; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_filename (string arg) { set_xml_dirty (true); filename_ = arg; }
  string get_filename() const { return filename_; }

  void set_len (int arg) { set_xml_dirty (true); len_ = arg; }
  int get_len() const { return len_; }

  void set_time (int arg) { set_xml_dirty (true); time_ = arg; }
  int get_time() const { return time_; }

};

class ACM_TUnit  : public ACM_File {
  bool dirty_flag_;
public:
  ACM_TUnit () {
  }
  virtual ~ACM_TUnit () {}
  virtual const char *type_str () const { return "TUnit"; }
  virtual JoinPointType type_val () const { return JPT_TUnit; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_dirty_flag (bool arg) { set_xml_dirty (true); dirty_flag_ = arg; }
  bool get_dirty_flag() const { return dirty_flag_; }

};

class ACM_Header  : public ACM_File {
  ACM_Container<ACM_TUnit, false> in_;
public:
  ACM_Header () {
  }
  virtual ~ACM_Header () {}
  virtual const char *type_str () const { return "Header"; }
  virtual JoinPointType type_val () const { return JPT_Header; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_TUnit, false> &get_in() { return in_; }
  const ACM_Container<ACM_TUnit, false> &get_in() const { return in_; }

};

class ACM_Source  : public ACM_Node {
  SourceLocKind kind_;
  ACM_File* file_;
  int line_;
  int len_;
public:
  ACM_Source () {
  }
  virtual ~ACM_Source () {}
  virtual const char *type_str () const { return "Source"; }
  virtual JoinPointType type_val () const { return JPT_Source; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_kind (SourceLocKind arg) { set_xml_dirty (true); kind_ = arg; }
  SourceLocKind get_kind() const { return kind_; }

  void set_file (ACM_File* arg) { set_xml_dirty (true); file_ = arg; }
  ACM_File* get_file() { return file_; }

  void set_line (int arg) { set_xml_dirty (true); line_ = arg; }
  int get_line() const { return line_; }

  void set_len (int arg) { set_xml_dirty (true); len_ = arg; }
  int get_len() const { return len_; }

};

class ACM_MemberIntro  : public ACM_Node {
  ACM_Introduction* intro_;
  ACM_Container<ACM_Name, false> members_;
public:
  ACM_MemberIntro () {
  }
  virtual ~ACM_MemberIntro () {}
  virtual const char *type_str () const { return "MemberIntro"; }
  virtual JoinPointType type_val () const { return JPT_MemberIntro; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_intro (ACM_Introduction* arg) { set_xml_dirty (true); intro_ = arg; }
  ACM_Introduction* get_intro() { return intro_; }

  ACM_Container<ACM_Name, false> &get_members() { return members_; }
  const ACM_Container<ACM_Name, false> &get_members() const { return members_; }

};

class ACM_BaseIntro  : public ACM_Node {
  ACM_Introduction* intro_;
  ACM_Container<ACM_Class, false> bases_;
public:
  ACM_BaseIntro () {
  }
  virtual ~ACM_BaseIntro () {}
  virtual const char *type_str () const { return "BaseIntro"; }
  virtual JoinPointType type_val () const { return JPT_BaseIntro; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_intro (ACM_Introduction* arg) { set_xml_dirty (true); intro_ = arg; }
  ACM_Introduction* get_intro() { return intro_; }

  ACM_Container<ACM_Class, false> &get_bases() { return bases_; }
  const ACM_Container<ACM_Class, false> &get_bases() const { return bases_; }

};

class ACM_ClassPlan  : public ACM_Node {
  ACM_Container<ACM_BaseIntro, true> base_intros_;
  ACM_Container<ACM_MemberIntro, true> member_intros_;
public:
  ACM_ClassPlan () {
    base_intros_.set_parent (this);
    member_intros_.set_parent (this);
  }
  virtual ~ACM_ClassPlan () {}
  virtual const char *type_str () const { return "ClassPlan"; }
  virtual JoinPointType type_val () const { return JPT_ClassPlan; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_BaseIntro, true> &get_base_intros() { return base_intros_; }
  const ACM_Container<ACM_BaseIntro, true> &get_base_intros() const { return base_intros_; }

  ACM_Container<ACM_MemberIntro, true> &get_member_intros() { return member_intros_; }
  const ACM_Container<ACM_MemberIntro, true> &get_member_intros() const { return member_intros_; }

};

class ACM_CodeAdvice  : public ACM_Node {
  ACM_AdviceCode* advice_;
  bool conditional_;
public:
  ACM_CodeAdvice () {
  }
  virtual ~ACM_CodeAdvice () {}
  virtual const char *type_str () const { return "CodeAdvice"; }
  virtual JoinPointType type_val () const { return JPT_CodeAdvice; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_advice (ACM_AdviceCode* arg) { set_xml_dirty (true); advice_ = arg; }
  ACM_AdviceCode* get_advice() { return advice_; }

  void set_conditional (bool arg) { set_xml_dirty (true); conditional_ = arg; }
  bool get_conditional() const { return conditional_; }

};

class ACM_CodePlan  : public ACM_Node {
  ACM_Container<ACM_CodeAdvice, true> before_;
  bool has_around_;
  ACM_CodeAdvice* around_;
  ACM_Container<ACM_CodeAdvice, true> after_;
  bool has_next_level_;
  ACM_CodePlan* next_level_;
public:
  ACM_CodePlan () {
    before_.set_parent (this);
    has_around_ = false;
    after_.set_parent (this);
    has_next_level_ = false;
  }
  virtual ~ACM_CodePlan () {}
  virtual const char *type_str () const { return "CodePlan"; }
  virtual JoinPointType type_val () const { return JPT_CodePlan; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_CodeAdvice, true> &get_before() { return before_; }
  const ACM_Container<ACM_CodeAdvice, true> &get_before() const { return before_; }

  bool has_around () const { return has_around_; }
  void set_around (ACM_CodeAdvice* arg) { set_xml_dirty (true); around_ = arg; if (arg) arg->set_parent(this); has_around_ = true; }
  ACM_CodeAdvice* get_around() { return around_; }

  ACM_Container<ACM_CodeAdvice, true> &get_after() { return after_; }
  const ACM_Container<ACM_CodeAdvice, true> &get_after() const { return after_; }

  bool has_next_level () const { return has_next_level_; }
  void set_next_level (ACM_CodePlan* arg) { set_xml_dirty (true); next_level_ = arg; if (arg) arg->set_parent(this); has_next_level_ = true; }
  ACM_CodePlan* get_next_level() { return next_level_; }

};

class ACM_Any  : public ACM_Node {
  bool has_jpid_;
  int jpid_;
  ACM_Container<ACM_Source, true> source_;
  ACM_Container<ACM_Attribute, false> attributes_;
public:
  ACM_Any () {
  has_jpid_ = false;
    source_.set_parent (this);
  }
  virtual ~ACM_Any () {}
  virtual const char *type_str () const { return "Any"; }
  virtual JoinPointType type_val () const { return JPT_Any; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_jpid () const { return has_jpid_; }
  void set_jpid (int arg) { set_xml_dirty (true); jpid_ = arg; has_jpid_ = true; }
  int get_jpid() const { return jpid_; }

  ACM_Container<ACM_Source, true> &get_source() { return source_; }
  const ACM_Container<ACM_Source, true> &get_source() const { return source_; }

  ACM_Container<ACM_Attribute, false> &get_attributes() { return attributes_; }
  const ACM_Container<ACM_Attribute, false> &get_attributes() const { return attributes_; }

};

class ACM_Type  : public ACM_Any {
  string signature_;
  MatchSignature match_sig_;
public:
  ACM_Type () {
  }
  virtual ~ACM_Type () {}
  virtual const char *type_str () const { return "Type"; }
  virtual JoinPointType type_val () const { return JPT_Type; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_signature (string arg) { set_xml_dirty (true); signature_ = arg; }
  string get_signature() const { return signature_; }

  MatchSignature &get_match_sig() { return match_sig_; }
  const MatchSignature &get_match_sig() const { return match_sig_; }

};

class ACM_Arg  : public ACM_Any {
  string type_;
  string name_;
public:
  ACM_Arg () {
  }
  virtual ~ACM_Arg () {}
  virtual const char *type_str () const { return "Arg"; }
  virtual JoinPointType type_val () const { return JPT_Arg; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_type (string arg) { set_xml_dirty (true); type_ = arg; }
  string get_type() const { return type_; }

  void set_name (string arg) { set_xml_dirty (true); name_ = arg; }
  string get_name() const { return name_; }

};

class ACM_Name  : public ACM_Any {
  ACM_Container<ACM_Any, true> children_;
  string name_;
  MatchSignature match_sig_;
  NameMap name_map_;
  bool builtin_;
  ACM_Container<ACM_TUnit, false> tunits_;
public:
  ACM_Name () {
    children_.set_parent (this);
  }
  virtual ~ACM_Name () {}
  virtual const char *type_str () const { return "Name"; }
  virtual JoinPointType type_val () const { return JPT_Name; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_Any, true> &get_children() { return children_; }
  const ACM_Container<ACM_Any, true> &get_children() const { return children_; }

  void set_name (string arg) { set_xml_dirty (true); name_ = arg; }
  string get_name() const { return name_; }

  MatchSignature &get_match_sig() { return match_sig_; }
  const MatchSignature &get_match_sig() const { return match_sig_; }

  NameMap &get_name_map() { return name_map_; }
  const NameMap &get_name_map() const { return name_map_; }

  void set_builtin (bool arg) { set_xml_dirty (true); builtin_ = arg; }
  bool get_builtin() const { return builtin_; }

  ACM_Container<ACM_TUnit, false> &get_tunits() { return tunits_; }
  const ACM_Container<ACM_TUnit, false> &get_tunits() const { return tunits_; }

};

class ACM_Namespace  : public ACM_Name {
public:
  ACM_Namespace () {
  }
  virtual ~ACM_Namespace () {}
  virtual const char *type_str () const { return "Namespace"; }
  virtual JoinPointType type_val () const { return JPT_Namespace; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

};

class ACM_Function  : public ACM_Name {
  bool has_result_type_;
  ACM_Type* result_type_;
  ACM_Container<ACM_Type, true> arg_types_;
  FunctionType kind_;
  bool variadic_args_;
  CVQualifiers cv_qualifiers_;
  bool has_static_in_;
  ACM_TUnit* static_in_;
  ACM_Container<ACM_Call, false> calls_;
public:
  ACM_Function () {
    has_result_type_ = false;
    arg_types_.set_parent (this);
    has_static_in_ = false;
  }
  virtual ~ACM_Function () {}
  virtual const char *type_str () const { return "Function"; }
  virtual JoinPointType type_val () const { return JPT_Function; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_result_type () const { return has_result_type_; }
  void set_result_type (ACM_Type* arg) { set_xml_dirty (true); result_type_ = arg; if (arg) arg->set_parent(this); has_result_type_ = true; }
  ACM_Type* get_result_type() { return result_type_; }

  ACM_Container<ACM_Type, true> &get_arg_types() { return arg_types_; }
  const ACM_Container<ACM_Type, true> &get_arg_types() const { return arg_types_; }

  void set_kind (FunctionType arg) { set_xml_dirty (true); kind_ = arg; }
  FunctionType get_kind() const { return kind_; }

  void set_variadic_args (bool arg) { set_xml_dirty (true); variadic_args_ = arg; }
  bool get_variadic_args() const { return variadic_args_; }

  void set_cv_qualifiers (CVQualifiers arg) { set_xml_dirty (true); cv_qualifiers_ = arg; }
  CVQualifiers get_cv_qualifiers() const { return cv_qualifiers_; }

  bool has_static_in () const { return has_static_in_; }
  void set_static_in (ACM_TUnit* arg) { set_xml_dirty (true); static_in_ = arg; has_static_in_ = true; }
  ACM_TUnit* get_static_in() { return static_in_; }

  ACM_Container<ACM_Call, false> &get_calls() { return calls_; }
  const ACM_Container<ACM_Call, false> &get_calls() const { return calls_; }

};

class ACM_Variable  : public ACM_Name {
  ACM_Type* type_;
  VariableType kind_;
public:
  ACM_Variable () {
  }
  virtual ~ACM_Variable () {}
  virtual const char *type_str () const { return "Variable"; }
  virtual JoinPointType type_val () const { return JPT_Variable; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_type (ACM_Type* arg) { set_xml_dirty (true); type_ = arg; if (arg) arg->set_parent(this); }
  ACM_Type* get_type() { return type_; }

  void set_kind (VariableType arg) { set_xml_dirty (true); kind_ = arg; }
  VariableType get_kind() const { return kind_; }

};

class ACM_Class  : public ACM_Name {
  ACM_Container<ACM_Class, false> bases_;
  ACM_Container<ACM_Class, false> derived_;
  bool intro_target_;
  bool has_plan_;
  ACM_ClassPlan* plan_;
public:
  ACM_Class () {
    has_plan_ = false;
  }
  virtual ~ACM_Class () {}
  virtual const char *type_str () const { return "Class"; }
  virtual JoinPointType type_val () const { return JPT_Class; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_Class, false> &get_bases() { return bases_; }
  const ACM_Container<ACM_Class, false> &get_bases() const { return bases_; }

  ACM_Container<ACM_Class, false> &get_derived() { return derived_; }
  const ACM_Container<ACM_Class, false> &get_derived() const { return derived_; }

  void set_intro_target (bool arg) { set_xml_dirty (true); intro_target_ = arg; }
  bool get_intro_target() const { return intro_target_; }

  bool has_plan () const { return has_plan_; }
  void set_plan (ACM_ClassPlan* arg) { set_xml_dirty (true); plan_ = arg; if (arg) arg->set_parent(this); has_plan_ = true; }
  ACM_ClassPlan* get_plan() { return plan_; }

};

class ACM_ClassSlice  : public ACM_Name {
  bool is_struct_;
public:
  ACM_ClassSlice () {
  }
  virtual ~ACM_ClassSlice () {}
  virtual const char *type_str () const { return "ClassSlice"; }
  virtual JoinPointType type_val () const { return JPT_ClassSlice; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_is_struct (bool arg) { set_xml_dirty (true); is_struct_ = arg; }
  bool get_is_struct() const { return is_struct_; }

};

class ACM_Code  : public ACM_Any {
  bool has_plan_;
  ACM_CodePlan* plan_;
public:
  ACM_Code () {
    has_plan_ = false;
  }
  virtual ~ACM_Code () {}
  virtual const char *type_str () const { return "Code"; }
  virtual JoinPointType type_val () const { return JPT_Code; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_plan () const { return has_plan_; }
  void set_plan (ACM_CodePlan* arg) { set_xml_dirty (true); plan_ = arg; if (arg) arg->set_parent(this); has_plan_ = true; }
  ACM_CodePlan* get_plan() { return plan_; }

};

class ACM_Access  : public ACM_Code {
  int lid_;
  bool has_target_object_lid_;
  int target_object_lid_;
  bool has_cfg_block_lid_;
  int cfg_block_lid_;
  bool has_target_class_;
  ACM_Class* target_class_;
public:
  ACM_Access () {
  has_target_object_lid_ = false;
  has_cfg_block_lid_ = false;
    has_target_class_ = false;
  }
  virtual ~ACM_Access () {}
  virtual const char *type_str () const { return "Access"; }
  virtual JoinPointType type_val () const { return JPT_Access; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_lid (int arg) { set_xml_dirty (true); lid_ = arg; }
  int get_lid() const { return lid_; }

  bool has_target_object_lid () const { return has_target_object_lid_; }
  void set_target_object_lid (int arg) { set_xml_dirty (true); target_object_lid_ = arg; has_target_object_lid_ = true; }
  int get_target_object_lid() const { return target_object_lid_; }

  bool has_cfg_block_lid () const { return has_cfg_block_lid_; }
  void set_cfg_block_lid (int arg) { set_xml_dirty (true); cfg_block_lid_ = arg; has_cfg_block_lid_ = true; }
  int get_cfg_block_lid() const { return cfg_block_lid_; }

  bool has_target_class () const { return has_target_class_; }
  void set_target_class (ACM_Class* arg) { set_xml_dirty (true); target_class_ = arg; has_target_class_ = true; }
  ACM_Class* get_target_class() { return target_class_; }

};

class ACM_Get  : public ACM_Access {
  bool has_variable_;
  ACM_Variable* variable_;
public:
  ACM_Get () {
    has_variable_ = false;
  }
  virtual ~ACM_Get () {}
  virtual const char *type_str () const { return "Get"; }
  virtual JoinPointType type_val () const { return JPT_Get; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_variable () const { return has_variable_; }
  void set_variable (ACM_Variable* arg) { set_xml_dirty (true); variable_ = arg; has_variable_ = true; }
  ACM_Variable* get_variable() { return variable_; }

};

class ACM_Set  : public ACM_Access {
  bool has_variable_;
  ACM_Variable* variable_;
public:
  ACM_Set () {
    has_variable_ = false;
  }
  virtual ~ACM_Set () {}
  virtual const char *type_str () const { return "Set"; }
  virtual JoinPointType type_val () const { return JPT_Set; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_variable () const { return has_variable_; }
  void set_variable (ACM_Variable* arg) { set_xml_dirty (true); variable_ = arg; has_variable_ = true; }
  ACM_Variable* get_variable() { return variable_; }

};

class ACM_Call  : public ACM_Access {
  ACM_Function* target_;
  ACM_Container<ACM_Type, true> variadic_arg_types_;
  bool has_default_args_;
  int default_args_;
public:
  ACM_Call () {
    variadic_arg_types_.set_parent (this);
  has_default_args_ = false;
  }
  virtual ~ACM_Call () {}
  virtual const char *type_str () const { return "Call"; }
  virtual JoinPointType type_val () const { return JPT_Call; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_target (ACM_Function* arg) { set_xml_dirty (true); target_ = arg; }
  ACM_Function* get_target() { return target_; }

  ACM_Container<ACM_Type, true> &get_variadic_arg_types() { return variadic_arg_types_; }
  const ACM_Container<ACM_Type, true> &get_variadic_arg_types() const { return variadic_arg_types_; }

  bool has_default_args () const { return has_default_args_; }
  void set_default_args (int arg) { set_xml_dirty (true); default_args_ = arg; has_default_args_ = true; }
  int get_default_args() const { return default_args_; }

};

class ACM_Builtin  : public ACM_Access {
  ACM_Function* target_;
  ACM_Container<ACM_Access, true> implicit_access_;
public:
  ACM_Builtin () {
    implicit_access_.set_parent (this);
  }
  virtual ~ACM_Builtin () {}
  virtual const char *type_str () const { return "Builtin"; }
  virtual JoinPointType type_val () const { return JPT_Builtin; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_target (ACM_Function* arg) { set_xml_dirty (true); target_ = arg; }
  ACM_Function* get_target() { return target_; }

  ACM_Container<ACM_Access, true> &get_implicit_access() { return implicit_access_; }
  const ACM_Container<ACM_Access, true> &get_implicit_access() const { return implicit_access_; }

};

class ACM_Ref  : public ACM_Access {
  ACM_Variable* variable_;
  ACM_Type* type_;
public:
  ACM_Ref () {
  }
  virtual ~ACM_Ref () {}
  virtual const char *type_str () const { return "Ref"; }
  virtual JoinPointType type_val () const { return JPT_Ref; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_variable (ACM_Variable* arg) { set_xml_dirty (true); variable_ = arg; }
  ACM_Variable* get_variable() { return variable_; }

  void set_type (ACM_Type* arg) { set_xml_dirty (true); type_ = arg; if (arg) arg->set_parent(this); }
  ACM_Type* get_type() { return type_; }

};

class ACM_GetRef  : public ACM_Access {
  ACM_Type* type_;
public:
  ACM_GetRef () {
  }
  virtual ~ACM_GetRef () {}
  virtual const char *type_str () const { return "GetRef"; }
  virtual JoinPointType type_val () const { return JPT_GetRef; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_type (ACM_Type* arg) { set_xml_dirty (true); type_ = arg; if (arg) arg->set_parent(this); }
  ACM_Type* get_type() { return type_; }

};

class ACM_SetRef  : public ACM_Access {
  ACM_Type* type_;
public:
  ACM_SetRef () {
  }
  virtual ~ACM_SetRef () {}
  virtual const char *type_str () const { return "SetRef"; }
  virtual JoinPointType type_val () const { return JPT_SetRef; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_type (ACM_Type* arg) { set_xml_dirty (true); type_ = arg; if (arg) arg->set_parent(this); }
  ACM_Type* get_type() { return type_; }

};

class ACM_CallRef  : public ACM_Access {
  ACM_Type* type_;
public:
  ACM_CallRef () {
  }
  virtual ~ACM_CallRef () {}
  virtual const char *type_str () const { return "CallRef"; }
  virtual JoinPointType type_val () const { return JPT_CallRef; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_type (ACM_Type* arg) { set_xml_dirty (true); type_ = arg; if (arg) arg->set_parent(this); }
  ACM_Type* get_type() { return type_; }

};

class ACM_Execution  : public ACM_Code {
public:
  ACM_Execution () {
  }
  virtual ~ACM_Execution () {}
  virtual const char *type_str () const { return "Execution"; }
  virtual JoinPointType type_val () const { return JPT_Execution; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

};

class ACM_Construction  : public ACM_Code {
public:
  ACM_Construction () {
  }
  virtual ~ACM_Construction () {}
  virtual const char *type_str () const { return "Construction"; }
  virtual JoinPointType type_val () const { return JPT_Construction; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

};

class ACM_Destruction  : public ACM_Code {
public:
  ACM_Destruction () {
  }
  virtual ~ACM_Destruction () {}
  virtual const char *type_str () const { return "Destruction"; }
  virtual JoinPointType type_val () const { return JPT_Destruction; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

};

class ACM_Pointcut  : public ACM_Name {
  string expr_;
  ACM_Container<ACM_Arg, true> args_;
  PointcutType kind_;
  PointcutExpression parsed_expr_;
public:
  ACM_Pointcut () {
    args_.set_parent (this);
  }
  virtual ~ACM_Pointcut () {}
  virtual const char *type_str () const { return "Pointcut"; }
  virtual JoinPointType type_val () const { return JPT_Pointcut; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_expr (string arg) { set_xml_dirty (true); expr_ = arg; }
  string get_expr() const { return expr_; }

  ACM_Container<ACM_Arg, true> &get_args() { return args_; }
  const ACM_Container<ACM_Arg, true> &get_args() const { return args_; }

  void set_kind (PointcutType arg) { set_xml_dirty (true); kind_ = arg; }
  PointcutType get_kind() const { return kind_; }

  PointcutExpression &get_parsed_expr() { return parsed_expr_; }
  const PointcutExpression &get_parsed_expr() const { return parsed_expr_; }

};

class ACM_Attribute  : public ACM_Name {
  ACM_Container<ACM_Arg, true> args_;
public:
  ACM_Attribute () {
    args_.set_parent (this);
  }
  virtual ~ACM_Attribute () {}
  virtual const char *type_str () const { return "Attribute"; }
  virtual JoinPointType type_val () const { return JPT_Attribute; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_Arg, true> &get_args() { return args_; }
  const ACM_Container<ACM_Arg, true> &get_args() const { return args_; }

};

class ACM_Advice  : public ACM_Any {
  ACM_Pointcut* pointcut_;
  int lid_;
public:
  ACM_Advice () {
  }
  virtual ~ACM_Advice () {}
  virtual const char *type_str () const { return "Advice"; }
  virtual JoinPointType type_val () const { return JPT_Advice; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_pointcut (ACM_Pointcut* arg) { set_xml_dirty (true); pointcut_ = arg; if (arg) arg->set_parent(this); }
  ACM_Pointcut* get_pointcut() { return pointcut_; }

  void set_lid (int arg) { set_xml_dirty (true); lid_ = arg; }
  int get_lid() const { return lid_; }

};

class ACM_AdviceCode  : public ACM_Advice {
  AdviceCodeType kind_;
  AdviceCodeContext context_;
public:
  ACM_AdviceCode () {
  }
  virtual ~ACM_AdviceCode () {}
  virtual const char *type_str () const { return "AdviceCode"; }
  virtual JoinPointType type_val () const { return JPT_AdviceCode; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_kind (AdviceCodeType arg) { set_xml_dirty (true); kind_ = arg; }
  AdviceCodeType get_kind() const { return kind_; }

  void set_context (AdviceCodeContext arg) { set_xml_dirty (true); context_ = arg; }
  AdviceCodeContext get_context() const { return context_; }

};

class ACM_Introduction  : public ACM_Advice {
  bool has_named_slice_;
  ACM_ClassSlice* named_slice_;
  bool has_anon_slice_;
  ACM_ClassSlice* anon_slice_;
public:
  ACM_Introduction () {
    has_named_slice_ = false;
    has_anon_slice_ = false;
  }
  virtual ~ACM_Introduction () {}
  virtual const char *type_str () const { return "Introduction"; }
  virtual JoinPointType type_val () const { return JPT_Introduction; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  bool has_named_slice () const { return has_named_slice_; }
  void set_named_slice (ACM_ClassSlice* arg) { set_xml_dirty (true); named_slice_ = arg; has_named_slice_ = true; }
  ACM_ClassSlice* get_named_slice() { return named_slice_; }

  bool has_anon_slice () const { return has_anon_slice_; }
  void set_anon_slice (ACM_ClassSlice* arg) { set_xml_dirty (true); anon_slice_ = arg; if (arg) arg->set_parent(this); has_anon_slice_ = true; }
  ACM_ClassSlice* get_anon_slice() { return anon_slice_; }

};

class ACM_Order  : public ACM_Advice {
  ACM_Container<ACM_Pointcut, true> aspect_exprs_;
public:
  ACM_Order () {
    aspect_exprs_.set_parent (this);
  }
  virtual ~ACM_Order () {}
  virtual const char *type_str () const { return "Order"; }
  virtual JoinPointType type_val () const { return JPT_Order; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_Pointcut, true> &get_aspect_exprs() { return aspect_exprs_; }
  const ACM_Container<ACM_Pointcut, true> &get_aspect_exprs() const { return aspect_exprs_; }

};

class ACM_Aspect  : public ACM_Class {
  ACM_Container<ACM_Introduction, true> intros_;
  ACM_Container<ACM_AdviceCode, true> advices_;
  ACM_Container<ACM_Order, true> orders_;
public:
  ACM_Aspect () {
    intros_.set_parent (this);
    advices_.set_parent (this);
    orders_.set_parent (this);
  }
  virtual ~ACM_Aspect () {}
  virtual const char *type_str () const { return "Aspect"; }
  virtual JoinPointType type_val () const { return JPT_Aspect; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_Introduction, true> &get_intros() { return intros_; }
  const ACM_Container<ACM_Introduction, true> &get_intros() const { return intros_; }

  ACM_Container<ACM_AdviceCode, true> &get_advices() { return advices_; }
  const ACM_Container<ACM_AdviceCode, true> &get_advices() const { return advices_; }

  ACM_Container<ACM_Order, true> &get_orders() { return orders_; }
  const ACM_Container<ACM_Order, true> &get_orders() const { return orders_; }

};

class ACM_Speculation  : public ACM_Node {
  ACM_Advice* advice_;
  ACM_TUnit* tunit_;
  ACM_Container<ACM_Any, false> true_jps_;
  ACM_Container<ACM_Any, false> false_jps_;
  ACM_Container<ACM_Any, false> conditional_jps_;
public:
  ACM_Speculation () {
  }
  virtual ~ACM_Speculation () {}
  virtual const char *type_str () const { return "Speculation"; }
  virtual JoinPointType type_val () const { return JPT_Speculation; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  void set_advice (ACM_Advice* arg) { set_xml_dirty (true); advice_ = arg; }
  ACM_Advice* get_advice() { return advice_; }

  void set_tunit (ACM_TUnit* arg) { set_xml_dirty (true); tunit_ = arg; }
  ACM_TUnit* get_tunit() { return tunit_; }

  ACM_Container<ACM_Any, false> &get_true_jps() { return true_jps_; }
  const ACM_Container<ACM_Any, false> &get_true_jps() const { return true_jps_; }

  ACM_Container<ACM_Any, false> &get_false_jps() { return false_jps_; }
  const ACM_Container<ACM_Any, false> &get_false_jps() const { return false_jps_; }

  ACM_Container<ACM_Any, false> &get_conditional_jps() { return conditional_jps_; }
  const ACM_Container<ACM_Any, false> &get_conditional_jps() const { return conditional_jps_; }

};

class ACM_Model  : public ACM_Node {
  ACM_Container<ACM_File, true> files_;
  ACM_Namespace* root_;
  ACM_Container<ACM_Speculation, true> speculations_;
  string version_;
public:
  ACM_Model () {
    files_.set_parent (this);
    speculations_.set_parent (this);
  }
  virtual ~ACM_Model () {}
  virtual const char *type_str () const { return "Model"; }
  virtual JoinPointType type_val () const { return JPT_Model; }
  virtual std::string key () const;
  
  // dump tree functions
  virtual void dump (std::ostream &out = cout, int indent = 0) const;
  void dump_attr (std::ostream &out = cout) const;
  void dump_refs (std::ostream &out = cout) const;
  void dump_aggr (std::ostream &out = cout, int indent = 0) const;

  
  // XML functions
  virtual void to_xml(SaveContext &);

  ACM_Container<ACM_File, true> &get_files() { return files_; }
  const ACM_Container<ACM_File, true> &get_files() const { return files_; }

  void set_root (ACM_Namespace* arg) { set_xml_dirty (true); root_ = arg; if (arg) arg->set_parent(this); }
  ACM_Namespace* get_root() { return root_; }

  ACM_Container<ACM_Speculation, true> &get_speculations() { return speculations_; }
  const ACM_Container<ACM_Speculation, true> &get_speculations() const { return speculations_; }

  void set_version (string arg) { set_xml_dirty (true); version_ = arg; }
  string get_version() const { return version_; }

};

// model facade class
class ProjectModel : public ACM_Model {
  typedef std::list<ModelNode*> NodeList; 
  NodeList _join_points[MAX_JOIN_POINT_TYPE_INDEX];
  xmlDocPtr _xml_doc;

  typedef std::set<ACM_File*> FileSet;
  FileSet _modified_files;
  int _merge_id;
  unsigned _jpids_in_use;
  std::set<unsigned> _jpids_freed;

  // delete all source entries that are referencing a modified file
  // true: *all* entries have been deleted
  bool purgeSources(ACM_Any &me);

  template <typename Container>
  bool sourceCleanupContainer(Container &container) {
    typedef typename Container::Type T;
    list<T*> found;
    for (typename Container::iterator i = container.begin ();
        i != container.end (); ++i) {
      int mc = (*i)->get_merge_count ();
      if (mc == _merge_id) {
        if ((*i)->is_deleted ())
          found.push_back (*i);
      }
      else {
        (*i)->set_merge_count (_merge_id);
        if (source_cleanup (**i)) {
          found.push_back (*i);
          unlink_node (*i);
        }
      }
    }
    for (typename list<T*>::iterator i = found.begin (); i != found.end (); ++i)
      container.remove (*i);
    return false;
  }

  bool source_cleanup(ACM_Node &node);
    
protected:
  template <typename T> T *remember (T* elem_ptr, int index) {
    _join_points[index].push_back (elem_ptr);
    return elem_ptr;
  }

  void unlink_node (ACM_Node *node) {
    if (node->get_xml_node ()) {
      xmlUnlinkNode (node->get_xml_node ());
      xmlFreeNode (node->get_xml_node ());
      node->set_xml_node (0);
      node->set_xml_dirty ();
    }
    node->is_deleted (true);
  }

public:
  // constructor: initialize attributes
  ProjectModel () : _xml_doc (0), _merge_id (0), _jpids_in_use (0) {}

  // destructor: delete all registered model elements and the XML DOM
  ~ProjectModel () { clear (); }
  // delete all model elements
  void clear ();

  void xml_doc (xmlDocPtr doc_ptr) { _xml_doc = doc_ptr; }
  xmlDocPtr xml_doc () const { return _xml_doc; }
  
  // select and return all join points of a given type (any of them)
  typedef std::list<ModelNode*> Selection; 
  void select (JoinPointType jpt, Selection &result, bool prj_only = true);

  // allocated a new joinpoint ID
  unsigned alloc_jpid ();
  
  // mark a join point ID as allocated (needed for loading models)
  void alloc_jpid (unsigned jpid);

  // free a joinpoint ID
  void free_jpid (unsigned jpid);
  
  // merge the elements of another model into this one
  void merge (ProjectModel &that);
  
  // before merging two models find out which files have been changed
  // (file in 'that' newer as in 'this')
  void init_change_set(ProjectModel &that);
  
  // cleanup list of modified files (at the end of merging)
  void clear_change_set();

  // merge a tunit node into a similar project mode node
  void merge_node (ACM_Node *prj_node, ACM_Node *tu_node);
 
  // copy a node of another model into this model (deep copy)
  ACM_Node *copyNode(ACM_Node *that_node);

  // fix all node references; this is the second phase of merging a tunit
  // model into a project model. Reference still address the tunit model
  // nodes. Now they have to be replaced with the corresponding project
  // model node references
  void fix_refs(ACM_Node *me);
  
  // helper function for merging two models
  template <typename Container>
  void merge_container (Container &that, Container &into, bool refs = false) {
    typename Container::Set &me  = into.get_sorted ();
    typename Container::Set &you = that.get_sorted ();
    // iterate over both sorted lists
    typename Container::Set::iterator curr_me = me.begin ();
    typename Container::Set::iterator curr_you = you.begin ();
    while (curr_me != me.end () && curr_you != you.end ()) {
      if (**curr_me < ** curr_you) // an element in 'me', which is not present
        ++curr_me;                 // in 'you' => simply skip
      else if (**curr_you < ** curr_me) { // a new element (not in 'me' yet
        into.insert ((typename Container::Type*)(refs ? *curr_you : copyNode (*curr_you)));
        ++curr_you;
      }
      else { // two equal elements according to their key => merge
        if (!refs) merge_node (*curr_me, *curr_you);
        ++curr_you;
        ++curr_me;
      }
    }
    while (curr_you != you.end ()) {
      into.insert ((typename Container::Type*)(refs ? *curr_you : copyNode (*curr_you)));
      ++curr_you;      
    }
  }

  // handle attribute variations when merging models
  bool merge_attr (ACM_Node &me, ACM_Node &that, const std::string &attr_name);
  void merge_attr_only_in_me (ACM_Node &me, ACM_Node &that, const std::string &attr_name);
  void merge_attr_only_in_that (ACM_Node &me, ACM_Node &that, const std::string &attr_name);

  // factory methods that remember all objects of this model
  virtual ACM_TUnit *newTUnit() { return remember (new ACM_TUnit, JPTI_TUnit); }
  virtual ACM_Header *newHeader() { return remember (new ACM_Header, JPTI_Header); }
  virtual ACM_Source *newSource() { return remember (new ACM_Source, JPTI_Source); }
  virtual ACM_MemberIntro *newMemberIntro() { return remember (new ACM_MemberIntro, JPTI_MemberIntro); }
  virtual ACM_BaseIntro *newBaseIntro() { return remember (new ACM_BaseIntro, JPTI_BaseIntro); }
  virtual ACM_ClassPlan *newClassPlan() { return remember (new ACM_ClassPlan, JPTI_ClassPlan); }
  virtual ACM_CodeAdvice *newCodeAdvice() { return remember (new ACM_CodeAdvice, JPTI_CodeAdvice); }
  virtual ACM_CodePlan *newCodePlan() { return remember (new ACM_CodePlan, JPTI_CodePlan); }
  virtual ACM_Type *newType() { return remember (new ACM_Type, JPTI_Type); }
  virtual ACM_Arg *newArg() { return remember (new ACM_Arg, JPTI_Arg); }
  virtual ACM_Namespace *newNamespace() { return remember (new ACM_Namespace, JPTI_Namespace); }
  virtual ACM_Function *newFunction() { return remember (new ACM_Function, JPTI_Function); }
  virtual ACM_Variable *newVariable() { return remember (new ACM_Variable, JPTI_Variable); }
  virtual ACM_Class *newClass() { return remember (new ACM_Class, JPTI_Class); }
  virtual ACM_ClassSlice *newClassSlice() { return remember (new ACM_ClassSlice, JPTI_ClassSlice); }
  virtual ACM_Get *newGet() { return remember (new ACM_Get, JPTI_Get); }
  virtual ACM_Set *newSet() { return remember (new ACM_Set, JPTI_Set); }
  virtual ACM_Call *newCall() { return remember (new ACM_Call, JPTI_Call); }
  virtual ACM_Builtin *newBuiltin() { return remember (new ACM_Builtin, JPTI_Builtin); }
  virtual ACM_Ref *newRef() { return remember (new ACM_Ref, JPTI_Ref); }
  virtual ACM_GetRef *newGetRef() { return remember (new ACM_GetRef, JPTI_GetRef); }
  virtual ACM_SetRef *newSetRef() { return remember (new ACM_SetRef, JPTI_SetRef); }
  virtual ACM_CallRef *newCallRef() { return remember (new ACM_CallRef, JPTI_CallRef); }
  virtual ACM_Execution *newExecution() { return remember (new ACM_Execution, JPTI_Execution); }
  virtual ACM_Construction *newConstruction() { return remember (new ACM_Construction, JPTI_Construction); }
  virtual ACM_Destruction *newDestruction() { return remember (new ACM_Destruction, JPTI_Destruction); }
  virtual ACM_Pointcut *newPointcut() { return remember (new ACM_Pointcut, JPTI_Pointcut); }
  virtual ACM_Attribute *newAttribute() { return remember (new ACM_Attribute, JPTI_Attribute); }
  virtual ACM_AdviceCode *newAdviceCode() { return remember (new ACM_AdviceCode, JPTI_AdviceCode); }
  virtual ACM_Introduction *newIntroduction() { return remember (new ACM_Introduction, JPTI_Introduction); }
  virtual ACM_Order *newOrder() { return remember (new ACM_Order, JPTI_Order); }
  virtual ACM_Aspect *newAspect() { return remember (new ACM_Aspect, JPTI_Aspect); }
  virtual ACM_Speculation *newSpeculation() { return remember (new ACM_Speculation, JPTI_Speculation); }
  virtual ACM_Model *newModel() { return remember (new ACM_Model, JPTI_Model); }

  // copy methods that copy elements from other models into this one
  // NOTE: the elements are only copied and registerd, but no link is set!
  virtual void copyNode(ACM_Node &me, ACM_Node &that);
  virtual void copyFile(ACM_File &me, ACM_File &that);
  virtual void copyTUnit(ACM_TUnit &me, ACM_TUnit &that);
  virtual void copyHeader(ACM_Header &me, ACM_Header &that);
  virtual void copySource(ACM_Source &me, ACM_Source &that);
  virtual void copyMemberIntro(ACM_MemberIntro &me, ACM_MemberIntro &that);
  virtual void copyBaseIntro(ACM_BaseIntro &me, ACM_BaseIntro &that);
  virtual void copyClassPlan(ACM_ClassPlan &me, ACM_ClassPlan &that);
  virtual void copyCodeAdvice(ACM_CodeAdvice &me, ACM_CodeAdvice &that);
  virtual void copyCodePlan(ACM_CodePlan &me, ACM_CodePlan &that);
  virtual void copyAny(ACM_Any &me, ACM_Any &that);
  virtual void copyType(ACM_Type &me, ACM_Type &that);
  virtual void copyArg(ACM_Arg &me, ACM_Arg &that);
  virtual void copyName(ACM_Name &me, ACM_Name &that);
  virtual void copyNamespace(ACM_Namespace &me, ACM_Namespace &that);
  virtual void copyFunction(ACM_Function &me, ACM_Function &that);
  virtual void copyVariable(ACM_Variable &me, ACM_Variable &that);
  virtual void copyClass(ACM_Class &me, ACM_Class &that);
  virtual void copyClassSlice(ACM_ClassSlice &me, ACM_ClassSlice &that);
  virtual void copyCode(ACM_Code &me, ACM_Code &that);
  virtual void copyAccess(ACM_Access &me, ACM_Access &that);
  virtual void copyGet(ACM_Get &me, ACM_Get &that);
  virtual void copySet(ACM_Set &me, ACM_Set &that);
  virtual void copyCall(ACM_Call &me, ACM_Call &that);
  virtual void copyBuiltin(ACM_Builtin &me, ACM_Builtin &that);
  virtual void copyRef(ACM_Ref &me, ACM_Ref &that);
  virtual void copyGetRef(ACM_GetRef &me, ACM_GetRef &that);
  virtual void copySetRef(ACM_SetRef &me, ACM_SetRef &that);
  virtual void copyCallRef(ACM_CallRef &me, ACM_CallRef &that);
  virtual void copyExecution(ACM_Execution &me, ACM_Execution &that);
  virtual void copyConstruction(ACM_Construction &me, ACM_Construction &that);
  virtual void copyDestruction(ACM_Destruction &me, ACM_Destruction &that);
  virtual void copyPointcut(ACM_Pointcut &me, ACM_Pointcut &that);
  virtual void copyAttribute(ACM_Attribute &me, ACM_Attribute &that);
  virtual void copyAdvice(ACM_Advice &me, ACM_Advice &that);
  virtual void copyAdviceCode(ACM_AdviceCode &me, ACM_AdviceCode &that);
  virtual void copyIntroduction(ACM_Introduction &me, ACM_Introduction &that);
  virtual void copyOrder(ACM_Order &me, ACM_Order &that);
  virtual void copyAspect(ACM_Aspect &me, ACM_Aspect &that);
  virtual void copySpeculation(ACM_Speculation &me, ACM_Speculation &that);
  virtual void copyModel(ACM_Model &me, ACM_Model &that);

  // merge methods that are called for corresponding translation unit model
  // and project model nodes. The functions call merge or copy functions on
  // their aggregated sub-nodes.
  virtual bool sourceCleanupNode(ACM_Node &me);
  virtual bool sourceCleanupFile(ACM_File &me);
  virtual bool sourceCleanupTUnit(ACM_TUnit &me);
  virtual bool sourceCleanupHeader(ACM_Header &me);
  virtual bool sourceCleanupSource(ACM_Source &me);
  virtual bool sourceCleanupMemberIntro(ACM_MemberIntro &me);
  virtual bool sourceCleanupBaseIntro(ACM_BaseIntro &me);
  virtual bool sourceCleanupClassPlan(ACM_ClassPlan &me);
  virtual bool sourceCleanupCodeAdvice(ACM_CodeAdvice &me);
  virtual bool sourceCleanupCodePlan(ACM_CodePlan &me);
  virtual bool sourceCleanupAny(ACM_Any &me);
  virtual bool sourceCleanupType(ACM_Type &me);
  virtual bool sourceCleanupArg(ACM_Arg &me);
  virtual bool sourceCleanupName(ACM_Name &me);
  virtual bool sourceCleanupNamespace(ACM_Namespace &me);
  virtual bool sourceCleanupFunction(ACM_Function &me);
  virtual bool sourceCleanupVariable(ACM_Variable &me);
  virtual bool sourceCleanupClass(ACM_Class &me);
  virtual bool sourceCleanupClassSlice(ACM_ClassSlice &me);
  virtual bool sourceCleanupCode(ACM_Code &me);
  virtual bool sourceCleanupAccess(ACM_Access &me);
  virtual bool sourceCleanupGet(ACM_Get &me);
  virtual bool sourceCleanupSet(ACM_Set &me);
  virtual bool sourceCleanupCall(ACM_Call &me);
  virtual bool sourceCleanupBuiltin(ACM_Builtin &me);
  virtual bool sourceCleanupRef(ACM_Ref &me);
  virtual bool sourceCleanupGetRef(ACM_GetRef &me);
  virtual bool sourceCleanupSetRef(ACM_SetRef &me);
  virtual bool sourceCleanupCallRef(ACM_CallRef &me);
  virtual bool sourceCleanupExecution(ACM_Execution &me);
  virtual bool sourceCleanupConstruction(ACM_Construction &me);
  virtual bool sourceCleanupDestruction(ACM_Destruction &me);
  virtual bool sourceCleanupPointcut(ACM_Pointcut &me);
  virtual bool sourceCleanupAttribute(ACM_Attribute &me);
  virtual bool sourceCleanupAdvice(ACM_Advice &me);
  virtual bool sourceCleanupAdviceCode(ACM_AdviceCode &me);
  virtual bool sourceCleanupIntroduction(ACM_Introduction &me);
  virtual bool sourceCleanupOrder(ACM_Order &me);
  virtual bool sourceCleanupAspect(ACM_Aspect &me);
  virtual bool sourceCleanupSpeculation(ACM_Speculation &me);
  virtual bool sourceCleanupModel(ACM_Model &me);

  virtual void mergeNode(ACM_Node &me, ACM_Node &that);
  virtual void mergeFile(ACM_File &me, ACM_File &that);
  virtual void mergeTUnit(ACM_TUnit &me, ACM_TUnit &that);
  virtual void mergeHeader(ACM_Header &me, ACM_Header &that);
  virtual void mergeSource(ACM_Source &me, ACM_Source &that);
  virtual void mergeMemberIntro(ACM_MemberIntro &me, ACM_MemberIntro &that);
  virtual void mergeBaseIntro(ACM_BaseIntro &me, ACM_BaseIntro &that);
  virtual void mergeClassPlan(ACM_ClassPlan &me, ACM_ClassPlan &that);
  virtual void mergeCodeAdvice(ACM_CodeAdvice &me, ACM_CodeAdvice &that);
  virtual void mergeCodePlan(ACM_CodePlan &me, ACM_CodePlan &that);
  virtual void mergeAny(ACM_Any &me, ACM_Any &that);
  virtual void mergeType(ACM_Type &me, ACM_Type &that);
  virtual void mergeArg(ACM_Arg &me, ACM_Arg &that);
  virtual void mergeName(ACM_Name &me, ACM_Name &that);
  virtual void mergeNamespace(ACM_Namespace &me, ACM_Namespace &that);
  virtual void mergeFunction(ACM_Function &me, ACM_Function &that);
  virtual void mergeVariable(ACM_Variable &me, ACM_Variable &that);
  virtual void mergeClass(ACM_Class &me, ACM_Class &that);
  virtual void mergeClassSlice(ACM_ClassSlice &me, ACM_ClassSlice &that);
  virtual void mergeCode(ACM_Code &me, ACM_Code &that);
  virtual void mergeAccess(ACM_Access &me, ACM_Access &that);
  virtual void mergeGet(ACM_Get &me, ACM_Get &that);
  virtual void mergeSet(ACM_Set &me, ACM_Set &that);
  virtual void mergeCall(ACM_Call &me, ACM_Call &that);
  virtual void mergeBuiltin(ACM_Builtin &me, ACM_Builtin &that);
  virtual void mergeRef(ACM_Ref &me, ACM_Ref &that);
  virtual void mergeGetRef(ACM_GetRef &me, ACM_GetRef &that);
  virtual void mergeSetRef(ACM_SetRef &me, ACM_SetRef &that);
  virtual void mergeCallRef(ACM_CallRef &me, ACM_CallRef &that);
  virtual void mergeExecution(ACM_Execution &me, ACM_Execution &that);
  virtual void mergeConstruction(ACM_Construction &me, ACM_Construction &that);
  virtual void mergeDestruction(ACM_Destruction &me, ACM_Destruction &that);
  virtual void mergePointcut(ACM_Pointcut &me, ACM_Pointcut &that);
  virtual void mergeAttribute(ACM_Attribute &me, ACM_Attribute &that);
  virtual void mergeAdvice(ACM_Advice &me, ACM_Advice &that);
  virtual void mergeAdviceCode(ACM_AdviceCode &me, ACM_AdviceCode &that);
  virtual void mergeIntroduction(ACM_Introduction &me, ACM_Introduction &that);
  virtual void mergeOrder(ACM_Order &me, ACM_Order &that);
  virtual void mergeAspect(ACM_Aspect &me, ACM_Aspect &that);
  virtual void mergeSpeculation(ACM_Speculation &me, ACM_Speculation &that);
  virtual void mergeModel(ACM_Model &me, ACM_Model &that);

  virtual void fix_refs_Node(ACM_Node &me);
  virtual void fix_refs_File(ACM_File &me);
  virtual void fix_refs_TUnit(ACM_TUnit &me);
  virtual void fix_refs_Header(ACM_Header &me);
  virtual void fix_refs_Source(ACM_Source &me);
  virtual void fix_refs_MemberIntro(ACM_MemberIntro &me);
  virtual void fix_refs_BaseIntro(ACM_BaseIntro &me);
  virtual void fix_refs_ClassPlan(ACM_ClassPlan &me);
  virtual void fix_refs_CodeAdvice(ACM_CodeAdvice &me);
  virtual void fix_refs_CodePlan(ACM_CodePlan &me);
  virtual void fix_refs_Any(ACM_Any &me);
  virtual void fix_refs_Type(ACM_Type &me);
  virtual void fix_refs_Arg(ACM_Arg &me);
  virtual void fix_refs_Name(ACM_Name &me);
  virtual void fix_refs_Namespace(ACM_Namespace &me);
  virtual void fix_refs_Function(ACM_Function &me);
  virtual void fix_refs_Variable(ACM_Variable &me);
  virtual void fix_refs_Class(ACM_Class &me);
  virtual void fix_refs_ClassSlice(ACM_ClassSlice &me);
  virtual void fix_refs_Code(ACM_Code &me);
  virtual void fix_refs_Access(ACM_Access &me);
  virtual void fix_refs_Get(ACM_Get &me);
  virtual void fix_refs_Set(ACM_Set &me);
  virtual void fix_refs_Call(ACM_Call &me);
  virtual void fix_refs_Builtin(ACM_Builtin &me);
  virtual void fix_refs_Ref(ACM_Ref &me);
  virtual void fix_refs_GetRef(ACM_GetRef &me);
  virtual void fix_refs_SetRef(ACM_SetRef &me);
  virtual void fix_refs_CallRef(ACM_CallRef &me);
  virtual void fix_refs_Execution(ACM_Execution &me);
  virtual void fix_refs_Construction(ACM_Construction &me);
  virtual void fix_refs_Destruction(ACM_Destruction &me);
  virtual void fix_refs_Pointcut(ACM_Pointcut &me);
  virtual void fix_refs_Attribute(ACM_Attribute &me);
  virtual void fix_refs_Advice(ACM_Advice &me);
  virtual void fix_refs_AdviceCode(ACM_AdviceCode &me);
  virtual void fix_refs_Introduction(ACM_Introduction &me);
  virtual void fix_refs_Order(ACM_Order &me);
  virtual void fix_refs_Aspect(ACM_Aspect &me);
  virtual void fix_refs_Speculation(ACM_Speculation &me);
  virtual void fix_refs_Model(ACM_Model &me);
};

#endif // __Elements_h__
