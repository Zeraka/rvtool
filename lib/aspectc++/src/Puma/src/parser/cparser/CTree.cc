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

#include "Puma/CTree.h"
#include "Puma/CConstant.h"
#include "Puma/GnuCTree.h"
#include "Puma/WinCTree.h"
#include "Puma/DelayedParseTree.h"
#include "Puma/CTokens.h"
#include "Puma/CTypeInfo.h"
#include "Puma/CObjectInfo.h"

#include <sstream>            // ostringstream
#include <iostream>           // ostream
#include <iomanip>            // hex, setfill, setw

namespace Puma {


int CTree::alloc = 0;
int CTree::release = 0;

CTree *CTree::Son (CTree * const *sons, int max, int num) const {
  int cnt = 0;
  for (int i = 0; i < max; i++) {
    if (sons[i] == (CTree*)0)
      continue;
    if (cnt == num)
      return sons[i];
    cnt++;
  }
  return (CTree*)0;
}

int CTree::Sons (CTree * const *sons, int max) const {
  int cnt = 0;
  for (int i = 0; i < max; i++) {
    if (sons[i] == (CTree*)0)
      continue;
    cnt++;
  }
  return cnt;
}

void CTree::ReplaceSon (CTree **sons, int max, CTree *old_son, CTree *new_son) {
  for (int i = 0; i < max; i++) {
    if (sons[i] == old_son) {
      sons[i] = new_son;
      if (new_son) SetParent (new_son, this);
    }
  }
}

void CTree::ReplaceSon (CTree *&son, CTree *old_son, CTree *new_son) {
  if (old_son == son) {
    son = new_son;
    if (new_son) SetParent (new_son, this);
  }
}

void CTree::AddSon (CTree *&son, CTree *new_son) {
  son = new_son;
  if (new_son) SetParent (new_son, this);
}

// find the start token
Token* CTree::token () const {
  CTree *son;
  Token *tok;
  unsigned sons = Sons ();

  for (unsigned i = 0; i < sons; i++) {
    son = Son (i);
    tok = son ? son->token () : (Token*)0;
    if (tok)
      return tok;
  }

  return (Token*)0;
}

// find the end token
Token* CTree::end_token () const {
  CTree *son;
  Token *tok;
  unsigned sons = Sons ();

  for (unsigned i = sons; i > 0; i--) {
    son = Son (i-1);
    tok = son ? son->end_token () : (Token*)0;
    if (tok)
      return tok;
  }

  return (Token*)0;
}

// find the start token node
CT_Token *CTree::token_node () const {
  CTree *son;
  CT_Token *tok;
  unsigned sons = Sons ();

  for (unsigned i = 0; i < sons; i++) {
    son = Son (i);
    tok = son ? son->token_node () : (CT_Token*)0;
    if (tok)
      return tok;
  }

  return (CT_Token*)0;
}

// find the end token node
CT_Token* CTree::end_token_node () const {
  CTree *son;
  CT_Token *tok;
  unsigned sons = Sons ();

  for (unsigned i = sons; i > 0; i--) {
    son = Son (i-1);
    tok = son ? son->end_token_node () : (CT_Token*)0;
    if (tok)
      return tok;
  }

  return (CT_Token*)0;
}

/** Return true if the tree has the constant value 0.
 *  \return True if constant value is 0. */
bool CTree::HasValueNull () const {
  CExprValue* v = Value();
  if (v) {
    CConstant* c = v->Constant();
    return c && c->isNull();
  }
  return false;
}

int CT_List::Entries () const {
  if (_properties & FORCE_EMPTY) return 0;
  int n = _sons.length ();
  if (_properties & OPEN) n -= 1;
  if (_properties & CLOSE) n -= 1;
  if (_properties & INTRO) n -= 1;
  if (!(_properties & SEPARATORS)) return n;
  if (!(_properties & END_SEP)) n += 1;
  if (_properties & NO_LAST_SEP) n += 1;
  return n / 2;
}

CTree *CT_List::Entry (int no) const {
  int n = no;
  if (_properties & SEPARATORS) n *= 2;
  if (_properties & OPEN) n += 1; // this is also OPEN_CLOSE
  if (_properties & INTRO) n += 1;
  if (no == Entries () - 1 && (_properties & NO_LAST_SEP)) n -= 1;
  return _sons.lookup (n);
}
 
void CT_List::AddSon (CTree *s) { 
  if (s) { 
    _son2idx[s] = _sons.length ();
    _sons.append (s); 
    SetParent (s, this); 
  } 
}

void CT_List::PrefixSon (CTree *s) { 
  if (s) { 
    SonToIndexMap::iterator idx = _son2idx.begin();
    for (; idx != _son2idx.end(); ++idx) idx->second++;
    _son2idx[s] = 0;
    _sons.prepend (s);
    SetParent (s, this); 
  } 
}

void CT_List::InsertSon (int idx, CTree *s) { 
  int len = _sons.length();
  if (s && idx <= len) { 
    _son2idx[s] = idx;
    _sons.insert (idx, s); 
    for (int i = idx+1; i <= len; ++i) _son2idx[_sons[i]] = i;
    SetParent (s, this); 
  } 
}

void CT_List::ReplaceSon (int idx, CTree *s) { 
  if (s && idx < Sons ()) { 
    _son2idx.erase(_sons[idx]);
    _son2idx[s] = idx;
    SetParent (_sons[idx], 0); 
    _sons[idx] = s; 
    SetParent (s, this); 
  } 
}

void CT_List::RemoveSon (int idx) { 
  int len = _sons.length();
  if (idx < len) { 
    _son2idx.erase(_sons[idx]);
    for (int i = idx+1; i < len; ++i) _son2idx[_sons[i]]--;
    SetParent (_sons[idx], 0); 
    _sons.remove (idx);
  } 
}

void CT_List::InsertSon (CTree *before_son, CTree *new_son) {
  SonToIndexMap::iterator iidx = _son2idx.find(before_son);
  if (iidx != _son2idx.end()) {
    int idx = iidx->second; 
    if (new_son) {
      SetParent (new_son, this); 
      _son2idx[new_son] = idx;
    }
    _sons.insert(idx, new_son);
    int len = _sons.length();
    for (int i = idx+1; i < len; ++i) _son2idx[_sons[i]]++;
  }
}

void CT_List::ReplaceSon (CTree *old_son, CTree *new_son) {
  SonToIndexMap::iterator iidx = _son2idx.find(old_son);
  if (iidx != _son2idx.end()) {
    int idx = iidx->second; 
    _son2idx.erase(iidx);
    if (new_son) {
      _son2idx[new_son] = idx;
      SetParent (new_son, this);
    }
    _sons[idx] = new_son;
  }
}

void CT_List::RemoveSon (CTree *son) {
  SonToIndexMap::iterator iidx = _son2idx.find(son);
  if (iidx != _son2idx.end()) {
    int idx = iidx->second; 
    _son2idx.erase(iidx);
    SetParent (son, 0); 
    _sons.remove(idx);
    int len = _sons.length();
    for (int i = idx; i < len; ++i) _son2idx[_sons[i]]--;
  }
}

int CT_List::Index (CTree *son) {
  SonToIndexMap::iterator idx = _son2idx.find(son);
  if (idx != _son2idx.end())
    return idx->second;
  return -1;
}

void CT_DeclList::Linkage (CT_LinkageSpec *l) {
  for (int e = 0; e < Entries (); e++)
    ((CT_Decl*)Entry (e))->Linkage (l);
}

void CT_PrimDeclSpec::determine_type () {
  switch (_token->token ()->type ()) {
    case TOK_FRIEND   : _type = PDS_FRIEND  ; break;
    case TOK_TYPEDEF  : _type = PDS_TYPEDEF ; break;
    case TOK_AUTO     : _type = PDS_AUTO    ; break;
    case TOK_REGISTER : _type = PDS_REGISTER; break;
    case TOK_STATIC   : _type = PDS_STATIC  ; break;
    case TOK_EXTERN   : _type = PDS_EXTERN  ; break;
    case TOK_MUTABLE  : _type = PDS_MUTABLE ; break;
    case TOK_INLINE   : _type = PDS_INLINE  ; break;
    case TOK_VIRTUAL  : _type = PDS_VIRTUAL ; break;
    case TOK_EXPLICIT : _type = PDS_EXPLICIT; break;
    case TOK_CONST    : _type = PDS_CONST   ; break;
    case TOK_VOLATILE : _type = PDS_VOLATILE; break;
    case TOK_CHAR     : _type = PDS_CHAR    ; break;
    case TOK_WCHAR_T  : _type = PDS_WCHAR_T ; break;
    case TOK_BOOL     : _type = PDS_BOOL    ; break;
    case TOK_C_BOOL   : _type = PDS_C_BOOL  ; break;
    case TOK_SHORT    : _type = PDS_SHORT   ; break;
    case TOK_INT      : _type = PDS_INT     ; break;
    case TOK_LONG     : _type = PDS_LONG    ; break;
    case TOK_SIGNED   : _type = PDS_SIGNED  ; break;
    case TOK_UNSIGNED : _type = PDS_UNSIGNED; break;
    case TOK_FLOAT    : _type = PDS_FLOAT   ; break;
    case TOK_DOUBLE   : _type = PDS_DOUBLE  ; break;
    case TOK_VOID     : _type = PDS_VOID    ; break;
    // GNU C specific type specifier
    case TOK_INT128   : _type = PDS_INT128  ; break;
    // GNU C++ specific storage class specifier
    case TOK_THREAD   : _type = PDS_THREAD  ; break;
    // AspectC++ specific type specifier
    case TOK_UNKNOWN_T: _type = PDS_UNKNOWN_T; break;
    // Win specific declaration specifiers 
    case TOK_INT64    : _type = PDS_INT64   ; break;
    case TOK_CDECL    : _type = PDS_CDECL   ; break;
    case TOK_STDCALL  : _type = PDS_STDCALL ; break;
    case TOK_FASTCALL : _type = PDS_FASTCALL; break;
    default:            _type = PDS_UNKNOWN;
  }
}

CT_SimpleName *CT_Declarator::Name () {
  CT_Declarator *dummy;
  return Name (dummy);
}

CT_SimpleName *CT_Declarator::Name (CT_Declarator *&last_declarator) {
  CT_SimpleName *name = Declarator ()->IsSimpleName ();
  if (name) {
    last_declarator = this;
    return name;
  }
  else
    return ((CT_Declarator*)Declarator ())->Name (last_declarator);
}

CT_DestructorName::CT_DestructorName (CTree *t, CTree *n) : 
 CT_SpecialName (2) {
  AddSon (t); 
  AddSon (n); 

  std::ostringstream name;
  name << "~" << n->token ()->text ();
  Name (name.str ().c_str ());
}

CT_OperatorName::CT_OperatorName (CTree *op) : CT_SpecialName (1) {
  AddSon (op);
  _oper = op->token ()->type ();
  Name (op->token ()->text ());
}

CT_OperatorName::CT_OperatorName (CTree *f, CTree *op, CTree *o, CTree *c) : 
 CT_SpecialName ((f ? 1 : 0) + (op ? 1 : 0) + (o ? 2 : 0)) { 
  if (f) AddSon (f); 
  if (op) AddSon (op); 
  if (o) AddSon (o); 
  if (c) AddSon (c); 

  std::ostringstream name;
  if (f) name << "operator ";
  op = op ? op : o;
  int oper = op->token ()->type ();
  switch (oper) {
    case TOK_NEW: 
      if (Sons () > 2) {
        _oper = NEW_ARRAY;
        name << "new[]";
      } else {
        _oper = oper;
        name << "new";
      }
      break;
    case TOK_DELETE: 
      if (Sons () > 2) {
        _oper = DEL_ARRAY;
        name << "delete[]";
      } else {
        _oper = oper;
        name << "delete";
      }
      break;
    case TOK_OPEN_ROUND: 
      _oper = FCT_CALL; 
      name << "()";
      break;
    case TOK_OPEN_SQUARE: 
      _oper = SUBSCRIPT; 
      name << "[]";
      break;
    default: 
      _oper = oper; 
      name << op->token ()->text ();
      break;
  }
  Name (name.str ().c_str ());
}

CT_ConversionName::CT_ConversionName (CTree *f, CTree *t) : 
 CT_SpecialName (2) { 
  AddSon (f); 
  AddSon (t); 

  std::ostringstream name;
  name << "operator ";
  TypeName ()->Object ()->TypeInfo ()->TypeText (name);
  Name (name.str ().c_str ());
}

void CT_QualName::print (std::ostream &os) const { 
  int entries = Entries () - 1;
  if (NodeName () == CT_RootQualName::NodeId ())
    os << "::";
  for (int i = 0; i < entries; i++)
    os << ((CT_SimpleName*)Entry (i))->Text () << "::";
  os << Text (); 
}

// static syntax tree node ids

// TODO: these extension node types shouldn't be defined here!
const char *CT_AsmBlock::NodeId () { return "AsmBlock"; }

const char *CT_GnuAsmSpec::NodeId () { return "GnuAsmSpec"; }
const char *CT_GnuAsmDef::NodeId () { return "GnuAsmDef"; }
const char *CT_GnuAsmOperand::NodeId () { return "GnuAsmOperand"; }
const char *CT_GnuAsmOperands::NodeId () { return "GnuAsmOperands"; }
const char *CT_GnuAsmClobbers::NodeId () { return "GnuAsmClobbers"; }
const char *CT_GnuTypeof::NodeId () { return "GnuTypeof"; }
const char *CT_GnuStatementExpr::NodeId () { return "GnuStatementExpr"; }
const char *CT_GnuAttribute::NodeId () { return "GnuAttribute"; }
const char *CT_GnuLocalLabelStmt::NodeId () { return "GnuLocalLabelStmt"; }
const char *CT_GnuCaseStmt::NodeId () { return "GnuCaseStmt"; }

const char *CT_Error::NodeId () { return "Error"; }
const char *CT_Token::NodeId () { return "Token"; }
const char *CT_ExprList::NodeId () { return "ExprList"; }
const char *CT_DeclaratorList::NodeId () { return "DeclaratorList"; }
const char *CT_EnumeratorList::NodeId () { return "EnumeratorList"; }
const char *CT_DeclList::NodeId () { return "DeclList"; }
const char *CT_DeclSpecSeq::NodeId () { return "DeclSpecSeq"; }
const char *CT_CmpdStmt::NodeId () { return "CmpdStmt"; }
const char *CT_HandlerSeq::NodeId () { return "HandlerSeq"; }
const char *CT_TemplateParamList::NodeId () { return "TemplateParamList"; }
const char *CT_TemplateArgList::NodeId () { return "TemplateArgList"; }
const char *CT_ExtensionList::NodeId () { return "ExtensionList"; }
const char *CT_Expression::NodeId() { return "Expression"; }
const char *CT_Call::NodeId() { return "Call"; }
const char *CT_ImplicitCall::NodeId() { return "ImplicitCall"; }
const char *CT_String::NodeId () { return "String"; }
const char *CT_WideString::NodeId () { return "WideString"; }
const char *CT_Integer::NodeId () { return "Integer"; }
const char *CT_Character::NodeId () { return "Character"; }
const char *CT_WideCharacter::NodeId () { return "WideCharacter"; }
const char *CT_Float::NodeId () { return "Float"; }
const char *CT_Bool::NodeId () { return "Bool"; }
const char *CT_BracedExpr::NodeId () { return "BracedExpr"; }
const char *CT_SimpleName::NodeId () { return "SimpleName"; }
const char *CT_PrivateName::NodeId () { return "PrivateName"; }
const char *CT_DestructorName::NodeId () { return "DestructorName"; }
const char *CT_TemplateName::NodeId () { return "TemplateName"; }
const char *CT_OperatorName::NodeId () { return "OperatorName"; }
const char *CT_ConversionName::NodeId () { return "ConversionName"; }
const char *CT_QualName::NodeId () { return "QualName"; }
const char *CT_RootQualName::NodeId () { return "RootQualName"; }
const char *CT_BinaryExpr::NodeId () { return "BinaryExpr"; }
const char *CT_MembPtrExpr::NodeId () { return "MembPtrExpr"; }
const char *CT_MembRefExpr::NodeId () { return "MembRefExpr"; }
const char *CT_UnaryExpr::NodeId () { return "UnaryExpr"; }
const char *CT_PostfixExpr::NodeId () { return "PostfixExpr"; }
const char *CT_AddrExpr::NodeId () { return "AddrExpr"; }
const char *CT_DerefExpr::NodeId () { return "DerefExpr"; }
const char *CT_DeleteExpr::NodeId () { return "DeleteExpr"; }
const char *CT_NewExpr::NodeId () { return "NewExpr"; }
const char *CT_IfThenExpr::NodeId () { return "IfThenExpr"; }
const char *CT_CmpdLiteral::NodeId () { return "CmpdLiteral"; }
const char *CT_ConstructExpr::NodeId () { return "ConstructExpr"; }
const char *CT_ThrowExpr::NodeId () { return "ThrowExpr"; }
const char *CT_IndexExpr::NodeId () { return "IndexExpr"; }
const char *CT_CallExpr::NodeId () { return "CallExpr"; }
const char *CT_CastExpr::NodeId () { return "CastExpr"; }
const char *CT_StaticCast::NodeId () { return "StaticCast"; }
const char *CT_ConstCast::NodeId () { return "ConstCast"; }
const char *CT_ReintCast::NodeId () { return "ReintCast"; }
const char *CT_DynamicCast::NodeId () { return "DynamicCast"; }
const char *CT_ImplicitCast::NodeId () { return "ImplicitCast"; }
const char *CT_TypeidExpr::NodeId () { return "TypeidExpr"; }
const char *CT_SizeofExpr::NodeId () { return "SizeofExpr"; }
const char *CT_AlignofExpr::NodeId () { return "AlignofExpr"; }
const char *CT_TypeTraitExpr::NodeId () { return "TypeTraitExpr"; }
const char *CT_OffsetofExpr::NodeId () { return "OffsetofExpr"; }
const char *CT_IndexDesignator::NodeId () { return "IndexDesignator"; }
const char *CT_MembDesignator::NodeId () { return "MembDesignator"; }
const char *CT_DesignatorSeq::NodeId () { return "DesignatorSeq"; }
const char *CT_PrimDeclSpec::NodeId () { return "PrimDeclSpec"; }
const char *CT_NamedType::NodeId () { return "NamedType"; }
const char *CT_ClassSpec::NodeId () { return "ClassSpec"; }
const char *CT_UnionSpec::NodeId () { return "UnionSpec"; }
const char *CT_EnumSpec::NodeId () { return "EnumSpec"; }
const char *CT_ExceptionSpec::NodeId () { return "ExceptionSpec"; }
const char *CT_Program::NodeId () { return "Program"; }
const char *CT_ObjDecl::NodeId () { return "ObjDecl"; }
const char *CT_TemplateDecl::NodeId () { return "TemplateDecl"; }
const char *CT_NonTypeParamDecl::NodeId () { return "NonTypeParamDecl"; }
const char *CT_TypeParamDecl::NodeId () { return "TypeParamDecl"; }
const char *CT_EnumDef::NodeId () { return "EnumDef"; }
const char *CT_Enumerator::NodeId () { return "Enumerator"; }
const char *CT_FctDef::NodeId () { return "FctDef"; }
const char *CT_AsmDef::NodeId () { return "AsmDef"; }
const char *CT_Handler::NodeId () { return "Handler"; }
const char *CT_LinkageSpec::NodeId () { return "LinkageSpec"; }
const char *CT_ArgDecl::NodeId () { return "ArgDecl"; }
const char *CT_ArgDeclList::NodeId () { return "ArgDeclList"; }
const char *CT_ArgDeclSeq::NodeId () { return "ArgDeclSeq"; }
const char *CT_ArgNameList::NodeId () { return "ArgNameList"; }
const char *CT_NamespaceDef::NodeId () { return "NamespaceDef"; }
const char *CT_NamespaceAliasDef::NodeId () { return "NamespaceAliasDef"; }
const char *CT_UsingDirective::NodeId () { return "UsingDirective"; }
const char *CT_InitDeclarator::NodeId () { return "InitDeclarator"; }
const char *CT_BracedDeclarator::NodeId () { return "BracedDeclarator"; }
const char *CT_ArrayDelimiter::NodeId () { return "ArrayDelimiter"; }
const char *CT_ArrayDeclarator::NodeId () { return "ArrayDeclarator"; }
const char *CT_FctDeclarator::NodeId () { return "FctDeclarator"; }
const char *CT_RefDeclarator::NodeId () { return "RefDeclarator"; }
const char *CT_PtrDeclarator::NodeId () { return "PtrDeclarator"; }
const char *CT_MembPtrDeclarator::NodeId () { return "MembPtrDeclarator"; }
const char *CT_BitFieldDeclarator::NodeId () { return "BitFieldDeclarator"; }
const char *CT_LabelStmt::NodeId () { return "LabelStmt"; }
const char *CT_DefaultStmt::NodeId () { return "DefaultStmt"; }
const char *CT_TryStmt::NodeId () { return "TryStmt"; }
const char *CT_CaseStmt::NodeId () { return "CaseStmt"; }
const char *CT_ExprStmt::NodeId () { return "ExprStmt"; }
const char *CT_DeclStmt::NodeId () { return "DeclStmt"; }
const char *CT_SwitchStmt::NodeId () { return "SwitchStmt"; }
const char *CT_IfStmt::NodeId () { return "IfStmt"; }
const char *CT_IfElseStmt::NodeId () { return "IfElseStmt"; }
const char *CT_BreakStmt::NodeId () { return "BreakStmt"; }
const char *CT_ContinueStmt::NodeId () { return "ContinueStmt"; }
const char *CT_GotoStmt::NodeId () { return "GotoStmt"; }
const char *CT_ReturnStmt::NodeId () { return "ReturnStmt"; }
const char *CT_WhileStmt::NodeId () { return "WhileStmt"; }
const char *CT_DoStmt::NodeId () { return "DoStmt"; }
const char *CT_ForStmt::NodeId () { return "ForStmt"; }
const char *CT_Condition::NodeId () { return "Condition"; }
const char *CT_ClassDef::NodeId () { return "ClassDef"; }
const char *CT_UnionDef::NodeId () { return "UnionDef"; }
const char *CT_MembList::NodeId () { return "MembList"; }
const char *CT_MembInitList::NodeId () { return "MembInitList"; }
const char *CT_MembInit::NodeId () { return "MembInit"; }
const char *CT_BaseSpecList::NodeId () { return "BaseSpecList"; }
const char *CT_AccessSpec::NodeId () { return "AccessSpec"; }
const char *CT_BaseSpec::NodeId () { return "BaseSpec"; }
const char *CT_AccessDecl::NodeId () { return "AccessDecl"; }
const char *CT_UsingDecl::NodeId () { return "UsingDecl"; }
const char *CT_Any::NodeId () { return "Any"; }
const char *CT_AnyList::NodeId () { return "AnyList"; }
const char *CT_AnyExtension::NodeId () { return "AnyExtension"; }
const char *CT_AnyCondition::NodeId () { return "AnyCondition"; }

const char *CT_DelayedParse::NodeId () { return "DelayedParse"; }
const char *CT_DelayedParseParamInit::NodeId () { return "DelayedParseParamInit"; }
const char *CT_DelayedParseFctTryBlock::NodeId () { return "DelayedParseFctTryBlock"; }
const char *CT_DelayedParseCtorInit::NodeId () { return "DelayedParseCtorInit"; }
const char *CT_DelayedParsePureSpec::NodeId () { return "DelayedParsePureSpec"; }
const char *CT_DelayedParseFctBody::NodeId () { return "DelayedParseFctBody"; }
const char *CT_DelayedParseSkippedFctBody::NodeId () { return "DelayedParseSkippedFctBody"; }
const char *CT_DelayedParseConstInit::NodeId () { return "DelayedParseConstInit"; }


} // namespace Puma

