// This file is part of PUMA.
// Copyright (C) 1999-2015  The PUMA developer team.
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

#include "Puma/Token.h"
#include "Puma/CTokens.h"
#include "Puma/CCommentTokens.h"
#include "Puma/CWildcardTokens.h"
#include "Puma/PreMacroOpTokens.h"
#include "Puma/PreParser.h"
#include "Puma/PreExprParser.h"
#include "Puma/Chain.h"
#include "Puma/StrCol.h"
#include "Puma/ConstSizeMemPool.h"
#include "Puma/Unit.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

namespace Puma {

LanguageID Token::comment_id      = "Comment";
LanguageID Token::pre_id          = "Preprocessor";
LanguageID Token::macro_op_id     = "Pre macro op";
LanguageID Token::dir_id          = "Directive";
LanguageID Token::cpp_id          = "C++";
LanguageID Token::keyword_id      = "C++ keyword";
LanguageID Token::identifier_id   = "C++ identifier";
LanguageID Token::white_id        = "Whitespace";
LanguageID Token::wildcard_id     = "Wildcard";
LanguageID Token::open_id         = "(";
LanguageID Token::comma_id        = ",";
LanguageID Token::close_id        = ")";

void *Token::operator new(size_t n) {
  assert(n == sizeof(Token));
  return malloc<sizeof(Token)>();
}

void Token::operator delete(void *p) {
  free<sizeof(Token)>(p);
}

Token::Token(int t, LanguageID lang, const char *txt) {
  m_language = lang;
  m_flags = 0;
  m_continuousLines = 0;
  m_realType = t;
  m_type = get_general_type();
  determine_is_comment();
  determine_is_core();
  const char *static_text = get_static_text();
  if (static_text || !txt)
    m_text = 0;
  else
    m_text = StrCol::dup(txt);
}

Token::Token(const Token &t) {
  m_language = t.m_language;
  m_location = t.m_location;
  m_flags = t.m_flags;
  m_type = t.m_type;
  m_realType = t.m_realType;
  if (t.m_text)
    m_text = StrCol::dup(t.m_text);
  else
    m_text = 0;
  m_continuousLines = t.m_continuousLines ? new Array<int>(*t.m_continuousLines) : 0;
}

Token::~Token() {
  if (m_continuousLines) {
    delete m_continuousLines;
    m_continuousLines = 0;
  }
  if (m_text) {
    delete[] m_text;
    m_text = 0;
  }
}

DString &Token::dtext() const {
  if (m_dictText.empty())
    m_dictText = text();
  return m_dictText;
}

const char *Token::text() const {
  return m_text ? m_text : get_static_text();
}

int Token::get_general_type() const {
  switch (m_realType) {
    case TOK_AND_AND_ISO_646: return TOK_AND_AND;
    case TOK_AND_EQ_ISO_646: return TOK_AND_EQ;
    case TOK_AND_ISO_646: return TOK_AND;
    case TOK_OR_ISO_646: return TOK_OR;
    case TOK_TILDE_ISO_646: return TOK_TILDE;
    case TOK_NOT_ISO_646: return TOK_NOT;
    case TOK_NEQ_ISO_646: return TOK_NEQ;
    case TOK_OR_OR_ISO_646: return TOK_OR_OR;
    case TOK_IOR_EQ_ISO_646: return TOK_IOR_EQ;
    case TOK_ROOF_ISO_646: return TOK_ROOF;
    case TOK_XOR_EQ_ISO_646: return TOK_XOR_EQ;
    case TOK_ASM_2: return TOK_ASM;
    case TOK_ASM_3: return TOK_ASM;
    case TOK_CONST_2: return TOK_CONST;
    case TOK_CONST_3: return TOK_CONST;
    case TOK_INLINE_2: return TOK_INLINE;
    case TOK_INLINE_3: return TOK_INLINE;
    case TOK_RESTRICT_2: return TOK_RESTRICT;
    case TOK_RESTRICT_3: return TOK_RESTRICT;
    case TOK_SIGNED_2: return TOK_SIGNED;
    case TOK_SIGNED_3: return TOK_SIGNED;
    case TOK_VOLATILE_2: return TOK_VOLATILE;
    case TOK_VOLATILE_3: return TOK_VOLATILE;
    case TOK_WCHAR_T_2: return TOK_WCHAR_T;
    case TOK_TYPEOF_2: return TOK_TYPEOF;
    case TOK_TYPEOF_3: return TOK_TYPEOF;
    case TOK_TYPEOF_4: return TOK_TYPEOF;
    case TOK_ALIGNOF_2: return TOK_ALIGNOF;
    case TOK_ALIGNOF_3: return TOK_ALIGNOF;
    case TOK_CDECL_2: return TOK_CDECL;
    case TOK_STDCALL_2: return TOK_STDCALL;
    case TOK_FASTCALL_2: return TOK_FASTCALL;
    default: return m_realType;
  }
}

void Token::reset(int t, const char *txt, LanguageID id) {
  if (m_continuousLines) {
    delete m_continuousLines;
  }
  if (m_text) {
    delete[] m_text;
  }

  m_language = id;
  m_continuousLines = 0;
  m_realType = t;
  m_type = get_general_type();
  determine_is_comment();
  determine_is_core();
  const char *static_text = get_static_text();
  m_text = static_text || !txt ? 0 : StrCol::dup(txt);
}

char *Token::get_static_text() const {
  if (!(is_core() || is_wildcard()) || (is_identifier() && !is_keyword()))
    return 0;
  return get_static_text(m_realType);
}

char *Token::get_static_text(int token_type) {
  switch (token_type) {
    case TOK_COMMA: return (char*) ",";
    case TOK_ASSIGN: return (char*) "=";
    case TOK_AT: return (char*) "@";
    case TOK_QUESTION: return (char*) "?";
    case TOK_OR: return (char*) "|";
    case TOK_ROOF: return (char*) "^";
    case TOK_AND: return (char*) "&";
    case TOK_PLUS: return (char*) "+";
    case TOK_MINUS: return (char*) "-";
    case TOK_MUL: return (char*) "*";
    case TOK_DIV: return (char*) "/";
    case TOK_MODULO: return (char*) "%";
    case TOK_LESS: return (char*) "<";
    case TOK_GREATER: return (char*) ">";
    case TOK_OPEN_ROUND: return (char*) "(";
    case TOK_CLOSE_ROUND: return (char*) ")";
    case TOK_OPEN_SQUARE: return (char*) "[";
    case TOK_CLOSE_SQUARE: return (char*) "]";
    case TOK_OPEN_CURLY: return (char*) "{";
    case TOK_CLOSE_CURLY: return (char*) "}";
    case TOK_SEMI_COLON: return (char*) ";";
    case TOK_COLON: return (char*) ":";
    case TOK_NOT: return (char*) "!";
    case TOK_TILDE: return (char*) "~";
    case TOK_DOT: return (char*) ".";
    case TOK_MUL_EQ: return (char*) "*=";
    case TOK_DIV_EQ: return (char*) "/=";
    case TOK_MOD_EQ: return (char*) "%=";
    case TOK_ADD_EQ: return (char*) "+=";
    case TOK_SUB_EQ: return (char*) "-=";
    case TOK_LSH_EQ: return (char*) "<<=";
    case TOK_RSH_EQ: return (char*) ">>=";
    case TOK_AND_EQ: return (char*) "&=";
    case TOK_XOR_EQ: return (char*) "^=";
    case TOK_IOR_EQ: return (char*) "|=";
    case TOK_OR_OR: return (char*) "||";
    case TOK_AND_AND: return (char*) "&&";
    case TOK_EQL: return (char*) "==";
    case TOK_NEQ: return (char*) "!=";
    case TOK_LEQ: return (char*) "<=";
    case TOK_GEQ: return (char*) ">=";
    case TOK_LSH: return (char*) "<<";
    case TOK_RSH: return (char*) ">>";
    case TOK_DOT_STAR: return (char*) ".*";
    case TOK_PTS_STAR: return (char*) "->*";
    case TOK_INCR: return (char*) "++";
    case TOK_DECR: return (char*) "--";
    case TOK_PTS: return (char*) "->";
    case TOK_COLON_COLON: return (char*) "::";
    case TOK_ELLIPSIS: return (char*) "...";
    case TOK_ZERO_VAL: return (char*) "0";
    case TOK_AUTO: return (char*) "auto";
    case TOK_BREAK: return (char*) "break";
    case TOK_CASE: return (char*) "case";
    case TOK_CHAR: return (char*) "char";
    case TOK_CONTINUE: return (char*) "continue";
    case TOK_DEFAULT: return (char*) "default";
    case TOK_DO: return (char*) "do";
    case TOK_DOUBLE: return (char*) "double";
    case TOK_ELSE: return (char*) "else";
    case TOK_ENUM: return (char*) "enum";
    case TOK_EXTERN: return (char*) "extern";
    case TOK_FLOAT: return (char*) "float";
    case TOK_FOR: return (char*) "for";
    case TOK_GOTO: return (char*) "goto";
    case TOK_IF: return (char*) "if";
    case TOK_INT: return (char*) "int";
    case TOK_INT128: return (char*) "__int128";
    case TOK_INT64: return (char*) "__int64";
    case TOK_LONG: return (char*) "long";
    case TOK_REGISTER: return (char*) "register";
    case TOK_RETURN: return (char*) "return";
    case TOK_SHORT: return (char*) "short";
    case TOK_SIZEOF: return (char*) "sizeof";
    case TOK_STATIC: return (char*) "static";
    case TOK_THREAD: return (char*) "__thread";
    case TOK_STRUCT: return (char*) "struct";
    case TOK_SWITCH: return (char*) "switch";
    case TOK_TYPEDEF: return (char*) "typedef";
    case TOK_UNION: return (char*) "union";
    case TOK_UNSIGNED: return (char*) "unsigned";
    case TOK_VOID: return (char*) "void";
    case TOK_WHILE: return (char*) "while";
    case TOK_CATCH: return (char*) "catch";
    case TOK_CLASS: return (char*) "class";
    case TOK_CONST_CAST: return (char*) "const_cast";
    case TOK_DELETE: return (char*) "delete";
    case TOK_DYN_CAST: return (char*) "dynamic_cast";
    case TOK_EXPLICIT: return (char*) "explicit";
    case TOK_EXPORT: return (char*) "export";
    case TOK_FRIEND: return (char*) "friend";
    case TOK_MUTABLE: return (char*) "mutable";
    case TOK_NAMESPACE: return (char*) "namespace";
    case TOK_NEW: return (char*) "new";
    case TOK_OPERATOR: return (char*) "operator";
    case TOK_PRIVATE: return (char*) "private";
    case TOK_PROTECTED: return (char*) "protected";
    case TOK_PUBLIC: return (char*) "public";
    case TOK_REINT_CAST: return (char*) "reinterpret_cast";
    case TOK_STAT_CAST: return (char*) "static_cast";
    case TOK_TEMPLATE: return (char*) "template";
    case TOK_THIS: return (char*) "this";
    case TOK_THROW: return (char*) "throw";
    case TOK_TRY: return (char*) "try";
    case TOK_TYPEID: return (char*) "typeid";
    case TOK_TYPENAME: return (char*) "typename";
    case TOK_USING: return (char*) "using";
    case TOK_VIRTUAL: return (char*) "virtual";
    case TOK_POINTCUT: return (char*) "pointcut";
    case TOK_ASPECT: return (char*) "aspect";
    case TOK_ADVICE: return (char*) "advice";
    case TOK_SLICE: return (char*) "slice";
    case TOK_UNKNOWN_T: return (char*) "__unknown_t";
    case TOK_CCSINGLE: return (char*) "//";
    case TOK_CCMULTIBEGIN: return (char*) "/*";
    case TOK_CCMULTIEND: return (char*) "*/";
    case TOK_WC_AND: return (char*) "\\and";
    case TOK_WC_OR: return (char*) "\\or";
    case TOK_WC_XOR: return (char*) "\\xor";
    case TOK_WC_SEQ: return (char*) "\\seq";
    case TOK_WC_IF: return (char*) "\\if";
    case TOK_WC_ELIF: return (char*) "\\elif";
    case TOK_WC_ENDIF: return (char*) "\\endif";
    case TOK_WC_NOT: return (char*) "\\not";
    case TOK_WC_EXACT: return (char*) "\\exact";
    case TOK_WC_FIRST: return (char*) "\\first";
    case TOK_WC_LAST: return (char*) "\\last";
    case TOK_WC_LEAF: return (char*) "\\leaf";
    case TOK_WC_PRUNE: return (char*) "\\prune";
    case TOK_WC_COLLECT: return (char*) "\\collect";
    case TOK_WC_NODENAME: return (char*) "\\nodename";
    case TOK_WC_ELEMENT: return (char*) "\\element";
    case TOK_DO_EXPR: return (char*) "\\do-expression";
    case TOK_DO_EXPR_LIST: return (char*) "\\do-expression-list";
    case TOK_DO_ID_EXPR: return (char*) "\\do-id-expression";
    case TOK_DO_IDENTIFIER: return (char*) "\\do-identifier";
    case TOK_DO_DECL_SPEC: return (char*) "\\do-decl-specifier";
    case TOK_DO_DECL_SPEC_SEQ: return (char*) "\\do-decl-specifier-seq";
    case TOK_DO_INIT_DECLARATOR: return (char*) "\\do-init-declarator";
    case TOK_DO_INIT_DECLARATOR_LIST: return (char*) "\\do-init-declarator-list";
    case TOK_DO_DECLARATOR: return (char*) "\\do-declarator";
    case TOK_DO_INIT: return (char*) "\\do-initializer";
    case TOK_DO_ARG_DECL_SEQ: return (char*) "\\do-parameter-declaration-clause";
    case TOK_DO_FCT_BODY: return (char*) "\\do-function-body";
    case TOK_DO_CLASS_SPEC: return (char*) "\\do-class-specifier";
    case TOK_DO_MEMBER_SPEC: return (char*) "\\do-member-specification";
    case TOK_DO_MEMBER_DECL: return (char*) "\\do-member-declaration";
    case TOK_DO_BASE_CLAUSE: return (char*) "\\do-base-clause";
    case TOK_DO_CTOR_INIT: return (char*) "\\do-ctor-initializer";
    case TOK_DO_MEM_INIT: return (char*) "\\do-mem-initializer";
    case TOK_DO_STMT: return (char*) "\\do-statement";
    case TOK_DO_STMT_SEQ: return (char*) "\\do-statement-seq";
    case TOK_ANY_EXPR: return (char*) "\\any-expression";
    case TOK_ANY_EXPR_LIST: return (char*) "\\any-expression-list";
    case TOK_ANY_ID_EXPR: return (char*) "\\any-id-expression";
    case TOK_ANY_IDENTIFIER: return (char*) "\\any-identifier";
    case TOK_ANY_DECL_SPEC: return (char*) "\\any-decl-specifier";
    case TOK_ANY_DECL_SPEC_SEQ: return (char*) "\\any-decl-specifier-seq";
    case TOK_ANY_INIT_DECLARATOR_LIST: return (char*) "\\any-init-declarator-list";
    case TOK_ANY_INIT_DECLARATOR: return (char*) "\\any-init-declarator";
    case TOK_ANY_DECLARATOR: return (char*) "\\any-declarator";
    case TOK_ANY_INIT: return (char*) "\\any-initializer";
    case TOK_ANY_ARG_DECL_SEQ: return (char*) "\\any-parameter-declaration-clause";
    case TOK_ANY_FCT_BODY: return (char*) "\\any-function-body";
    case TOK_ANY_CLASS_SPEC: return (char*) "\\any-class-specifier";
    case TOK_ANY_MEMBER_SPEC: return (char*) "\\any-member-specification";
    case TOK_ANY_MEMBER_DECL: return (char*) "\\any-member-declaration";
    case TOK_ANY_BASE_CLAUSE: return (char*) "\\any-base-clause";
    case TOK_ANY_CTOR_INIT: return (char*) "\\any-ctor-initializer";
    case TOK_ANY_MEM_INIT: return (char*) "\\any-mem-initializer";
    case TOK_ANY_STMT: return (char*) "\\any-statement";
    case TOK_ANY_STMT_SEQ: return (char*) "\\any-statement-seq";
    case TOK_WC_IS_TYPEDEF: return (char*) "\\is-typedef";
    case TOK_WC_IS_CLASS: return (char*) "\\is-class";
    case TOK_WC_IS_ENUM: return (char*) "\\is-enum";
    case TOK_MO_HASH: return (char*) "#";
    case TOK_MO_HASHHASH: return (char*) "##";
    case ID_END_OF_FILE:
    case ID_ERROR:
    case ID_UNKNOWN:
    case ID_WARNING: return (char*) "";
    case TOK_AND_AND_ISO_646: return (char*) "and";
    case TOK_AND_EQ_ISO_646: return (char*) "and_eq";
    case TOK_AND_ISO_646: return (char*) "bitand";
    case TOK_OR_ISO_646: return (char*) "bitor";
    case TOK_TILDE_ISO_646: return (char*) "compl";
    case TOK_NOT_ISO_646: return (char*) "not";
    case TOK_NEQ_ISO_646: return (char*) "not_eq";
    case TOK_OR_OR_ISO_646: return (char*) "or";
    case TOK_IOR_EQ_ISO_646: return (char*) "or_eq";
    case TOK_ROOF_ISO_646: return (char*) "xor";
    case TOK_XOR_EQ_ISO_646: return (char*) "xor_eq";
    case TOK_ASM: return (char*) "asm";
    case TOK_ASM_2: return (char*) "__asm";
    case TOK_ASM_3: return (char*) "__asm__";
    case TOK_BOOL: return (char*) "bool";
    case TOK_C_BOOL: return (char*) "_Bool";
    case TOK_CONST: return (char*) "const";
    case TOK_CONST_2: return (char*) "__const";
    case TOK_CONST_3: return (char*) "__const__";
    case TOK_INLINE: return (char*) "inline";
    case TOK_INLINE_2: return (char*) "__inline";
    case TOK_INLINE_3: return (char*) "__inline__";
    case TOK_RESTRICT: return (char*) "restrict";
    case TOK_RESTRICT_2: return (char*) "__restrict";
    case TOK_RESTRICT_3: return (char*) "__restrict__";
    case TOK_SIGNED: return (char*) "signed";
    case TOK_SIGNED_2: return (char*) "__signed";
    case TOK_SIGNED_3: return (char*) "__signed__";
    case TOK_VOLATILE: return (char*) "volatile";
    case TOK_VOLATILE_2: return (char*) "__volatile";
    case TOK_VOLATILE_3: return (char*) "__volatile__";
    case TOK_WCHAR_T: return (char*) "wchar_t";
    case TOK_WCHAR_T_2: return (char*) "__wchar_t";
    case TOK_TYPEOF: return (char*) "typeof";
    case TOK_TYPEOF_2: return (char*) "__typeof";
    case TOK_TYPEOF_3: return (char*) "__typeof__";
    case TOK_TYPEOF_4: return (char*) "__decltype";
    case TOK_ALIGNOF:
    case TOK_ALIGNOF_2: return (char*) "__alignof";
    case TOK_ALIGNOF_3: return (char*) "__alignof__";
    case TOK_CDECL: return (char*) "_cdecl";
    case TOK_CDECL_2: return (char*) "__cdecl";
    case TOK_STDCALL: return (char*) "_stdcall";
    case TOK_STDCALL_2: return (char*) "__stdcall";
    case TOK_FASTCALL: return (char*) "_fastcall";
    case TOK_FASTCALL_2: return (char*) "__fastcall";
    case TOK_IF_EXISTS: return (char*) "__if_exists";
    case TOK_IF_NOT_EXISTS: return (char*) "__if_not_exists";
    case TOK_HAS_NOTHROW_ASSIGN: return (char*) "__has_nothrow_assign";
    case TOK_HAS_NOTHROW_COPY: return (char*) "__has_nothrow_copy";
    case TOK_HAS_NOTHROW_CTOR: return (char*) "__has_nothrow_constructor";
    case TOK_HAS_TRIVIAL_ASSIGN: return (char*) "__has_trivial_assign";
    case TOK_HAS_TRIVIAL_COPY: return (char*) "__has_trivial_copy";
    case TOK_HAS_TRIVIAL_CTOR: return (char*) "__has_trivial_constructor";
    case TOK_HAS_TRIVIAL_DTOR: return (char*) "__has_trivial_destructor";
    case TOK_HAS_VIRTUAL_DTOR: return (char*) "__has_virtual_destructor";
    case TOK_IS_ABSTRACT: return (char*) "__is_abstract";
    case TOK_IS_BASE_OF: return (char*) "__is_base_of";
    case TOK_IS_CLASS: return (char*) "__is_class";
    case TOK_IS_EMPTY: return (char*) "__is_empty";
    case TOK_IS_ENUM: return (char*) "__is_enum";
    case TOK_IS_POD: return (char*) "__is_pod";
    case TOK_IS_TRIVIAL: return (char*) "__is_trivial";
    case TOK_IS_POLYMORPHIC: return (char*) "__is_polymorphic";
    case TOK_IS_UNION: return (char*) "__is_union";
//    case TOK_EXPR_QUESTION: return (char*)"?";
//    case TOK_EXPR_BITOR: return (char*)"|";
//    case TOK_EXPR_BITXOR: return (char*)"^";
//    case TOK_EXPR_BITAND: return (char*)"&";
//    case TOK_EXPR_PLUS: return (char*)"+";
//    case TOK_EXPR_MINUS: return (char*)"-";
//    case TOK_EXPR_STAR: return (char*)"*";
//    case TOK_EXPR_DIV: return (char*)"/";
//    case TOK_EXPR_MOD: return (char*)"%";
//    case TOK_EXPR_LESS: return (char*)"<";
//    case TOK_EXPR_GREATER: return (char*)">";
//    case TOK_EXPR_LPAREN: return (char*)"(";
//    case TOK_EXPR_RPAREN: return (char*)")";
//    case TOK_EXPR_COLON: return (char*)":";
//    case TOK_EXPR_NOT: return (char*)"!";
//    case TOK_EXPR_TILDE: return (char*)"~";
//    case TOK_EXPR_OR: return (char*)"||";
//    case TOK_EXPR_AND: return (char*)"&&";
//    case TOK_EXPR_EQ: return (char*)"==";
//    case TOK_EXPR_NOTEQ: return (char*)"!=";
//    case TOK_EXPR_LEQ: return (char*)"<=";
//    case TOK_EXPR_GEQ: return (char*)">=";
//    case TOK_EXPR_SHIFTL: return (char*)"<<";
//    case TOK_EXPR_SHIFTR: return (char*)">>";
    default: return 0;
  }
}

void Token::print(std::ostream &os) const {
  // If m_continuousLines isn't NULL the real token text contains
  // continuation lines.
  const char *txt = text();
  if (m_continuousLines) {
    int pos = 0;
    for (int cl = 0; cl < m_continuousLines->length(); cl++) {
      for (int c = 0; c < m_continuousLines->fetch(cl); c++)
        os << txt[pos++];
      os << "\\\n";
    }
    if (txt[pos])
      os << txt + pos;
  } else if (txt) {
    os << txt;
  }
}

int Token::line_breaks() const {
  int result = 0;
  if (m_continuousLines)
    result += m_continuousLines->length();
  if (!is_core()) {
    const char *curr = text();
    while (*curr) {
      if (*curr == '\n')
        result++;
      curr++;
    }
  }
  return result;
}

bool Token::is_macro_generated() const {
  return unit()->isMacroExp();
}

Unit* Token::unit() const {
  return (Unit*) belonging_to();
}

} // namespace Puma