// constant size mempool
#include "Puma/ConstSizeMemPool.h"

namespace Puma {


void *CT_Token::operator new (size_t n) {
  assert (n == sizeof (CT_Token));
  return malloc<sizeof (CT_Token)> ();
}
void CT_Token::operator delete (void *p) {
  free<sizeof (CT_Token)> (p);
}

void *CT_SimpleName::operator new (size_t n) {
  assert (n == sizeof (CT_SimpleName));
  return malloc<sizeof (CT_SimpleName)> ();
}
void CT_SimpleName::operator delete (void *p) {
  free<sizeof (CT_SimpleName)> (p);
}

void *CT_SpecialName::operator new (size_t n) {
  assert (n == sizeof (CT_SpecialName));
  return malloc<sizeof (CT_SpecialName)> ();
}
void CT_SpecialName::operator delete (void *p) {
  free<sizeof (CT_SpecialName)> (p);
}

void *CT_PrivateName::operator new (size_t n) {
  assert (n == sizeof (CT_PrivateName));
  return malloc<sizeof (CT_PrivateName)> ();
}
void CT_PrivateName::operator delete (void *p) {
  free<sizeof (CT_PrivateName)> (p);
}

void *CT_OperatorName::operator new (size_t n) {
  assert (n == sizeof (CT_OperatorName));
  return malloc<sizeof (CT_OperatorName)> ();
}
void CT_OperatorName::operator delete (void *p) {
  free<sizeof (CT_OperatorName)> (p);
}

void *CT_DestructorName::operator new (size_t n) {
  assert (n == sizeof (CT_DestructorName));
  return malloc<sizeof (CT_DestructorName)> ();
}
void CT_DestructorName::operator delete (void *p) {
  free<sizeof (CT_DestructorName)> (p);
}

void *CT_ConversionName::operator new (size_t n) {
  assert (n == sizeof (CT_ConversionName));
  return malloc<sizeof (CT_ConversionName)> ();
}
void CT_ConversionName::operator delete (void *p) {
  free<sizeof (CT_ConversionName)> (p);
}

void *CT_TemplateName::operator new (size_t n) {
  assert (n == sizeof (CT_TemplateName));
  return malloc<sizeof (CT_TemplateName)> ();
}
void CT_TemplateName::operator delete (void *p) {
  free<sizeof (CT_TemplateName)> (p);
}

void *CT_QualName::operator new (size_t n) {
  assert (n == sizeof (CT_QualName));
  return malloc<sizeof (CT_QualName)> ();
}
void CT_QualName::operator delete (void *p) {
  free<sizeof (CT_QualName)> (p);
}

void *CT_RootQualName::operator new (size_t n) {
  assert (n == sizeof (CT_RootQualName));
  return malloc<sizeof (CT_RootQualName)> ();
}
void CT_RootQualName::operator delete (void *p) {
  free<sizeof (CT_RootQualName)> (p);
}


} // namespace Puma
